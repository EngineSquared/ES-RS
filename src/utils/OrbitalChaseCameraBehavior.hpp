#pragma once

#include <cmath>
#include <algorithm>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "component/Camera.hpp"
#include "component/Transform.hpp"
#include "core/Core.hpp"
#include "resource/CameraManager.hpp"
#include "resource/InputManager.hpp"
#include "resource/Window.hpp"
#include "utils/CameraBehavior.hpp"
#include "utils/CameraUtils.hpp"

/**
 * @brief Chase camera behavior that allows orbital movement around the vehicle using the mouse.
 */
class OrbitalChaseCameraBehavior : public CameraMovement::Utils::ICameraBehavior {
  public:
    explicit OrbitalChaseCameraBehavior(Engine::Core &core, Engine::Entity vehicleEntity)
        : _core(&core), _vehicleEntity(vehicleEntity)
    {
        if (!core.HasResource<Input::Resource::InputManager>())
        {
            return;
        }

        auto &inputManager = core.GetResource<Input::Resource::InputManager>();

        _mouseButtonCallbackId = inputManager.RegisterMouseButtonCallback(
            [this](Engine::Core &core, int button, int action, int mods) { HandleMouseButton(core, button, action, mods); });

        _cursorPosCallbackId = inputManager.RegisterCursorPosCallback(
            [this](Engine::Core &core, double xpos, double ypos) { HandleCursorPos(core, xpos, ypos); });

        _scrollCallbackId = inputManager.RegisterScrollCallback(
            [this](Engine::Core &core, double xoffset, double yoffset) { HandleScroll(core, xoffset, yoffset); });

        // Initialize mouse position to avoid large jumps on first move
        auto &cameraManager = core.GetResource<CameraMovement::Resource::CameraManager>();
        _lastMouseX = cameraManager.GetLastMouseX();
        _lastMouseY = cameraManager.GetLastMouseY();
    }

    ~OrbitalChaseCameraBehavior() override
    {
        if (!_core)
            return;

        try
        {
            if (_core->HasResource<Input::Resource::InputManager>())
            {
                auto &inputManager = _core->GetResource<Input::Resource::InputManager>();
                if (_mouseButtonCallbackId != 0)
                {
                    inputManager.DeleteMouseButtonCallback(_mouseButtonCallbackId);
                }
                if (_cursorPosCallbackId != 0)
                {
                    inputManager.DeleteCursorPosCallback(_cursorPosCallbackId);
                }
                if (_scrollCallbackId != 0)
                {
                    inputManager.DeleteScrollCallback(_scrollCallbackId);
                }
            }
        }
        catch (...)
        {
            // Destructor should not throw
        }
    }

    /**
     * @brief Update the camera position and rotation based on vehicle position and orbital offsets.
     */
    void Update(Engine::Core &core, CameraMovement::Resource::CameraManager &manager,
                Object::Component::Transform &transform, Object::Component::Camera & /*camera*/, float /*deltaTime*/) override
    {
        auto &registry = core.GetRegistry();
        if (!_vehicleEntity.IsAlive() || !registry.valid(_vehicleEntity))
        {
            return;
        }

        auto &window = core.GetResource<Window::Resource::Window>();
        manager.SetWasCursorMasked(window.IsCursorMasked());

        auto &vehicleTransform = registry.get<Object::Component::Transform>(_vehicleEntity);
        glm::vec3 vehiclePos = vehicleTransform.GetPosition();
        glm::quat vehicleRot = vehicleTransform.GetRotation();

        glm::vec3 targetPos = vehiclePos + glm::vec3(0.0f, 1.0f, 0.0f);

        float horizontalDistance = _distance * std::cos(_pitch);
        glm::vec3 localOffset;
        localOffset.x = horizontalDistance * std::sin(_yaw);
        localOffset.y = _distance * std::sin(_pitch);
        localOffset.z = -horizontalDistance * std::cos(_yaw);

        glm::vec3 horizontalOffset = vehicleRot * glm::vec3(localOffset.x, 0.0f, localOffset.z);
        glm::vec3 cameraPosition = targetPos + horizontalOffset + glm::vec3(0.0f, localOffset.y, 0.0f);

        transform.SetPosition(cameraPosition);

        glm::quat lookRotation =
            CameraMovement::Utils::ComputeLookAtQuaternion(cameraPosition, targetPos, glm::vec3(0.0f, 1.0f, 0.0f));
        transform.SetRotation(lookRotation);
    }

    Engine::Entity GetVehicleEntity() const { return _vehicleEntity; }

    void SetVehicleEntity(Engine::Entity vehicleEntity) { _vehicleEntity = vehicleEntity; }

  private:
    /**
     * @brief Handle mouse button presses for dragging the camera.
     */
    void HandleMouseButton(Engine::Core & /*core*/, int button, int action, int /*mods*/)
    {
        if (button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            _isDragging = (action == GLFW_PRESS);
        }
    }

    /**
     * @brief Handle cursor movement to update orbital rotation.
     */
    void HandleCursorPos(Engine::Core &core, double xpos, double ypos)
    {
        auto &cameraManager = core.GetResource<CameraMovement::Resource::CameraManager>();
        auto &window = core.GetResource<Window::Resource::Window>();

        bool shouldRotate = (window.IsCursorMasked() || _isDragging) &&
                            !(window.IsCursorMasked() && !cameraManager.WasCursorMasked());

        if (shouldRotate)
        {
            float sensitivity = cameraManager.GetMouseSensitivity() * 2.0f;

            float dx = static_cast<float>(xpos - _lastMouseX);
            float dy = static_cast<float>(ypos - _lastMouseY);

            _yaw -= dx * sensitivity;
            _pitch += dy * sensitivity;

            // Clamp pitch to avoid gimbal lock and looking from directly above/below
            // TODO: reactivate
            // constexpr float maxPitch = 1.48f; // ~85 degrees
            // constexpr float minPitch = -0.1f; // ~-5.7 degrees (slightly below horizon)
            // _pitch = std::max(minPitch, std::min(maxPitch, _pitch));
        }

        _lastMouseX = xpos;
        _lastMouseY = ypos;
    }

    /**
     * @brief Handle mouse wheel scrolling for zooming.
     */
    void HandleScroll(Engine::Core & /*core*/, double /*xoffset*/, double yoffset)
    {
        _distance -= static_cast<float>(yoffset) * 1.1f;
        _distance = std::max(0.001f, std::min(50.0f, _distance));
    }

    Engine::Core *_core;
    Engine::Entity _vehicleEntity;

    float _yaw = 0.0f;
    float _pitch = 0.35f; // Initial angle (looking slightly down)
    float _distance = 8.0f;

    bool _isDragging = false;
    double _lastMouseX = 0.0;
    double _lastMouseY = 0.0;

    FunctionUtils::FunctionID _mouseButtonCallbackId = 0;
    FunctionUtils::FunctionID _cursorPosCallbackId = 0;
    FunctionUtils::FunctionID _scrollCallbackId = 0;
};

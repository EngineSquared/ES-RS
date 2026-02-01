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

/**
 * @brief First-person orbital camera behavior: places the camera inside the vehicle (driver's
 * perspective) but allows the user to rotate the view with the mouse just like
 * OrbitalChaseCameraBehavior (yaw/pitch drag + cursor masking).
 */
class FirstPersonOrbitalCameraBehavior : public CameraMovement::Utils::ICameraBehavior {
  public:
    explicit FirstPersonOrbitalCameraBehavior(Engine::Core &core, Engine::Entity vehicleEntity)
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

        auto &cameraManager = core.GetResource<CameraMovement::Resource::CameraManager>();
        _lastMouseX = cameraManager.GetLastMouseX();
        _lastMouseY = cameraManager.GetLastMouseY();
    }

    ~FirstPersonOrbitalCameraBehavior() override
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
     * @brief Update the camera position and rotation based on vehicle position and local yaw/pitch.
     */
    void Update(Engine::Core &core, CameraMovement::Resource::CameraManager &manager,
                Object::Component::Transform &transform, Object::Component::Camera & /*camera*/, float /*deltaTime*/) override
    {
        auto &window = core.GetResource<Window::Resource::Window>();
        if (!window.IsCursorMasked())
        {
            window.MaskCursor();
        }

        auto &registry = core.GetRegistry();
        if (!_vehicleEntity.IsAlive() || !registry.valid(_vehicleEntity))
        {
            return;
        }

        manager.SetWasCursorMasked(window.IsCursorMasked());

        auto &vehicleTransform = registry.get<Object::Component::Transform>(_vehicleEntity);
        glm::vec3 vehiclePos = vehicleTransform.GetPosition();
        glm::quat vehicleRot = vehicleTransform.GetRotation();

        glm::vec3 localOffset = _localOffset;
        glm::vec3 cameraPosition = vehiclePos + vehicleRot * localOffset;

        transform.SetPosition(cameraPosition);

        glm::quat yawQuat = glm::angleAxis(_yaw, glm::vec3(0.0f, -1.0f, 0.0f));
        glm::quat pitchQuat = glm::angleAxis(_pitch, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::quat localRot = yawQuat * pitchQuat;

        glm::quat finalRot = vehicleRot * localRot;
        transform.SetRotation(finalRot);
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
     * @brief Handle cursor movement to update yaw/pitch.
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

            // Clamp pitch to avoid flipping
            constexpr float maxPitch = 1.48f; // ~85 degrees
            constexpr float minPitch = -1.48f; // ~-85 degrees
            _pitch = std::max(minPitch, std::min(maxPitch, _pitch));
        }

        _lastMouseX = xpos;
        _lastMouseY = ypos;
    }

    /**
     * @brief Handle mouse wheel. For first-person we use scroll to adjust a small vertical offset
     * to simulate sitting/leaning; it's clamped to reasonable values.
     */
    void HandleScroll(Engine::Core & /*core*/, double /*xoffset*/, double yoffset)
    {
        _localOffset.y += static_cast<float>(yoffset) * 0.02f;
        _localOffset.y = std::max(-0.5f, std::min(1.0f, _localOffset.y));
    }

    Engine::Core *_core = nullptr;
    Engine::Entity _vehicleEntity;

    float _yaw = 0.0f;
    float _pitch = 0.0f; // start level with vehicle's forward

    bool _isDragging = false;
    double _lastMouseX = 0.0;
    double _lastMouseY = 0.0;

    glm::vec3 _localOffset = glm::vec3(-0.35f, 0.42f, -0.25f);

    FunctionUtils::FunctionID _mouseButtonCallbackId = 0;
    FunctionUtils::FunctionID _cursorPosCallbackId = 0;
    FunctionUtils::FunctionID _scrollCallbackId = 0;
};

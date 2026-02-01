#pragma once

#include <cmath>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "component/Camera.hpp"
#include "component/Transform.hpp"
#include "core/Core.hpp"
#include "resource/CameraManager.hpp"
#include "resource/Window.hpp"
#include "utils/CameraBehavior.hpp"

/**
 * @brief First-person camera behavior: places the camera inside the vehicle (driver's perspective).
 */
class FirstPersonCameraBehavior : public CameraMovement::Utils::ICameraBehavior {
  public:
    explicit FirstPersonCameraBehavior(Engine::Core &core, Engine::Entity vehicleEntity)
        : _core(&core), _vehicleEntity(vehicleEntity) {}

    explicit FirstPersonCameraBehavior(Engine::Entity vehicleEntity) : _core(nullptr), _vehicleEntity(vehicleEntity) {}

    ~FirstPersonCameraBehavior() override = default;

    void Update(Engine::Core &core, CameraMovement::Resource::CameraManager & /*manager*/,
                Object::Component::Transform &transform, Object::Component::Camera & /*camera*/, float /*deltaTime*/) override
    {
        auto &window = core.GetResource<Window::Resource::Window>();
        if (window.IsCursorMasked())
        {
            window.ShowCursor();
        }

        auto &registry = core.GetRegistry();
        if (!_vehicleEntity.IsAlive() || !registry.valid(_vehicleEntity))
        {
            return;
        }

        auto &vehicleTransform = registry.get<Object::Component::Transform>(_vehicleEntity);
        glm::vec3 vehiclePos = vehicleTransform.GetPosition();
        glm::quat vehicleRot = vehicleTransform.GetRotation();

        glm::vec3 localOffset = glm::vec3(-0.35f, 0.42f, -0.25f);
        glm::vec3 cameraPosition = vehiclePos + vehicleRot * localOffset;

        transform.SetPosition(cameraPosition);

        transform.SetRotation(vehicleRot);
    }

    Engine::Entity GetVehicleEntity() const { return _vehicleEntity; }

    void SetVehicleEntity(Engine::Entity vehicleEntity) { _vehicleEntity = vehicleEntity; }

  private:
    Engine::Core *_core = nullptr;
    Engine::Entity _vehicleEntity;
};

#include "WheeledVehicleKeyboardMovement.hpp"

#include "WheeledVehicle3D.hpp"
#include "Logger.hpp"
#include "JoltPhysics.hpp"

void WheeledVehicleKeyboardMovement::operator()(ES::Engine::Core &core) const
{
    if (!entity.template HasComponents<ES::Plugin::Physics::Component::WheeledVehicle3D>(core))
    {
        ES::Utils::Log::Error(fmt::format("WheeledVehicleKeyboardMovement component is not fully initialized for entity {}",
                                          static_cast<uint32_t>(entity)));
        return;
    }

    auto &wheeledVehicle = entity.template GetComponents<ES::Plugin::Physics::Component::WheeledVehicle3D>(core);

    auto forwardForce = ES::Plugin::Input::Utils::IsKeyPressed(forwardKey) ? 1.0f : 0.0f;
    auto reverseForce = ES::Plugin::Input::Utils::IsKeyPressed(reverseKey) ? -1.0f : 0.0f;
    auto leftForce = ES::Plugin::Input::Utils::IsKeyPressed(leftKey) ? -1.0f : 0.0f;
    auto rightForce = ES::Plugin::Input::Utils::IsKeyPressed(rightKey) ? 1.0f : 0.0f;
    auto brakeForce = ES::Plugin::Input::Utils::IsKeyPressed(brakeKey) ? 1.0f : 0.0f;
    auto handbrakeForce = ES::Plugin::Input::Utils::IsKeyPressed(handbrakeKey) ? 1.0f : 0.0f;

    auto throttle = forwardForce + reverseForce;
    auto steering = leftForce + rightForce;

    wheeledVehicle.SetDriverInput(throttle, steering, brakeForce, handbrakeForce);

    if (throttle != 0.0f || steering != 0.0f || brakeForce != 0.0f || handbrakeForce != 0.0f)
    {
        if (!entity.template HasComponents<ES::Plugin::Physics::Component::RigidBody3D>(core))
        {
            ES::Utils::Log::Error(fmt::format("WheeledVehicleKeyboardMovement component is not fully initialized for entity {}",
                                              static_cast<uint32_t>(entity)));
            return;
        }
        auto &rigidBody = entity.template GetComponents<ES::Plugin::Physics::Component::RigidBody3D>(core);
        auto &physicsSystem = core.GetResource<ES::Plugin::Physics::Resource::PhysicsManager>().GetPhysicsSystem();
        physicsSystem.GetBodyInterface().ActivateBody(rigidBody.body->GetID());
    }
}
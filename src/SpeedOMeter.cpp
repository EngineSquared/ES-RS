#include "SpeedOMeter.hpp"
#include "PhysicsManager.hpp"
#include "RigidBody3D.hpp"

void UpdateSpeedOmeter(ES::Engine::Core &core)
{
    core.GetRegistry().view<ES::Plugin::Physics::Component::WheeledVehicle3D, ES::Plugin::Physics::Component::RigidBody3D>().each(
        [&core](auto &, ES::Plugin::Physics::Component::RigidBody3D &rigidBody) {
            const auto &physicsSystem = core.GetResource<ES::Plugin::Physics::Resource::PhysicsManager>().GetPhysicsSystem();
            const auto &linearVelocity = physicsSystem.GetBodyInterface().GetLinearVelocity(rigidBody.body->GetID());
            const int &speed = static_cast<int>(linearVelocity.Length() * 3.6f);
            core.GetResource<ES::Plugin::UI::Resource::UIResource>().UpdateInnerContent("value", std::to_string(speed));
        }
    );
}

void UpdateSpeedOmeterAnimations(ES::Engine::Core &core)
{
    float rpm = 0.0f;
    const float minRPM = 0.0f;
    const float maxRPM = 8000.0f;
    const float minAngle = 0.0f;
    const float maxAngle = 300.0f;

    core.GetRegistry().view<ES::Plugin::Physics::Component::WheeledVehicle3D>().each(
        [&core, &rpm](ES::Plugin::Physics::Component::WheeledVehicle3D &vehicle) {
            auto controller = reinterpret_cast<JPH::WheeledVehicleController *>(vehicle.vehicleConstraint->GetController());
            if (controller)
            {
                rpm = controller->GetWheelSpeedAtClutch();
            }
        }
    );

    float t = std::clamp((rpm - minRPM) / (maxRPM - minRPM), 0.0f, 1.0f);
    float angle = minAngle + t * (maxAngle - minAngle);

    core.GetResource<ES::Plugin::UI::Resource::UIResource>().SetTransformProperty("speed-counter-pointer", {
        {ES::Plugin::UI::Resource::UIResource::TransformType::Rotate, angle},
    });
}

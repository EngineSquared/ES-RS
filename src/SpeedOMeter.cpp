#include "SpeedOMeter.hpp"
#include "PhysicsManager.hpp"
#include "RigidBody3D.hpp"

void UpdateSpeedOmeter(ES::Engine::Core &core)
{
    const std::string &UITitle = core.GetResource<ES::Plugin::UI::Resource::UIResource>().GetTitle();
    
    if (UITitle == "game")
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
}

void UpdateSpeedOmeterAnimations(ES::Engine::Core &core)
{
    float rpm = 0.0f;
    const float minRPM = 0.0f;
    const float maxRPM = 5000.0f;
    const float minAngle = 0.0f;
    const float maxAngle = 230.0f;
    int gear = 0;
    const int ledCount = 41;
    const std::string &UITitle = core.GetResource<ES::Plugin::UI::Resource::UIResource>().GetTitle();

    if (UITitle == "game")
    {
        core.GetRegistry().view<ES::Plugin::Physics::Component::WheeledVehicle3D>().each(
            [&core, &rpm, &gear](ES::Plugin::Physics::Component::WheeledVehicle3D &vehicle) {
                auto controller = reinterpret_cast<JPH::WheeledVehicleController *>(vehicle.vehicleConstraint->GetController());
                if (controller)
                {
                    rpm = controller->GetEngine().GetCurrentRPM();
                    gear = controller->GetTransmission().GetCurrentGear();
                }
            }
        );
        float t = std::clamp((rpm - minRPM) / (maxRPM - minRPM), 0.0f, 1.0f);
        float angle = minAngle + t * (maxAngle - minAngle);
    
        core.GetResource<ES::Plugin::UI::Resource::UIResource>().SetTransformProperty("speed-counter-pointer", {
            {ES::Plugin::UI::Resource::UIResource::TransformType::Rotate, angle},
        });
        std::string formattedGear = std::to_string(gear);

        if (gear == 0)
            formattedGear = "N";
        else if (gear == -1)
            formattedGear = "R";
        core.GetResource<ES::Plugin::UI::Resource::UIResource>().UpdateInnerContent("gear-current", formattedGear);

        int ledsOn = static_cast<int>(std::round(t * ledCount));
        for (int i = 1; i <= ledCount; i++)
        {
            std::string ledId = "sc-" + (i < 10 ? "0" + std::to_string(i) : std::to_string(i));
            float ledOpacity = (i <= ledsOn ? 1.0f : 0.0f);

            core.GetResource<ES::Plugin::UI::Resource::UIResource>().SetStyleProperty(
                ledId,
                "opacity",
                std::to_string(ledOpacity)
            );
        }
    }
}

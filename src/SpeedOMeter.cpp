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
#include "WheeledVehicleControllerMovement.hpp"

#include "WheeledVehicle3D.hpp"
#include "Logger.hpp"
#include "JoltPhysics.hpp"

constexpr int JOYSTICK_ID = 0;
constexpr int PS5_L3_LR_AXIS = 0;
constexpr int PS5_L3_UD_AXIS = 1;
constexpr int PS5_L2_TRIGGER_AXIS = 3;
constexpr int PS5_R2_TRIGGER_AXIS = 4;
// TODO/ this appears to be the cross button, not the circle...
constexpr int PS5_CIRCLE_BUTTON = 1;

// Tested on PS5 controller on Windows 11
void WheeledVehicleControllerMovement::operator()(ES::Engine::Core &core) const
{
    if (!ES::Plugin::Input::Utils::IsJoystickPresent(JOYSTICK_ID))
    {
        return;
    }

    if (!entity.template HasComponents<ES::Plugin::Physics::Component::WheeledVehicle3D, ES::Plugin::Physics::Component::RigidBody3D>(core))
    {
        ES::Utils::Log::Error(fmt::format("WheeledVehicleControllerMovement component is not fully initialized for entity {}",
                                          static_cast<uint32_t>(entity)));
        return;
    }
    auto &physicsSystem = core.GetResource<ES::Plugin::Physics::Resource::PhysicsManager>().GetPhysicsSystem();

    auto &wheeledVehicle = entity.template GetComponents<ES::Plugin::Physics::Component::WheeledVehicle3D>(core);
    auto &rigidBody = entity.template GetComponents<ES::Plugin::Physics::Component::RigidBody3D>(core);

    auto joystickAxes = ES::Plugin::Input::Utils::GetJoystickAxes(JOYSTICK_ID);
    auto joystickButtons = ES::Plugin::Input::Utils::GetJoystickButtons(JOYSTICK_ID);

    if (joystickAxes.size() < 6)
    {
        return;
    }

    if (joystickButtons.size() < 1)
    {
        return;
    }

    bool shouldReverse = false;

    auto vehicleSpeedJph = rigidBody.body->GetLinearVelocity();
    auto vehicleRotationJph = rigidBody.body->GetRotation().GetXYZW();
    auto vehicleSpeed = glm::vec3(vehicleSpeedJph.GetX(), 0.0f, vehicleSpeedJph.GetZ());
    auto vehicleRotation = glm::quat(vehicleRotationJph.GetX(), vehicleRotationJph.GetY(), vehicleRotationJph.GetZ(),
                                 vehicleRotationJph.GetW());
    
    glm::vec3 forward = vehicleRotation * glm::vec3(0.0f, 0.0f, 1.0f);  // Local Z-axis

    // Dot product to determine direction
    float direction = glm::dot(vehicleSpeed, forward);

    // If dot product is negative, it's moving backwards
    shouldReverse = direction < 0.0f;

    // L2 is throttle and R2 is brake
    // Automatic gearbox : if the car speed is very slow, the brake key is used as reverse, like in most games
    auto throttle = (joystickAxes[PS5_R2_TRIGGER_AXIS] + 1.0f) / 2.0f;
    auto brakeForce = (joystickAxes[PS5_L2_TRIGGER_AXIS] + 1.0f) / 2.0f;
    if (shouldReverse)
    {
        throttle -= brakeForce;
        brakeForce = 0.0f;
    }

    // L3 is left/right steering
    auto steering = joystickAxes[PS5_L3_LR_AXIS];

    // Circle is handbrake
    auto handbrakeForce = (joystickButtons[PS5_CIRCLE_BUTTON] + 1.0f) / 2.0f;
    if (handbrakeForce > 0.51f)
    {
        handbrakeForce = 1.0f;
    }
    else
    {
        handbrakeForce = 0.0f;
    }

    wheeledVehicle.SetDriverInput(throttle, steering, brakeForce, handbrakeForce);

    if (throttle != 0.0f || steering != 0.0f || brakeForce != 0.0f || handbrakeForce != 0.0f)
    {
        printf("Throttle: %.2f, Steering: %.2f, Brake: %.2f, Handbrake: %.2f\n", throttle, steering, brakeForce,
               handbrakeForce);
        physicsSystem.GetBodyInterface().ActivateBody(rigidBody.body->GetID());
    }
}
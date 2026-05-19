#include "VehicleInput.hpp"

#include <cmath>

#include "component/PlayerVehicle.hpp"
#include "component/VehicleController.hpp"
#include "resource/InputManager.hpp"
#include "utils/InputUtils.hpp"

void VehicleInput(Engine::Core &core) {
  auto &registry = core.GetRegistry();

  auto &inputManager = core.GetResource<Input::Resource::InputManager>();

  auto view =
      registry.view<PlayerVehicle, Physics::Component::VehicleController>();

  for (auto entity : view) {
    auto &controller = view.get<Physics::Component::VehicleController>(entity);

    controller.ResetInputs();

    float forward = 0.0f;
    float steering = 0.0f;
    float brake = 0.0f;
    float handbrake = 0.0f;

    if (inputManager.IsKeyPressed(GLFW_KEY_W) ||
        inputManager.IsKeyPressed(GLFW_KEY_Z)) {
      forward += 1.0f;
    }

    if (inputManager.IsKeyPressed(GLFW_KEY_S)) {
      forward -= 1.0f;
    }

    if (inputManager.IsKeyPressed(GLFW_KEY_A) ||
        inputManager.IsKeyPressed(GLFW_KEY_Q)) {
      steering += 1.0f;
    }

    if (inputManager.IsKeyPressed(GLFW_KEY_D)) {
      steering -= 1.0f;
    }

    if (inputManager.IsKeyPressed(GLFW_KEY_B)) {
      brake = 1.0f;
    }

    if (inputManager.IsKeyPressed(GLFW_KEY_SPACE)) {
      handbrake = 1.0f;
    }

    // Controller input
    constexpr int PS5_L3_LR_AXIS = 0;
    constexpr int PS5_L3_UD_AXIS = 1;
    constexpr int PS5_L2_TRIGGER_AXIS = 3;
    constexpr int PS5_R2_TRIGGER_AXIS = 4;
    constexpr int PS5_CROSS_BUTTON = 0;
    constexpr float JOYSTICK_DEADZONE = 0.15f;
    constexpr int JOYSTICK_ID = GLFW_JOYSTICK_1;

    if (Input::Utils::IsJoystickPresent(JOYSTICK_ID)) {
      try {
        auto axes = Input::Utils::GetJoystickAxes(JOYSTICK_ID);

        if (axes.size() >= 5) {
          // Left stick horizontal: steering
          float steeringAxis = axes[PS5_L3_LR_AXIS];
          if (std::abs(steeringAxis) > JOYSTICK_DEADZONE) {
            steering -= steeringAxis;
          }

          // Left stick vertical: forward/backward
          float forwardAxis = -axes[PS5_L3_UD_AXIS];
          if (std::abs(forwardAxis) > JOYSTICK_DEADZONE) {
            forward += forwardAxis;
          }

          // R2 trigger: accelerate (maps from -1..1 to 0..1)
          float r2Value = (axes[PS5_R2_TRIGGER_AXIS] + 1.0f) / 2.0f;
          if (r2Value > JOYSTICK_DEADZONE) {
            forward = std::max(forward, r2Value);
          }

          // L2 trigger: brake (maps from -1..1 to 0..1)
          float l2Value = (axes[PS5_L2_TRIGGER_AXIS] + 1.0f) / 2.0f;
          if (l2Value > JOYSTICK_DEADZONE) {
            brake = std::max(brake, l2Value);
          }
        }

        // Cross button (X on PlayStation): handbrake
        auto buttons = Input::Utils::GetJoystickButtons(JOYSTICK_ID);
        if (buttons.size() > PS5_CROSS_BUTTON) {
          if (buttons[PS5_CROSS_BUTTON] == GLFW_PRESS) {
            handbrake = 1.0f;
          }
        }
      } catch (const std::exception &) {
        // Ignore joystick errors
      }
    }

    controller.SetForward(forward);
    controller.SetSteering(steering);
    controller.SetBrake(brake);
    controller.SetHandBrake(handbrake);
  }
}

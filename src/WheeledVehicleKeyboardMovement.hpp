#pragma once

#include "Engine.hpp"

#include "Input.hpp"

class WheeledVehicleKeyboardMovement
{
  public:
    WheeledVehicleKeyboardMovement(ES::Engine::Entity entity)
        : entity(entity)
    {
    }

    void operator()(ES::Engine::Core &core) const;

    inline void SetForwardKey(int key) { forwardKey = key; }
    inline void SetLeftKey(int key) { leftKey = key; }
    inline void SetRightKey(int key) { rightKey = key; }
    inline void SetReverseKey(int key) { reverseKey = key; }
    inline void SetBrakeKey(int key) { brakeKey = key; }
    inline void SetHandbrakeKey(int key) { handbrakeKey = key; }

  private:
    mutable ES::Engine::Entity entity;
    int forwardKey = GLFW_KEY_W;
    int leftKey = GLFW_KEY_A;
    int rightKey = GLFW_KEY_D;
    int reverseKey = GLFW_KEY_S;
    int brakeKey = GLFW_KEY_Q;
    int handbrakeKey = GLFW_KEY_SPACE;
};
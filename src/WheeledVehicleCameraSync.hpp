#pragma once

#include "Engine.hpp"

#include "Input.hpp"

class WheeledVehicleCameraSync
{
  public:
    WheeledVehicleCameraSync(ES::Engine::Entity entity)
        : entity(entity)
    {
    }

    void operator()(ES::Engine::Core &core) const;

  private:
    mutable ES::Engine::Entity entity;
};
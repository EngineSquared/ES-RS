#pragma once

#include "core/Core.hpp"

#include <tuple>
#include <limits>

ES::Engine::Entity CreateVehicle(ES::Engine::Core &core);

extern ES::Utils::FunctionContainer::FunctionID movementSystemId;
extern ES::Utils::FunctionContainer::FunctionID controllerSystemId;
extern ES::Utils::FunctionContainer::FunctionID cameraSystemId;

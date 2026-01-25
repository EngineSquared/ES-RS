#pragma once

#include "Engine.hpp"

void CreateCheckeredFloor(Engine::Core &core);

void LoadCourse(Engine::Core &core, const std::string &modelPath, const std::string &colliderPath);

Engine::Entity CreateVehicle(Engine::Core &core);

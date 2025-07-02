#pragma once

#include "Core.hpp"
#include "Mesh.hpp"

void LoadCourseModels(ES::Engine::Core &core, const glm::vec3 &position, const glm::quat &rotation, const glm::vec3 &size);

void LoadCourseCollision(ES::Engine::Core &core, const glm::vec3 &size);

void LoadCourseTextures(ES::Engine::Core &core);
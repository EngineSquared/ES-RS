#pragma once

#include "Core.hpp"
#include "Mesh.hpp"

ES::Engine::Entity CreateMap(
    ES::Engine::Core &core,
	const glm::vec3 &position,
	const glm::quat &rotation,
	const glm::vec3 &size);

void LoadCourseTextures(ES::Engine::Core &core);
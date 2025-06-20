#pragma once

#include "Engine.hpp"
#include "Object.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

ES::Plugin::Object::Component::Mesh CreateBoxMesh(const glm::vec3 &size);

ES::Engine::Entity CreateBox(
    ES::Engine::Core &core,
    const glm::vec3 &position = glm::vec3(0.0f, 0.0f, 0.0f),
    const glm::quat &rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
    const glm::vec3 &size = glm::vec3(1.0f, 1.0f, 1.0f)
);

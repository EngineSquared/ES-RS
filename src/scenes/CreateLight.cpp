#include "scenes/CreateLight.hpp"

#include "Object.hpp"

#include <glm/glm.hpp>

Engine::Entity CreateLight(Engine::Core &core)
{
    auto pointLight = core.CreateEntity();
    pointLight.AddComponent<Object::Component::Transform>(glm::vec3(-2.0f, 7.0f, -1.0f));
    pointLight.AddComponent<Object::Component::PointLight>(
        Object::Component::PointLight{.color = glm::vec3(1.0f, 1.0f, 1.0f),
                                      .intensity = 3.0f,
                                      .radius = 100.0f,
                                      .falloff = 1.0f});

    auto ambientLight = core.CreateEntity();
    ambientLight.AddComponent<Object::Component::AmbientLight>(
        Object::Component::AmbientLight{.color = glm::vec3(0.4f, 0.4f, 0.4f)});

    return pointLight;
}
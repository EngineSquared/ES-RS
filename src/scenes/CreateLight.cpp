#include "scenes/CreateLight.hpp"

#include "Object.hpp"

#include <glm/glm.hpp>

Engine::Entity CreateLight(Engine::Core &core) {
  auto pointLight = core.CreateEntity();
  pointLight.AddComponent<Object::Component::Transform>(
      glm::vec3(-2.0f, 7.0f, -1.0f));
  pointLight.AddComponent<Object::Component::PointLight>(
      Object::Component::PointLight{.color = glm::vec3(1.0f, 1.0f, 1.0f),
                                    .intensity = 3.0f,
                                    .radius = 100.0f,
                                    .falloff = 1.0f});

  auto ambientLight = core.CreateEntity();
  ambientLight.AddComponent<Object::Component::AmbientLight>(
      Object::Component::AmbientLight{.color = glm::vec3{0.1f}});

  auto directionalLight = core.CreateEntity();
  directionalLight
      .AddComponent<Object::Component::Transform>(glm::vec3(-5, 3, 1.5))
      .SetRotation(0.408217877, -0.234569684, 0.109381631, 0.875426114);
  directionalLight.AddComponent<Object::Component::DirectionalLight>(
      {.color = glm::vec4(0.5f, 0.35f, 0.05f, 1.0f),
       .projection =
           glm::orthoLH_ZO(-40.0f, 40.0f, -40.0f, 40.0f, 0.1f, 60.0f)});

  return pointLight;
}

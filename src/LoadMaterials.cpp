#include "LoadMaterials.hpp"

#include "OpenGL.hpp"

#include <entt/entt.hpp>

void LoadMaterials(ES::Engine::Core &core)
{
    using namespace entt;
    auto &materialManager = core.GetResource<ES::Plugin::OpenGL::Resource::MaterialCache>();
	auto material = ES::Plugin::OpenGL::Utils::Material();

    // floor: light grey
	material.Ka = glm::vec3(0.6f, 0.6f, 0.6f);
    material.Kd = glm::vec3(0.8f, 0.8f, 0.8f);
    material.Ks = glm::vec3(0.5f, 0.5f, 0.5f);
    material.Shiness = 64.0f;

	materialManager.Add("floor"_hs, material);

    // car body: red
    material.Ka = glm::vec3(0.8f, 0.0f, 0.0f);
    material.Kd = glm::vec3(0.8f, 0.0f, 0.0f);
    material.Ks = glm::vec3(0.5f, 0.5f, 0.5f);
    material.Shiness = 64.0f;
    materialManager.Add("car_body"_hs, material);

    // car wheel: blue
    material.Ka = glm::vec3(0.0f, 0.0f, 0.8f);
    material.Kd = glm::vec3(0.0f, 0.0f, 0.8f);
    material.Ks = glm::vec3(0.5f, 0.5f, 0.5f);
    material.Shiness = 64.0f;

    materialManager.Add("car_wheel"_hs, material);
}

#pragma once

#include "Scene.hpp"

using namespace ES::Plugin;

class Game : public ES::Plugin::Scene::Utils::AScene {

public:
    Game() : ES::Plugin::Scene::Utils::AScene() {}

protected:
    void _onCreate(ES::Engine::Core &core) final
    {
        CreateFloor(core);
        CreateVehicle(core);

        AddLights(core, "default");
        AddLights(core, "noTextureLightShadow");
    }

    void _onDestroy(ES::Engine::Core &core) final
    {
        core.ClearEntities();
    }

private:
    void AddLights(ES::Engine::Core &core, const std::string &shaderName)
    {
        ES::Engine::Entity ambient_light = core.CreateEntity();
        ambient_light.AddComponent<OpenGL::Component::ShaderHandle>(core, shaderName);
        ambient_light.AddComponent<Object::Component::Transform>(core);
        ambient_light.AddComponent<OpenGL::Component::Light>(core, OpenGL::Component::Light::Type::AMBIENT, glm::vec3(0.2f, 0.2f, 0.2f));

        ES::Engine::Entity light_1 = core.CreateEntity();
        light_1.AddComponent<OpenGL::Component::ShaderHandle>(core, shaderName);
        light_1.AddComponent<Object::Component::Transform>(core, glm::vec3(3.0f, 20.0f, 0.0f));
        light_1.AddComponent<OpenGL::Component::Light>(core, OpenGL::Component::Light::Type::POINT, glm::vec3(1.f, 1.f, 1.f));
    }
};
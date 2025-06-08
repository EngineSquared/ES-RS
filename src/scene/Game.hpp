#pragma once

#include "Engine.pch.hpp"

#include "Scene.hpp"
#include "CreateFloor.hpp"
#include "CreateVehicle.hpp"

#include "Timer.hpp"

using namespace ES::Plugin;

struct StartupCircuitTimer {
    Timer timer;
};

struct GameChrono {
    Timer timer;
};

void AddChronoDisplay(ES::Engine::Core &core)
{
    core.GetResource<ES::Plugin::OpenGL::Resource::FontManager>().Add(
        entt::hashed_string("tomorrow"),
        ES::Plugin::OpenGL::Utils::Font("asset/font/Tomorrow-Medium.ttf", 32)
    );

    auto timeElapsedText = ES::Engine::Entity::Create(core);

    timeElapsedText.AddComponent<ES::Plugin::UI::Component::Text>(core, "Time elapsed: 0.0s", glm::vec2(10.0f, 10.0f), 1.0f, ES::Plugin::Colors::Utils::WHITE_COLOR);
    timeElapsedText.AddComponent<ES::Plugin::OpenGL::Component::FontHandle>(core, "tomorrow");
    timeElapsedText.AddComponent<ES::Plugin::OpenGL::Component::ShaderHandle>(core, "textDefault");
    timeElapsedText.AddComponent<ES::Plugin::OpenGL::Component::TextHandle>(core, "chronoText");
    timeElapsedText.AddComponent<GameChrono>(core, Timer(1.f).SetInfinite(true));
}

void UpdateTextTime(ES::Engine::Core &core)
{
    auto dt = core.GetScheduler<ES::Engine::Scheduler::Update>().GetDeltaTime();

    core.GetRegistry()
        .view<GameChrono>()
        .each([&dt](auto, auto &chrono) {
            chrono.timer.Update(dt);
        });
    core.GetRegistry()
        .view<ES::Plugin::OpenGL::Component::TextHandle, ES::Plugin::UI::Component::Text, GameChrono>()
        .each([](auto, auto &textHandle, auto &text, auto &chrono) {
            if (textHandle.name == "chronoText")
            {
                text.text = fmt::format("Time elapsed: {:.2f}s", chrono.timer.elapsed);
            }
        });
}

void StartupCircuitTimerUpdate(ES::Engine::Core &core)
{
    auto dt = core.GetScheduler<ES::Engine::Scheduler::Update>().GetDeltaTime();
    core.GetRegistry()
        .view<StartupCircuitTimer>()
        .each([&core, &dt](auto e, auto &startupCircuitTimer) {
            auto &timer = startupCircuitTimer.timer;
            timer.Update(dt);
            if (timer.JustCompleted()) {
                ES::Utils::Log::Info(fmt::format("Circuit timer just completed after {} seconds", timer.elapsed));
            }
            if (timer.Completed()) {
                ES::Utils::Log::Info(fmt::format("Circuit timer completed after {} seconds", timer.elapsed));
                ES::Engine::Entity(e).Destroy(core);
                core.RegisterSystem<ES::Engine::Scheduler::Update>(UpdateTextTime);
            }
        });
}

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
        CreateStartChrono(core);
        AddChronoDisplay(core);
    }

    void _onDestroy(ES::Engine::Core &core) final
    {
        core.ClearEntities();
    }

private:
    void CreateStartChrono(ES::Engine::Core &core)
    {
        ES::Engine::Entity chrono = core.CreateEntity();

        chrono.AddComponent<StartupCircuitTimer>(core, Timer(1.f).SetIterations(3));

        core.RegisterSystem<ES::Engine::Scheduler::Update>(StartupCircuitTimerUpdate);
    }

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
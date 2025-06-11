#pragma once

#include "Engine.pch.hpp"

#include "Scene.hpp"
#include "CreateFloor.hpp"
#include "CreateVehicle.hpp"

#include "UI.hpp"
#include "Timer.hpp"

using namespace ES::Plugin;

struct StartupCircuitTimer {
    Timer timer;
};

struct GameChrono {
    Timer timer;
};

class Game : public ES::Plugin::Scene::Utils::AScene {
public:
    Game() : _gameChrono{GameChrono(Timer(1.0f).SetInfinite(true))}, _startupCircuitChrono{StartupCircuitTimer(Timer(1.f).SetIterations(3))}, _IsCountingDown(true)
    {}

    void AddChronoDisplay(ES::Engine::Core &core)
    {
        core.RegisterSystem<ES::Engine::Scheduler::Update>(
            [this](ES::Engine::Core &core) {
                this->StartupCircuitTimerUpdate(core);
            }
        );
    }

    void UpdateTextTime(ES::Engine::Core &core)
    {
        auto dt = core.GetScheduler<ES::Engine::Scheduler::Update>().GetDeltaTime();

        _gameChrono.timer.Update(dt);

        std::ostringstream timeStream;
        timeStream << std::fixed << std::setprecision(3) << _gameChrono.timer.elapsed;

        core.GetResource<ES::Plugin::UI::Resource::UIResource>().UpdateInnerContent("time-value", timeStream.str());
    }
    
    void StartupCircuitTimerUpdate(ES::Engine::Core &core)
    {
        auto dt = core.GetScheduler<ES::Engine::Scheduler::Update>().GetDeltaTime();
        
        _startupCircuitChrono.timer.Update(dt);
        if (_startupCircuitChrono.timer.JustCompleted()) {
            ES::Utils::Log::Info(fmt::format("Circuit timer just completed after {} seconds", _startupCircuitChrono.timer.elapsed));
        }
        if (_startupCircuitChrono.timer.Completed() && _IsCountingDown) {
            ES::Utils::Log::Info(fmt::format("Circuit timer completed after {} seconds", _startupCircuitChrono.timer.elapsed));
            core.RegisterSystem<ES::Engine::Scheduler::Update>(
                [this](ES::Engine::Core &core) { this->UpdateTextTime(core); }
            );
            _IsCountingDown = false;
        }
    }

protected:
    void _onCreate(ES::Engine::Core &core) final
    {
        CreateFloor(core);
        CreateVehicle(core);

        AddLights(core, "default");
        AddLights(core, "noTextureLightShadow");
        AddChronoDisplay(core);
    }

    void _onDestroy(ES::Engine::Core &core) final
    {
        core.ClearEntities();
    }

private:
    GameChrono _gameChrono;
    StartupCircuitTimer _startupCircuitChrono;
    bool _IsCountingDown;

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
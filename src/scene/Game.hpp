#pragma once

#include "Engine.pch.hpp"

#include "Scene.hpp"
#include "CreateRace.hpp"
#include "LoadCourse.hpp"
#include "CreateVehicle.hpp"
#include "SpeedOMeter.hpp"
#include "utils/CreateSkyBox.hpp"
#include "PauseGame.hpp"

#include "UI.hpp"
#include "Timer.hpp"

#include <limits>

using namespace ES::Plugin;

struct StartupCircuitTimer {
    Timer timer;
};

struct GameChrono {
    Timer timer;
};

namespace Game
{
class Race : public ES::Plugin::Scene::Utils::AScene {
public:
    Race() : _gameChrono{GameChrono(Timer(1.0f).SetInfinite(true))}, _startupCircuitChrono{StartupCircuitTimer(Timer(1.f).SetIterations(3))}, _IsCountingDown(true)
    {}

    void AddChronoDisplay(ES::Engine::Core &core)
    {
        if (_chronoSystemId == std::numeric_limits<std::size_t>::max())
            _chronoSystemId = std::get<0>(core.RegisterSystem<ES::Engine::Scheduler::Update>(
                [this](ES::Engine::Core &core) {
                    auto &uiResource = core.GetResource<ES::Plugin::UI::Resource::UIResource>();
                    if (uiResource.GetTitle() == "game")
                    {
                        const auto &visibility = uiResource.GetStyle("pause-menu", "visibility");
                        if (visibility == "hidden")
                            this->StartupCircuitTimerUpdate(core);
                    }
                }
            ));
    }

    void UpdateTextTime(ES::Engine::Core &core)
    {
        auto &uiResource = core.GetResource<ES::Plugin::UI::Resource::UIResource>();

        if (uiResource.GetTitle() == "game")
        {
            const auto &visibility = uiResource.GetStyle("pause-menu", "visibility");
            if (visibility == "hidden")
            {
                auto dt = core.GetScheduler<ES::Engine::Scheduler::Update>().GetDeltaTime();
                _gameChrono.timer.Update(dt);
        
                double elapsed = _gameChrono.timer.elapsed;
                int minutes = static_cast<int>(elapsed / 60.0);
                int seconds = static_cast<int>(elapsed) % 60;
                int milliseconds = static_cast<int>((elapsed - static_cast<int>(elapsed)) * 1000);
        
                std::ostringstream timeStream;
                timeStream << std::setfill('0')
                        << std::setw(2) << minutes << ":"
                        << std::setw(2) << seconds << ":"
                        << std::setw(3) << milliseconds;
        
                auto &uiResource = core.GetResource<ES::Plugin::UI::Resource::UIResource>();
                if (uiResource.GetTitle() == "game")
                    uiResource.UpdateInnerContent("time-value", timeStream.str());
            }
        }
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

    /* Manually binding to keep track of the FixedUpdate Scheduler systems */
    void BindPhysicPlugin(ES::Engine::Core &core)
    {
        if (_syncRigidBodiesToTransformsSystemId == std::numeric_limits<std::size_t>::max())
            std::tie(_syncRigidBodiesToTransformsSystemId, _syncPhysicUpdateSystemId,
                _syncTransformsToRigidBodiesSystemId, _syncSoftBodiesDataSystemId,
                _syncWheeledVehicleWheelsSystemId) = core.RegisterSystem<ES::Engine::Scheduler::FixedTimeUpdate>(
                ES::Plugin::Physics::System::SyncRigidBodiesToTransforms, ES::Plugin::Physics::System::PhysicsUpdate,
                ES::Plugin::Physics::System::SyncTransformsToRigidBodies, ES::Plugin::Physics::System::SyncSoftBodiesData,
                ES::Plugin::Physics::System::SyncWheeledVehicleWheels);
    }

protected:
    void _onCreate(ES::Engine::Core &core) final
    {
        BindPhysicPlugin(core);
        CreateRace(core);
        CreateVehicle(core);
        _gameChrono.timer.elapsed = 0.0f;

        std::array<std::string, 6> faces = {
            "asset/skybox/right.jpg",
            "asset/skybox/left.jpg",
            "asset/skybox/top.jpg",
            "asset/skybox/bottom.jpg",
            "asset/skybox/front.jpg",
            "asset/skybox/back.jpg"
        };
        ES::Plugin::OpenGL::Utils::CreateSkyBox(core, faces);

        AddLights(core, "default");
        AddLights(core, "noTextureLightShadow");
        AddChronoDisplay(core);

        core.GetResource<UI::Resource::UIResource>().InitDocument("asset/ui/race/game.rml");
        core.GetResource<UI::Resource::UIResource>().AttachEventHandlers("fade-out-mask", "animationend", [&core, this](const std::string &event, const std::string &elementId) {
            if (elementId == "fade-out-mask" && event == "animationend") {
                auto &soundManager = core.GetResource<ES::Plugin::Sound::Resource::SoundManager>();
                soundManager.Stop("race-ambient");
                soundManager.Stop("race-ambient-life");
                const auto &tmp = std::numeric_limits<std::size_t>::max();
                if (controllerSystemId != std::numeric_limits<std::size_t>::max())
                {
                    core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().Disable(controllerSystemId);
                    core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().Disable(movementSystemId);
                    core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().Disable(cameraSystemId);
                }
                core.RunSystems();
                core.GetResource<ES::Plugin::Scene::Resource::SceneManager>().SetNextScene("main-menu");
            }
        });
        core.GetResource<UI::Resource::UIResource>().AttachEventHandlers("resume-btn", "click",
            [&core](const std::string &event, const std::string &elementId) {
                if (elementId == "resume-btn" && event == "click") {
                    auto &soundManager = core.GetResource<ES::Plugin::Sound::Resource::SoundManager>();
                    soundManager.Play("button_click");
                    core.GetResource<UI::Resource::UIResource>().SetStyleProperty("pause-menu", "visibility", "hidden");
                }
            }
        );
        core.GetResource<UI::Resource::UIResource>().AttachEventHandlers("resume-btn", "mouseover",
            [&core](const std::string &event, const std::string &elementId) {
                if (elementId == "resume-btn" && event == "mouseover") {
                    auto &soundManager = core.GetResource<ES::Plugin::Sound::Resource::SoundManager>();
                    if (soundManager.IsPlaying("button_hover"))
                        soundManager.Stop("button_hover");
                    soundManager.Play("button_hover");
                }
            }
        );
        core.GetResource<UI::Resource::UIResource>().AttachEventHandlers("quit-btn", "click",
            [&core, this](const std::string &event, const std::string &elementId) {
                if (elementId == "quit-btn" && event == "click") {
                    auto &soundManager = core.GetResource<ES::Plugin::Sound::Resource::SoundManager>();
                    soundManager.Play("button_click");
                    core.GetResource<UI::Resource::UIResource>().SetStyleProperty("fade-out-mask", "visibility", "visible");
                    core.GetResource<UI::Resource::UIResource>().SetStyleProperty("fade-out-mask", "animation", "0.6s linear-in-out 1 fade-out");
                    // Enable back the systems to prevent duplicate disabling
                    core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().Enable(controllerSystemId);
                    core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().Enable(movementSystemId);
                    core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().Enable(_syncRigidBodiesToTransformsSystemId);
                    core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().Enable(_syncPhysicUpdateSystemId);
                    core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().Enable(_syncTransformsToRigidBodiesSystemId);
                    core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().Enable(_syncSoftBodiesDataSystemId);
                    core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().Enable(_syncWheeledVehicleWheelsSystemId);
                    core.RunSystems();
                }
            }
        );
        core.GetResource<UI::Resource::UIResource>().AttachEventHandlers("quit-btn", "mouseover",
            [&core](const std::string &event, const std::string &elementId) {
                if (elementId == "quit-btn" && event == "mouseover") {
                    auto &soundManager = core.GetResource<ES::Plugin::Sound::Resource::SoundManager>();
                    if (soundManager.IsPlaying("button_hover"))
                        soundManager.Stop("button_hover");
                    soundManager.Play("button_hover");
                }
            }
        );
        
        auto &soundManager = core.GetResource<ES::Plugin::Sound::Resource::SoundManager>();
        soundManager.Play("race-ambient");
        soundManager.Play("race-ambient-life");

        if (_speedometerSystemId == std::numeric_limits<std::size_t>::max())
        {
            std::tie(_speedometerSystemId, _speedometerAnimSystemId, _pauseMenuSystemId) =
                core.RegisterSystem<ES::Engine::Scheduler::FixedTimeUpdate>(
                    UpdateSpeedOmeter,
                    UpdateSpeedOmeterAnimations,
                    TogglePauseMenu
                );
                core.RegisterSystem<ES::Engine::Scheduler::FixedTimeUpdate>(
                    [this](ES::Engine::Core &core) {
                        auto &uiResource = core.GetResource<ES::Plugin::UI::Resource::UIResource>();
                        auto &systems = core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().GetSystems();

                        if (uiResource.GetTitle() == "game")
                        {
                            if (!_gamePaused && uiResource.GetStyle("pause-menu", "visibility") == "visible") {
                                _gamePaused = true;
                                core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().Disable(controllerSystemId);
                                core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().Disable(movementSystemId);
                                core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().Disable(_syncRigidBodiesToTransformsSystemId);
                                core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().Disable(_syncPhysicUpdateSystemId);
                                core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().Disable(_syncTransformsToRigidBodiesSystemId);
                                core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().Disable(_syncSoftBodiesDataSystemId);
                                core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().Disable(_syncWheeledVehicleWheelsSystemId);
                                core.RunSystems();
                            } else if (_gamePaused && uiResource.GetStyle("pause-menu", "visibility") == "hidden") {
                                _gamePaused = false;
                                core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().Enable(controllerSystemId);
                                core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().Enable(movementSystemId);
                                core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().Enable(_syncRigidBodiesToTransformsSystemId);
                                core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().Enable(_syncPhysicUpdateSystemId);
                                core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().Enable(_syncTransformsToRigidBodiesSystemId);
                                core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().Enable(_syncSoftBodiesDataSystemId);
                                core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().Enable(_syncWheeledVehicleWheelsSystemId);
                                core.RunSystems();
                            }
                        }
                    }
                );
            };
    }

    void _onDestroy(ES::Engine::Core &core) final
    {
        auto &cubeMapManager = core.GetResource<OpenGL::Resource::CubeMapManager>();

        core.ClearEntities();
        UnloadCourseTextures(core);
        const std::string unique_resource_id = "cubemap_faces_right";
        cubeMapManager.Remove(entt::hashed_string{unique_resource_id.c_str()});

        core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().Disable(_syncRigidBodiesToTransformsSystemId);
        core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().Disable(_syncPhysicUpdateSystemId);
        core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().Disable(_syncTransformsToRigidBodiesSystemId);
        core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().Disable(_syncSoftBodiesDataSystemId);
        core.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().Disable(_syncWheeledVehicleWheelsSystemId);
    }

private:
    GameChrono _gameChrono;
    StartupCircuitTimer _startupCircuitChrono;
    bool _IsCountingDown;
    mutable ES::Utils::FunctionContainer::FunctionID _chronoSystemId = std::numeric_limits<std::size_t>::max();
    ES::Utils::FunctionContainer::FunctionID _speedometerSystemId = std::numeric_limits<std::size_t>::max();
    ES::Utils::FunctionContainer::FunctionID _speedometerAnimSystemId = std::numeric_limits<std::size_t>::max();
    ES::Utils::FunctionContainer::FunctionID _pauseMenuSystemId = std::numeric_limits<std::size_t>::max();
    ES::Utils::FunctionContainer::FunctionID _syncRigidBodiesToTransformsSystemId = std::numeric_limits<std::size_t>::max();
    ES::Utils::FunctionContainer::FunctionID _syncPhysicUpdateSystemId = std::numeric_limits<std::size_t>::max();
    ES::Utils::FunctionContainer::FunctionID _syncTransformsToRigidBodiesSystemId = std::numeric_limits<std::size_t>::max();
    ES::Utils::FunctionContainer::FunctionID _syncSoftBodiesDataSystemId = std::numeric_limits<std::size_t>::max();
    ES::Utils::FunctionContainer::FunctionID _syncWheeledVehicleWheelsSystemId = std::numeric_limits<std::size_t>::max();

    bool _gamePaused = false;
   
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
} // namespace Game

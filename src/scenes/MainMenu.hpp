#pragma once

#include "resource/UIContext.hpp"
#include "resource/SceneManager.hpp"
#include "resource/SoundManager.hpp"
#include "scheduler/Update.hpp"
#include "utils/AScene.hpp"
#include "Logger.hpp"
#include <memory>

namespace Game
{
class MainMenu : public Scene::Utils::AScene {
public:
    MainMenu() {};

protected:
    void _onCreate(Engine::Core &core) final
    {
        auto &uiContext = core.GetResource<Rmlui::Resource::UIContext>();
        auto &soundManager = core.GetResource<Sound::Resource::SoundManager>();

        soundManager.RegisterSound("start-menu", "asset/sounds/start-menu.mp3");
        soundManager.RegisterSound("button_hover", "asset/sounds/btn-hover.mp3");
        soundManager.RegisterSound("button_click", "asset/sounds/btn-click.mp3");
        soundManager.RegisterSound("main-menu", "asset/sounds/main-menu.mp3", true);
        soundManager.RegisterSound("race-ambient", "asset/sounds/race-amb.mp3", true);
        soundManager.RegisterSound("race-ambient-life", "asset/sounds/race-amb-life.mp3", true);
        soundManager.RegisterSound("pause-menu", "asset/sounds/pause-menu.mp3");
        
        soundManager.SetVolume("start-menu", 0.2f);
        soundManager.SetVolume("button_hover", 0.6f);
        soundManager.SetVolume("button_click", 0.6f);
        soundManager.SetVolume("main-menu", 0.4f);
        soundManager.SetVolume("race-ambient", 0.02f);
        soundManager.SetVolume("race-ambient-life", 0.3f);
        soundManager.SetVolume("pause-menu", 0.3f);

        uiContext.SetFont("asset/font/Tomorrow-Medium.ttf");
        uiContext.SetFont("asset/font/airborne.ttf");
        uiContext.LoadDocument("asset/ui/main-menu/splash-screen.rml");
        auto *interactionArea = uiContext.GetElementById("interaction-area");
        if (interactionArea == nullptr) {
            Log::Error("MainMenu: missing 'interaction-area' in splash-screen.rml");
            return;
        }

        auto loadMainMenuRequested = std::make_shared<bool>(false);
        auto loadMainMenuCompleted = std::make_shared<bool>(false);

        core.RegisterSystem<Engine::Scheduler::Update>(
            [loadMainMenuRequested, loadMainMenuCompleted](Engine::Core &core) {
                if (!*loadMainMenuRequested || *loadMainMenuCompleted) {
                    return;
                }
                *loadMainMenuCompleted = true;
                *loadMainMenuRequested = false;

                auto &uiContext = core.GetResource<Rmlui::Resource::UIContext>();
                auto &soundManager = core.GetResource<Sound::Resource::SoundManager>();

                uiContext.LoadDocument("asset/ui/main-menu/main-menu.rml");

                auto *startGameBtn = uiContext.GetElementById("start-game-btn");
                auto *fadeOutMask = uiContext.GetElementById("fade-out-mask");
                auto *quitGameBtn = uiContext.GetElementById("quit-game-btn");
                if (startGameBtn == nullptr || fadeOutMask == nullptr || quitGameBtn == nullptr) {
                    Log::Error("MainMenu: missing UI elements in main-menu.rml");
                    return;
                }

                uiContext.RegisterEventListener(*startGameBtn, "click", [fadeOutMask, &uiContext, &core, &soundManager](Rml::Event &event) {
                    //soundManager.Play("button_click");
                    fadeOutMask->SetProperty("visibility", "visible");
                    fadeOutMask->SetProperty("animation", "0.6s linear-in-out 1 fade-out");
                });
                uiContext.RegisterEventListener(*fadeOutMask, "animationend", [&uiContext, &core, &soundManager](Rml::Event &event) {
                    //soundManager.Stop("main-menu");
                    core.GetResource<Scene::Resource::SceneManager>().SetNextScene("CourseScene");
                });
                uiContext.RegisterEventListener(*startGameBtn, "mouseover", [&uiContext, &core, &soundManager](Rml::Event &event) {
                    //if (soundManager.IsPlaying("button_hover"))
                    //    soundManager.Stop("button_hover");
                    //soundManager.Play("button_hover");
                });
                uiContext.RegisterEventListener(*quitGameBtn, "click", [&uiContext, &core, &soundManager](Rml::Event &event) {
                    //soundManager.Play("button_click");
                    core.Stop();
                });
                uiContext.RegisterEventListener(*quitGameBtn, "mouseover", [&uiContext, &core, &soundManager](Rml::Event &event) {
                    //if (soundManager.IsPlaying("button_hover"))
                    //    soundManager.Stop("button_hover");
                    //soundManager.Play("button_hover");
                });
            });

        uiContext.RegisterEventListener(*interactionArea, "animationend",
            [loadMainMenuRequested](Rml::Event &event) {
                *loadMainMenuRequested = true;
            });
        uiContext.RegisterEventListener(*interactionArea, "click", [&uiContext, &core, &soundManager](Rml::Event &event) {
            //soundManager.Play("start-menu");
            event.GetTargetElement()->SetProperty("animation", "0.6s linear-in-out 1 fade-out");
        });
        //soundManager.Play("main-menu");
    }

    void _onDestroy(Engine::Core &core) final
    {
        core.GetResource<Rmlui::Resource::UIContext>().Destroy(core); // TEMPORARY
    }
};
} // namespace Game
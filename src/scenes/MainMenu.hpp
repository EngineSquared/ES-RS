#pragma once

#include "resource/UIContext.hpp"
#include "resource/SceneManager.hpp"
#include "resource/SoundManager.hpp"
#include "utils/AScene.hpp"
#include "Logger.hpp"

namespace Game
{
class MainMenu : public Scene::Utils::AScene {
public:
    MainMenu() {};

protected:
    void _onCreate(Engine::Core &core) final
    {
        Log::Info("Creating MainMenu");
        auto &uiContext = core.GetResource<Rmlui::Resource::UIContext>();
        uiContext.SetFont("asset/font/Tomorrow-Medium.ttf");
        uiContext.SetFont("asset/font/airborne.ttf");
        uiContext.LoadDocument("asset/ui/main-menu/splash-screen.rml");
        uiContext.RegisterEventListener(*uiContext.GetElementById("interaction-area"), "animationend", [&uiContext, &core](Rml::Event &event) {
            if (event.GetTargetElement()->GetId() == "interaction-area" && event.GetType() == "animationend") {
                uiContext.LoadDocument("asset/ui/main-menu/main-menu.rml");
                uiContext.RegisterEventListener(*uiContext.GetElementById("start-game-btn"), "click", [&uiContext, &core](Rml::Event &event) {
                    if (event.GetTargetElement()->GetId() == "start-game-btn" && event.GetType() == "click") {
                        auto &soundManager = core.GetResource<Sound::Resource::SoundManager>();
                        soundManager.Play("button_click");
                        event.GetTargetElement()->SetProperty("visibility", "visible");
                        event.GetTargetElement()->SetProperty("animation", "0.6s linear-in-out 1 fade-out");
                    }
                });
                uiContext.RegisterEventListener(*uiContext.GetElementById("fade-out-mask"), "animationend", [&uiContext, &core](Rml::Event &event) {
                    if (event.GetTargetElement()->GetId() == "fade-out-mask" && event.GetType() == "animationend") {
                        auto &soundManager = core.GetResource<Sound::Resource::SoundManager>();
                        soundManager.Stop("main-menu");
                        core.ClearEntities();
                        core.GetResource<Scene::Resource::SceneManager>().SetNextScene("CourseScene");
                    }
                });
                uiContext.RegisterEventListener(*uiContext.GetElementById("start-game-btn"), "mouseover", [&uiContext, &core](Rml::Event &event) {
                    if (event.GetTargetElement()->GetId() == "start-game-btn" && event.GetType() == "mouseover") {
                        auto &soundManager = core.GetResource<Sound::Resource::SoundManager>();
                        if (soundManager.IsPlaying("button_hover"))
                            soundManager.Stop("button_hover");
                        soundManager.Play("button_hover");
                    }
                });
                uiContext.RegisterEventListener(*uiContext.GetElementById("quit-game-btn"), "click", [&uiContext, &core](Rml::Event &event) {
                    if (event.GetTargetElement()->GetId() == "quit-game-btn" && event.GetType() == "click") {
                        auto &soundManager = core.GetResource<Sound::Resource::SoundManager>();
                        soundManager.Play("button_click");
                        core.Stop();
                    }
                });
                uiContext.RegisterEventListener(*uiContext.GetElementById("quit-game-btn"), "mouseover", [&uiContext, &core](Rml::Event &event) {
                    if (event.GetTargetElement()->GetId() == "quit-game-btn" && event.GetType() == "mouseover") {
                        auto &soundManager = core.GetResource<Sound::Resource::SoundManager>();
                        if (soundManager.IsPlaying("button_hover"))
                            soundManager.Stop("button_hover");
                        soundManager.Play("button_hover");
                    }
                });
            }
        });
        uiContext.RegisterEventListener(*uiContext.GetElementById("interaction-area"), "click", [&uiContext, &core](Rml::Event &event) {
            if (event.GetTargetElement()->GetId() == "interaction-area" && event.GetType() == "click") {
                auto &soundManager = core.GetResource<Sound::Resource::SoundManager>();
                soundManager.Play("start-menu");
                event.GetTargetElement()->SetProperty("animation", "0.6s linear-in-out 1 fade-out");
            }
        });
        auto &soundManager = core.GetResource<Sound::Resource::SoundManager>();
        soundManager.Play("main-menu");
    }

    void _onDestroy(Engine::Core &core) final
    {
        core.ClearEntities();
    }
};
} // namespace Game
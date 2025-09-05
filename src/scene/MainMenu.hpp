#pragma once

#include "Engine.pch.hpp"

#include "Scene.hpp"
#include "CreateFloor.hpp"
#include "CreateVehicle.hpp"

#include "UI.hpp"
#include "Timer.hpp"

using namespace ES::Plugin;

namespace Game
{
class MainMenu : public ES::Plugin::Scene::Utils::AScene {
public:
    MainMenu() {};

protected:
    void _onCreate(ES::Engine::Core &core) final
    {
        core.GetResource<UI::Resource::UIResource>().SetFont("asset/font/Tomorrow-Medium.ttf");
        core.GetResource<UI::Resource::UIResource>().SetFont("asset/font/airborne.ttf");
        core.GetResource<UI::Resource::UIResource>().InitDocument("asset/ui/main-menu/splash-screen.rml");
        core.GetResource<UI::Resource::UIResource>().AttachEventHandlers("interaction-area", "animationend", [&core](const std::string &event, const std::string &elementId) {
            if (elementId == "interaction-area" && event == "animationend") {
                core.GetResource<UI::Resource::UIResource>().InitDocument("asset/ui/main-menu/main-menu.rml");
                core.GetResource<UI::Resource::UIResource>().AttachEventHandlers("start-game-btn", "click", [&core](const std::string &event, const std::string &elementId) {
                    if (elementId == "start-game-btn" && event == "click") {
                        auto &soundManager = core.GetResource<ES::Plugin::Sound::Resource::SoundManager>();
                        if (soundManager.IsPlaying("button_click"))
                            soundManager.Stop("button_click");
                        soundManager.Play("button_click");
                        core.GetResource<UI::Resource::UIResource>().SetStyleProperty("fade-out-mask", "visibility", "visible");
                        core.GetResource<UI::Resource::UIResource>().SetStyleProperty("fade-out-mask", "animation", "0.6s linear-in-out 1 fade-out");
                    }
                });
                core.GetResource<UI::Resource::UIResource>().AttachEventHandlers("fade-out-mask", "animationend", [&core](const std::string &event, const std::string &elementId) {
                    if (elementId == "fade-out-mask" && event == "animationend") {
                        auto &soundManager = core.GetResource<ES::Plugin::Sound::Resource::SoundManager>();
                        soundManager.Stop("main-menu");
                        core.ClearEntities();
                        core.GetResource<ES::Plugin::Scene::Resource::SceneManager>().SetNextScene("race");
                    }
                });
                core.GetResource<UI::Resource::UIResource>().AttachEventHandlers("start-game-btn", "mouseover", [&core](const std::string &event, const std::string &elementId) {
                    if (elementId == "start-game-btn" && event == "mouseover") {
                        auto &soundManager = core.GetResource<ES::Plugin::Sound::Resource::SoundManager>();
                        if (soundManager.IsPlaying("button_hover"))
                            soundManager.Stop("button_hover");
                        soundManager.Play("button_hover");
                    }
                });
                core.GetResource<UI::Resource::UIResource>().AttachEventHandlers("quit-game-btn", "click", [&core](const std::string &event, const std::string &elementId) {
                    if (elementId == "quit-game-btn" && event == "click") {
                        auto &soundManager = core.GetResource<ES::Plugin::Sound::Resource::SoundManager>();
                        if (soundManager.IsPlaying("button_click"))
                            soundManager.Stop("button_click");
                        soundManager.Play("button_click");
                        core.Stop();
                    }
                });
                core.GetResource<UI::Resource::UIResource>().AttachEventHandlers("quit-game-btn", "mouseover", [&core](const std::string &event, const std::string &elementId) {
                    if (elementId == "quit-game-btn" && event == "mouseover") {
                        auto &soundManager = core.GetResource<ES::Plugin::Sound::Resource::SoundManager>();
                        if (soundManager.IsPlaying("button_hover"))
                            soundManager.Stop("button_hover");
                        soundManager.Play("button_hover");
                    }
                });
            }
        });
        core.GetResource<UI::Resource::UIResource>().AttachEventHandlers("interaction-area", "click", [&core](const std::string &event, const std::string &elementId) {
            if (elementId == "interaction-area" && event == "click") {
                auto &soundManager = core.GetResource<ES::Plugin::Sound::Resource::SoundManager>();
                soundManager.Play("start-menu");
                core.GetResource<UI::Resource::UIResource>().SetStyleProperty("interaction-area", "animation", "0.6s linear-in-out 1 fade-out");
            }
        });
        auto &soundManager = core.GetResource<ES::Plugin::Sound::Resource::SoundManager>();
        soundManager.Play("main-menu");
    }

    void _onDestroy(ES::Engine::Core &core) final
    {
        core.ClearEntities();
    }

private:
};
} // namespace Game

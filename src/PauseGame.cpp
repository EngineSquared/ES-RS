#include "PauseGame.hpp"
#include "Input.hpp"
#include "resource/UIResource.hpp"
#include "Logger.hpp"
#include "resource/SoundManager.hpp"

void TogglePauseMenu(ES::Engine::Core &core)
{
    auto &uiResource = core.GetResource<ES::Plugin::UI::Resource::UIResource>();
    auto &soundManager = core.GetResource<ES::Plugin::Sound::Resource::SoundManager>();
    static bool escapeWasPressed = false;

    if (uiResource.GetTitle() == "game")
    {
        const auto &visibility = uiResource.GetStyle("pause-menu", "visibility");
        bool escapeIsPressed = ES::Plugin::Input::Utils::IsKeyPressed(GLFW_KEY_ESCAPE);

        if (escapeIsPressed && !escapeWasPressed)
        {
            if (visibility == "hidden") {
                uiResource.SetStyleProperty("pause-menu", "animation", "0.2s quadratic-out 1 slide-in");
                uiResource.SetStyleProperty("pause-menu", "visibility", "visible");
                if (soundManager.IsPlaying("pause-menu"))
                    soundManager.Stop("pause-menu");
                soundManager.Play("pause-menu");
            } else if (visibility == "visible") {
                uiResource.SetStyleProperty("pause-menu", "visibility", "hidden");
            }
        }
        escapeWasPressed = escapeIsPressed;
    }
}
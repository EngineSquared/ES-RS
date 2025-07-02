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
    }

    void _onDestroy(ES::Engine::Core &core) final
    {
        core.ClearEntities();
    }

private:
};
} // namespace Game

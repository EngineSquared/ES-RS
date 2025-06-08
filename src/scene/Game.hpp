#pragma once

#include "Scene.hpp"

using namespace ES::Plugin;

class Game : public ES::Plugin::Scene::Utils::AScene {

public:
    Game() : ES::Plugin::Scene::Utils::AScene() {}

protected:
    void _onCreate(ES::Engine::Core &core) final
    {
        _sceneEntities.push_back(CreateFloor(core));
        _sceneEntities.push_back(CreateVehicle(core));
    }

    void _onDestroy(ES::Engine::Core &core) final
    {
        for (auto entity : _sceneEntities) {
            if (entity.IsValid()) {
                entity.Destroy(core);
            }
        }
    }
private:
    std::vector<ES::Engine::Entity> _sceneEntities;
};
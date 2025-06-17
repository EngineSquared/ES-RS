#include "Engine.hpp"

// Engine headers
#include "Input.hpp"
#include "JoltPhysics.hpp"
#include "OpenGL.hpp"
#include "Camera.hpp" // TODO: remove when camera is in OpenGL
#include "Window.hpp"
#include "Scene.hpp"
#include "UI.hpp"

// Demo headers
#include "shader/LoadNoLightShader.hpp"
#include "LoadMaterials.hpp"
#include "CreateFloor.hpp"
#include "CreateVehicle.hpp"
#include "Game.hpp"
#include "SpeedOMeter.hpp"
#include "MainMenu.hpp"

using namespace ES::Plugin;

int main(void)
{
    ES::Engine::Core core;

	core.AddPlugins<Physics::Plugin, Input::Plugin, OpenGL::Plugin, Scene::Plugin, UI::Plugin>();

    core.RegisterSystem<ES::Engine::Scheduler::Startup>(
        LoadMaterials,
        LoadNoLightShader
    );

    core.RegisterSystem<ES::Engine::Scheduler::Startup>(
		[](ES::Engine::Core &c) {
			c.GetResource<Window::Resource::Window>().SetTitle("ES VehicleDemo");
			c.GetResource<Window::Resource::Window>().SetSize(1280, 720);
		},
		[](ES::Engine::Core &c) {
			c.GetResource<OpenGL::Resource::Camera>().viewer.centerAt(glm::vec3(0.0f, 0.0f, 0.0f));
			c.GetResource<OpenGL::Resource::Camera>().viewer.lookFrom(glm::vec3(0.0f, 10.0f, -20.0f));
            c.GetResource<Physics::Resource::PhysicsManager>().GetPhysicsSystem().OptimizeBroadPhase();
            c.GetScheduler<ES::Engine::Scheduler::FixedTimeUpdate>().SetTickRate(1.0f / 240.0f);
            printf("Available controllers:\n");
            ES::Plugin::Input::Utils::PrintAvailableControllers();
		},
        [](ES::Engine::Core &c) {
            c.GetResource<ES::Plugin::Scene::Resource::SceneManager>().RegisterScene<Game::MainMenu>("main-menu");
            c.GetResource<ES::Plugin::Scene::Resource::SceneManager>().RegisterScene<Game::Race>("race");
            c.GetResource<ES::Plugin::Scene::Resource::SceneManager>().SetNextScene("main-menu");
        },
        [](ES::Engine::Core &c) {
            c.GetResource<OpenGL::Resource::DirectionalLight>().posOfLight = glm::vec3(3.0f, 20.0f, 0.0f);
            c.GetResource<OpenGL::Resource::DirectionalLight>().lightProjection = glm::ortho(-50.0f, 50.0f, 50.0f, -50.0f, 1.0f, 50.0f);
            c.GetResource<OpenGL::Resource::DirectionalLight>().lightView =
                glm::lookAt(c.GetResource<OpenGL::Resource::DirectionalLight>().posOfLight,
                            glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            c.GetResource<OpenGL::Resource::DirectionalLight>().lightSpaceMatrix = c.GetResource<OpenGL::Resource::DirectionalLight>().lightProjection * c.GetResource<OpenGL::Resource::DirectionalLight>().lightView;
        },
        [](ES::Engine::Core &c) {
            c.GetResource<UI::Resource::UIResource>().SetFont("asset/font/Tomorrow-Medium.ttf");
            c.GetResource<UI::Resource::UIResource>().InitDocument("asset/ui/main-menu/main-menu.rml");
            c.GetResource<UI::Resource::UIResource>().AttachEventHandlers("start-game-btn", "click", [&c](const std::string &event, const std::string &elementId) {
                if (elementId == "start-game-btn" && event == "click") {
                    c.GetResource<ES::Plugin::Scene::Resource::SceneManager>().SetNextScene("race");
                }
            });
        }
    );

    core.RunCore();

    return 0;
}

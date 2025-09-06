#include "Engine.hpp"

// Engine headers
#include "Input.hpp"
#include "JoltPhysics.hpp"
#include "OpenGL.hpp"
#include "Camera.hpp" // TODO: remove when camera is in OpenGL
#include "Window.hpp"
#include "Scene.hpp"
#include "UI.hpp"
#include "Sound.hpp"

// Demo headers
#include "shader/LoadNoLightShader.hpp"
#include "shader/LoadTextureShader.hpp"
#include "LoadMaterials.hpp"
#include "CreateRace.hpp"
#include "CreateVehicle.hpp"
#include "Game.hpp"
#include "SpeedOMeter.hpp"
#include "MainMenu.hpp"

using namespace ES::Plugin;

int main(void)
{
    ES::Engine::Core core;

	core.AddPlugins<Input::Plugin, OpenGL::Plugin, Scene::Plugin, UI::Plugin, Sound::Plugin>();

    /* Binding the PhysicPlugin here to keep track of the FixedUpdate systems */
    core.RegisterSystem<ES::Engine::Scheduler::Startup>(ES::Plugin::Physics::System::InitJoltPhysics);
    core.RegisterSystem<ES::Engine::Scheduler::Startup>(ES::Plugin::Physics::System::InitPhysicsManager);
    core.RegisterSystem<ES::Engine::Scheduler::Startup>(
        ES::Plugin::Physics::System::OnConstructLinkRigidBodiesToPhysicsSystem);
    core.RegisterSystem<ES::Engine::Scheduler::Startup>(
        ES::Plugin::Physics::System::OnConstructLinkSoftBodiesToPhysicsSystem);
    core.RegisterSystem<ES::Engine::Scheduler::Startup>(
        ES::Plugin::Physics::System::OnConstructLinkWheeledVehiclesToPhysicsSystem);
    core.RegisterSystem<ES::Engine::Scheduler::Shutdown>(ES::Plugin::Physics::System::ShutdownJoltPhysics);

    core.RegisterSystem<ES::Engine::Scheduler::Startup>(
        LoadMaterials,
        LoadNoLightShader,
        LoadTextureShader
    );

    core.RegisterSystem<ES::Engine::Scheduler::Startup>(
		[](ES::Engine::Core &c) {
			c.GetResource<Window::Resource::Window>().SetTitle("ES VehicleDemo");
			c.GetResource<Window::Resource::Window>().SetSize(1280, 720);
		},
		[](ES::Engine::Core &c) {
			c.GetResource<OpenGL::Resource::Camera>().viewer.centerAt(glm::vec3(0.0f, 0.0f, 0.0f));
			c.GetResource<OpenGL::Resource::Camera>().viewer.lookFrom(glm::vec3(0.0f, 5.0f, -10.0f));
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
            c.GetResource<ES::Plugin::Sound::Resource::SoundManager>().RegisterSound("start-menu", "asset/sounds/start-menu.mp3");
            c.GetResource<ES::Plugin::Sound::Resource::SoundManager>().RegisterSound("button_hover", "asset/sounds/btn-hover.mp3");
            c.GetResource<ES::Plugin::Sound::Resource::SoundManager>().RegisterSound("button_click", "asset/sounds/btn-click.mp3");
            c.GetResource<ES::Plugin::Sound::Resource::SoundManager>().RegisterSound("main-menu", "asset/sounds/main-menu.mp3", true);
            c.GetResource<ES::Plugin::Sound::Resource::SoundManager>().RegisterSound("race-ambient", "asset/sounds/race-amb.mp3", true);
            c.GetResource<ES::Plugin::Sound::Resource::SoundManager>().RegisterSound("race-ambient-life", "asset/sounds/race-amb-life.mp3", true);
            c.GetResource<ES::Plugin::Sound::Resource::SoundManager>().RegisterSound("pause-menu", "asset/sounds/pause-menu.mp3");
        
            c.GetResource<ES::Plugin::Sound::Resource::SoundManager>().SetVolume("start-menu", 0.2f);
            c.GetResource<ES::Plugin::Sound::Resource::SoundManager>().SetVolume("button_hover", 0.6f);
            c.GetResource<ES::Plugin::Sound::Resource::SoundManager>().SetVolume("button_click", 0.6f);
            c.GetResource<ES::Plugin::Sound::Resource::SoundManager>().SetVolume("main-menu", 0.4f);
            c.GetResource<ES::Plugin::Sound::Resource::SoundManager>().SetVolume("race-ambient", 0.02f);
            c.GetResource<ES::Plugin::Sound::Resource::SoundManager>().SetVolume("race-ambient-life", 0.3f);
            c.GetResource<ES::Plugin::Sound::Resource::SoundManager>().SetVolume("pause-menu", 0.3f);
        }
    );

    core.RunCore();

    return 0;
}

#include "Engine.hpp"

// Engine headers
#include "Input.hpp"
#include "JoltPhysics.hpp"
#include "OpenGL.hpp"
#include "Camera.hpp" // TODO: remove when camera is in OpenGL
#include "Window.hpp"

// Demo headers
#include "shader/LoadNoLightShader.hpp"
#include "LoadMaterials.hpp"
#include "CreateFloor.hpp"
#include "CreateVehicle.hpp"

using namespace ES::Plugin;

int main(void)
{
    ES::Engine::Core core;

	core.AddPlugins<Physics::Plugin, Input::Plugin, OpenGL::Plugin>();

    core.RegisterSystem<ES::Engine::Scheduler::Startup>(
        LoadMaterials,
        LoadNoLightShader,
        [&](ES::Engine::Core &c){ CreateFloor(c); },
        CreateVehicle
    );

    core.RegisterSystem<ES::Engine::Scheduler::FixedTimeUpdate>(
        // VehicleMovement
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
		}
    );

    core.RunCore();

    return 0;
}

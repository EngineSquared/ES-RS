/**************************************************************************
 * EngineSquared - Vehicle Usage Example
 *
 * This example demonstrates how to use the Graphic and Physics plugins together for vehicle simulation.
 **************************************************************************/

#include "plugin/PhysicsPlugin.hpp"
#include "plugin/PluginCameraMovement.hpp"
#include "plugin/PluginDefaultPipeline.hpp"
#include "plugin/PluginInput.hpp"
#include "plugin/PluginRmlui.hpp"
#include "plugin/PluginScene.hpp"
#include "plugin/PluginWindow.hpp"
#include "plugin/PluginSound.hpp"

#include "resource/SceneManager.hpp"
#include "resource/Window.hpp"

#include "scenes/CourseScene.hpp"
#include "scenes/MainMenu.hpp"

void EscapeKeySystem(Engine::Core &core)
{
    auto &inputManager = core.GetResource<Input::Resource::InputManager>();

    if (inputManager.IsKeyPressed(GLFW_KEY_ESCAPE))
    {
        core.Stop();
    }
}

void Setup(Engine::Core &core)
{
    auto &window = core.GetResource<Window::Resource::Window>();
    window.SetSize(1280, 720);

    core.RegisterSystem(EscapeKeySystem);

    auto camera = core.CreateEntity();

    camera.AddComponent<Object::Component::Transform>(glm::vec3(0.0f, 1.0f, -10.0f));
    camera.AddComponent<Object::Component::Camera>();

    auto &cameraManager = core.GetResource<CameraMovement::Resource::CameraManager>();
    cameraManager.SetActiveCamera(camera);
    cameraManager.SetMovementSpeed(3.0f);

    core.GetResource<Scene::Resource::SceneManager>().RegisterScene<Game::MainMenu>("MenuScene");
    core.GetResource<Scene::Resource::SceneManager>().RegisterScene<Game::CourseScene>("CourseScene");
    core.GetResource<Scene::Resource::SceneManager>().SetNextScene("MenuScene");
}

class GraphicExampleError : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

int main(void)
{
    spdlog::set_level(spdlog::level::info);
    Engine::Core core;

    core.AddPlugins<Window::Plugin, Scene::Plugin, DefaultPipeline::Plugin, Input::Plugin, Rmlui::Plugin,
        CameraMovement::Plugin, Physics::Plugin, Sound::Plugin>();

    core.RegisterSystem<Engine::Scheduler::Startup>(Setup);

    core.RunCore();

    return 0;
}

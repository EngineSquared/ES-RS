/**************************************************************************
 * EngineSquared - Vehicle Usage Example
 *
 * This example demonstrates how to use the Graphic and Physics plugins together
 * for vehicle simulation.
 **************************************************************************/

#include "Engine.hpp"
#include <memory>

#include "CameraMovement.hpp"
#include "DefaultPipeline.hpp"
#include "Graphic.hpp"
#include "Input.hpp"
#include "Object.hpp"
#include "Physics.hpp"
#include "RenderingPipeline.hpp"
#include "Sound.hpp"
#include "plugin/PluginRmlui.hpp"
#include "plugin/PluginScene.hpp"
#include "plugin/PluginSound.hpp"
#include "plugin/PluginWindow.hpp"

#include "resource/InputManager.hpp"
#include "resource/CameraControlSystemManager.hpp"
#include "resource/SceneManager.hpp"
#include "resource/Window.hpp"

#include "component/PlayerVehicle.hpp"
#include "resource/CameraControlSystemManager.hpp"
#include "scenes/CourseScene.hpp"
#include "scenes/CreateLight.hpp"
#include "scenes/CreateVehicle.hpp"
#include "scenes/LoadCourse.hpp"
#include "scenes/MainMenu.hpp"
#include "system/ChildFollowParentSystem.hpp"
#include "system/EngineAudioSystem.hpp"
#include "resource/VehicleTelemetry.hpp"
#include "resource/PhysicsManager.hpp"
#include "component/VehicleInternal.hpp"
#include <Jolt/Physics/Vehicle/WheeledVehicleController.h>
#include "system/VehicleInput.hpp"
#include "utils/OrbitalChaseCameraBehavior.hpp"
#include "utils/ChaseCameraBehavior.hpp"
#include "utils/FirstPersonCameraBehavior.hpp"
#include "utils/FirstPersonOrbitalCameraBehavior.hpp"
#include "utils/cameraBehavior/DontMoveBehavior.hpp"

void EscapeKeySystem(Engine::Core &core) {
  auto &inputManager = core.GetResource<Input::Resource::InputManager>();

  if (inputManager.IsKeyPressed(GLFW_KEY_ESCAPE)) {
    auto &cameraManager =
        core.GetResource<CameraMovement::Resource::CameraManager>();
    cameraManager.SetBehavior(std::make_shared<CameraMovement::Utils::DontMoveBehavior>());
    core.Stop();
  }
}

void Setup(Engine::Core &core) {
  auto &window = core.GetResource<Window::Resource::Window>();
  window.SetSize(1280, 720);

  auto camera = core.CreateEntity();

  camera.AddComponent<Object::Component::Transform>(
      glm::vec3(0.0f, 1.0f, -10.0f));
  camera.AddComponent<Object::Component::Camera>();
  camera.GetComponents<Object::Component::Camera>().farPlane = 10000.0f;

  auto &cameraManager =
      core.GetResource<CameraMovement::Resource::CameraManager>();
  cameraManager.SetActiveCamera(camera);
  cameraManager.SetMovementSpeed(3.0f);

  core.GetResource<Scene::Resource::SceneManager>()
      .RegisterScene<Game::MainMenu>("MenuScene");
  core.GetResource<Scene::Resource::SceneManager>()
      .RegisterScene<Game::CourseScene>("CourseScene");
  core.GetResource<Scene::Resource::SceneManager>().SetNextScene("MenuScene");
}

int main(void) {
  spdlog::set_level(spdlog::level::info);
  Engine::Core core;

  core.AddPlugins<Window::Plugin, Scene::Plugin, DefaultPipeline::Plugin,
                  Input::Plugin, Rmlui::Plugin, CameraMovement::Plugin,
                  Physics::Plugin, Sound::Plugin>();

  core.RegisterSystem<Engine::Scheduler::Startup>(Setup);

  core.Run();

  return 0;
}

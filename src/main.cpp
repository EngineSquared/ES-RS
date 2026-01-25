/**************************************************************************
 * EngineSquared - Vehicle Usage Example
 *
 * This example demonstrates how to use the Graphic and Physics plugins together for vehicle simulation.
 **************************************************************************/

#include "Engine.hpp"

#include "CameraMovement.hpp"
#include "DefaultPipeline.hpp"
#include "Graphic.hpp"
#include "Input.hpp"
#include "Object.hpp"
#include "Physics.hpp"
#include "Sound.hpp"
#include "RenderingPipeline.hpp"
#include "plugin/PluginWindow.hpp"
#include "resource/Window.hpp"

#include "component/PlayerVehicle.hpp"
#include "resource/CameraControlSystemManager.hpp"
#include "scenes/LoadCourse.hpp"
#include "scenes/CreateVehicle.hpp"
#include "scenes/CreateLight.hpp"
#include "system/VehicleInput.hpp"
#include "system/ChildFollowParentSystem.hpp"
#include "utils/OrbitalChaseCameraBehavior.hpp"
#include "system/EngineAudioSystem.hpp"

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
    // Option to lock the cursor to the window
    auto &window = core.GetResource<Window::Resource::Window>();
    // window.MaskCursor();

    //CreateCheckeredFloor(core);
    LoadCourse(core);
    auto vehicle = CreateVehicle(core);
    auto light = CreateLight(core);

    auto camera = core.CreateEntity();

    camera.AddComponent<Object::Component::Transform>(glm::vec3(0.0f, 1.0f, -10.0f));
    camera.AddComponent<Object::Component::Camera>();
    camera.GetComponents<Object::Component::Camera>().farPlane = 10000.0f;

    auto &cameraManager = core.GetResource<CameraMovement::Resource::CameraManager>();
    cameraManager.SetActiveCamera(camera);
    cameraManager.SetMovementSpeed(3.0f);

    core.RegisterSystem(EscapeKeySystem);

    core.RegisterSystem<Engine::Scheduler::FixedTimeUpdate>(VehicleInput);
    core.RegisterSystem<Engine::Scheduler::FixedTimeUpdate>(ChildFollowParentSystem);

    auto chaseBehavior = std::make_shared<OrbitalChaseCameraBehavior>(core, vehicle);
    cameraManager.SetBehavior(chaseBehavior);

    auto &fixedTimeScheduler = core.GetScheduler<Engine::Scheduler::FixedTimeUpdate>();
    fixedTimeScheduler.SetTickRate(1.0f / 120.0f);

    auto &cameraControlSystemManager = core.GetResource<CameraMovement::Resource::CameraControlSystemManager>();
    cameraControlSystemManager.SetCameraControlSystemScheduler<Engine::Scheduler::FixedTimeUpdate>(core);

    // Register engine sound and attach audio component to vehicle entity
    auto &soundMgr = core.GetResource<Sound::Resource::SoundManager>();
    soundMgr.RegisterSound("engine_low", "asset/sounds/911_RSR30_1_in_on_high.wav", true);
    Log::Info("Engine sound registered: engine_low");

    vehicle.AddComponent<Game::Component::EngineAudioComponent>();
    Log::Info("EngineAudioComponent added to vehicle");

    // Drive audio updates on regular Update scheduler
    core.RegisterSystem<Engine::Scheduler::Update>(EngineAudioSystem);
}

class GraphicExampleError : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

int main(void)
{
    spdlog::set_level(spdlog::level::info);
    Engine::Core core;

    core.AddPlugins<Window::Plugin, DefaultPipeline::Plugin, Input::Plugin, CameraMovement::Plugin, Physics::Plugin, Sound::Plugin>();

    core.RegisterSystem<Engine::Scheduler::Startup>(Setup);

    core.RunCore();

    return 0;
}

/**************************************************************************
 * EngineSquared - Vehicle Usage Example
 *
 * This example demonstrates how to use the Graphic and Physics plugins together
 * for vehicle simulation.
 **************************************************************************/

#include "Engine.hpp"

#include "CameraMovement.hpp"
#include "DefaultPipeline.hpp"
#include "Graphic.hpp"
#include "Input.hpp"
#include "Object.hpp"
#include "Physics.hpp"
#include "RenderingPipeline.hpp"
#include "Sound.hpp"
#include "plugin/PluginWindow.hpp"
#include "resource/Window.hpp"

#include "component/PlayerVehicle.hpp"
#include "resource/CameraControlSystemManager.hpp"
#include "scenes/CreateLight.hpp"
#include "scenes/CreateVehicle.hpp"
#include "scenes/LoadCourse.hpp"
#include "system/ChildFollowParentSystem.hpp"
#include "system/EngineAudioSystem.hpp"
#include "resource/VehicleTelemetry.hpp"
#include "resource/PhysicsManager.hpp"
#include "component/VehicleInternal.hpp"
#include <Jolt/Physics/Vehicle/WheeledVehicleController.h>
#include "system/VehicleInput.hpp"
#include "utils/OrbitalChaseCameraBehavior.hpp"

void EscapeKeySystem(Engine::Core &core) {
  auto &inputManager = core.GetResource<Input::Resource::InputManager>();

  if (inputManager.IsKeyPressed(GLFW_KEY_ESCAPE)) {
    core.Stop();
  }
}

void VehicleDebugSystem(Engine::Core &core)
{
    auto &registry = core.GetRegistry();
    auto &telemetry = core.GetResource<Physics::Resource::VehicleTelemetry>();
    auto &physicsMgr = core.GetResource<Physics::Resource::PhysicsManager>();

    auto view = registry.view<Physics::Component::Vehicle, Physics::Component::VehicleInternal, Physics::Component::VehicleController>();
    for (auto [e, vehicle, internal, controllerComp] : view.each())
    {
        // Only debug player vehicle(s)
        if (!registry.all_of<PlayerVehicle>(e))
            continue;

        Engine::EntityId eid{ static_cast<Engine::EntityId::ValueType>(e) };
        auto rpmOpt = telemetry.GetRPM(eid);

        std::string rpmStr = "N/A";
        float rpmVal = 0.0f;
        if (rpmOpt.has_value()) {
            rpmVal = rpmOpt.value();
            rpmStr = fmt::format("{:.1f}", rpmVal);
        }

        std::string speedStr = "N/A";
        float speedVal = 0.0f;
        if (internal.IsValid())
        {
            const JPH::BodyLockRead lock(physicsMgr.GetPhysicsSystem().GetBodyLockInterface(), internal.chassisBodyID);
            if (lock.Succeeded())
            {
                const JPH::Body &body = lock.GetBody();
                // Convert speed from m/s to km/h for debug display
                JPH::Vec3 velocity = body.GetLinearVelocity();
                velocity.SetY(0.0f);
                JPH::Vec3 forward = body.GetRotation().RotateAxisZ();
                forward.SetY(0.0f);
                speedVal = velocity.Dot(forward) * 3.6f;
                speedStr = fmt::format("{:.2f} km/h", speedVal);
            }
        }

        std::string gearStr = "N/A";
        if (internal.IsValid() && internal.vehicleConstraint != nullptr)
        {
            auto *ctrl = internal.vehicleConstraint->GetController();
            if (ctrl != nullptr)
            {
                auto *wctrl = static_cast<JPH::WheeledVehicleController *>(ctrl);
                gearStr = fmt::format("{}", wctrl->GetTransmission().GetCurrentGear());
            }
        }

        Log::Debug(fmt::format("Vehicle Debug ({}): RPM: {}, Speed: {}, Gear: {}, Controller: Fwd {:.2f}, Steer {:.2f}, Brake {:.2f}, HandBrake {:.2f}",
                               Engine::Entity{core, e}, rpmStr, speedStr, gearStr,
                               controllerComp.forwardInput, controllerComp.steeringInput,
                               controllerComp.brakeInput, controllerComp.handBrakeInput));
    }
}

void Setup(Engine::Core &core)
{
    // Option to lock the cursor to the window
    auto &window = core.GetResource<Window::Resource::Window>();
    // window.MaskCursor();

  // CreateCheckeredFloor(core);
  LoadCourse(core);
  auto vehicle = CreateVehicle(core);
  auto light = CreateLight(core);

  auto camera = core.CreateEntity();

  camera.AddComponent<Object::Component::Transform>(
      glm::vec3(0.0f, 1.0f, -10.0f));
  camera.AddComponent<Object::Component::Camera>();
  camera.GetComponents<Object::Component::Camera>().farPlane = 10000.0f;

  auto &cameraManager =
      core.GetResource<CameraMovement::Resource::CameraManager>();
  cameraManager.SetActiveCamera(camera);
  cameraManager.SetMovementSpeed(3.0f);

  core.RegisterSystem(EscapeKeySystem);

    core.RegisterSystem<Engine::Scheduler::Update>(VehicleDebugSystem);
    core.RegisterSystem<Engine::Scheduler::FixedTimeUpdate>(VehicleInput);
    core.RegisterSystem<Engine::Scheduler::FixedTimeUpdate>(ChildFollowParentSystem);

  auto chaseBehavior =
      std::make_shared<OrbitalChaseCameraBehavior>(core, vehicle);
  cameraManager.SetBehavior(chaseBehavior);

  auto &fixedTimeScheduler =
      core.GetScheduler<Engine::Scheduler::FixedTimeUpdate>();
  fixedTimeScheduler.SetTickRate(1.0f / 120.0f);

  auto &cameraControlSystemManager =
      core.GetResource<CameraMovement::Resource::CameraControlSystemManager>();
  cameraControlSystemManager
      .SetCameraControlSystemScheduler<Engine::Scheduler::FixedTimeUpdate>(
          core);

  // Register engine sound and attach audio component to vehicle entity
  auto &soundMgr = core.GetResource<Sound::Resource::SoundManager>();
  soundMgr.RegisterSound("engine_low",
                         "asset/sounds/911_RSR30_1_in_on_high.wav", true);
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

int main(void) {
  spdlog::set_level(spdlog::level::info);
  Engine::Core core;

  core.AddPlugins<Window::Plugin, DefaultPipeline::Plugin, Input::Plugin,
                  CameraMovement::Plugin, Physics::Plugin, Sound::Plugin>();

  core.RegisterSystem<Engine::Scheduler::Startup>(Setup);

  core.RegisterSystem([](Engine::Core &core) {
    auto view1 =
        core.GetRegistry()
            .view<PlayerVehicle, Physics::Component::VehicleController>();
    if (!view1.front()) {
      throw GraphicExampleError(
          "No entity with PlayerVehicle and VehicleController found.");
    }
    Engine::Entity playerVehicle{core, *view1.begin()};
    const auto &vehicleTransform =
        playerVehicle.GetComponents<Object::Component::Transform>();

    auto view2 = core.GetRegistry().view<Object::Component::DirectionalLight>();
    if (!view2.front()) {
      throw GraphicExampleError(
          "No entity with PlayerVehicle and VehicleController found.");
    }
    Engine::Entity playerDirectionalLight{core, *view2.begin()};
    auto &directionalLightTransform =
        playerDirectionalLight.GetComponents<Object::Component::Transform>();

    directionalLightTransform.SetPosition(vehicleTransform.GetPosition() +
                                          glm::vec3(3.35, 7.12, -5.14) * 5.f);
  });

  core.RunCore();

  return 0;
}

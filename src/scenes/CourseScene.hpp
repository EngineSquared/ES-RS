#pragma once

#include "component/PlayerVehicle.hpp"
#include "component/Transform.hpp"
#include "resource/SceneManager.hpp"
#include "resource/SoundManager.hpp"
#include "resource/UIContext.hpp"

#include "Physics.hpp"
#include "resource/CameraControlSystemManager.hpp"
#include "resource/VehicleTelemetry.hpp"
#include "scenes/CreateLight.hpp"
#include "scenes/CreateVehicle.hpp"
#include "scenes/LoadCourse.hpp"
#include "system/ChildFollowParentSystem.hpp"
#include "system/EngineAudioSystem.hpp"
#include "system/VehicleInput.hpp"
#include "utils/AScene.hpp"
#include "utils/OrbitalChaseCameraBehavior.hpp"
#include "utils/Timer.hpp"
#include "CallableFunction.hpp"

#include <RmlUi/Core/StringUtilities.h>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>

namespace Game {
struct StartupCircuitTimer {
  Timer timer;
};

struct GameChrono {
  Timer timer;
};

class GraphicExampleError : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

void VehicleDebugSystem(Engine::Core &core) {
  auto &registry = core.GetRegistry();
  auto &telemetry = core.GetResource<Physics::Resource::VehicleTelemetry>();
  auto &physicsMgr = core.GetResource<Physics::Resource::PhysicsManager>();

  auto view = registry.view<Physics::Component::Vehicle,
                            Physics::Component::VehicleInternal,
                            Physics::Component::VehicleController>();
  for (auto [e, vehicle, internal, controllerComp] : view.each()) {
    // Only debug player vehicle(s)
    if (!registry.all_of<PlayerVehicle>(e))
      continue;

    Engine::EntityId eid{static_cast<Engine::EntityId::ValueType>(e)};
    auto rpmOpt = telemetry.GetRPM(eid);

    std::string rpmStr = "N/A";
    float rpmVal = 0.0f;
    if (rpmOpt.has_value()) {
      rpmVal = rpmOpt.value();
      rpmStr = fmt::format("{:.1f}", rpmVal);
    }

    std::string speedStr = "N/A";
    float speedVal = 0.0f;
    if (internal.IsValid()) {
      const JPH::BodyLockRead lock(
          physicsMgr.GetPhysicsSystem().GetBodyLockInterface(),
          internal.chassisBodyID);
      if (lock.Succeeded()) {
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
    if (internal.IsValid() && internal.vehicleConstraint != nullptr) {
      auto *ctrl = internal.vehicleConstraint->GetController();
      if (ctrl != nullptr) {
        auto *wctrl = static_cast<JPH::WheeledVehicleController *>(ctrl);
        gearStr = fmt::format("{}", wctrl->GetTransmission().GetCurrentGear());
      }
    }

    Log::Debug(fmt::format(
        "Vehicle Debug ({}): RPM: {}, Speed: {}, Gear: {}, Controller: Fwd "
        "{:.2f}, Steer {:.2f}, Brake {:.2f}, HandBrake {:.2f}",
        Engine::Entity{core, e}, rpmStr, speedStr, gearStr,
        controllerComp.forwardInput, controllerComp.steeringInput,
        controllerComp.brakeInput, controllerComp.handBrakeInput));
  }
}

class CourseScene : public Scene::Utils::AScene {
public:
  CourseScene()
      : _gameChrono{GameChrono(Timer(1.0f).SetInfinite(true))},
        _startupCircuitChrono{StartupCircuitTimer(Timer(1.f).SetIterations(3))},
        _IsCountingDown(true) {};

protected:
  void _onCreate(Engine::Core &core) final {
    Log::Info("Creating CourseScene");

    auto vehicle = CreateVehicle(core);
    auto light = CreateLight(core);
    LoadCourse(core);

    auto &cameraManager =
        core.GetResource<CameraMovement::Resource::CameraManager>();
    auto camera = cameraManager.GetActiveCamera();

    camera.GetComponents<Object::Component::Camera>().farPlane = 10000.0f;
    camera.GetComponents<Object::Component::Transform>().SetPosition(
        glm::vec3(0.0f, 1.0f, -10.0f));

    cameraManager.SetMovementSpeed(3.0f);

    core.RegisterSystem<Engine::Scheduler::Update>(VehicleDebugSystem);
    core.RegisterSystem<Engine::Scheduler::FixedTimeUpdate>(VehicleInput);
    core.RegisterSystem<Engine::Scheduler::FixedTimeUpdate>(
        ChildFollowParentSystem);

    auto chaseBehavior =
        std::make_shared<OrbitalChaseCameraBehavior>(core, vehicle);
    cameraManager.SetBehavior(chaseBehavior);

    auto &fixedTimeScheduler =
        core.GetScheduler<Engine::Scheduler::FixedTimeUpdate>();
    fixedTimeScheduler.SetTickRate(1.0f / 120.0f);

    auto &cameraControlSystemManager = core.GetResource<
        CameraMovement::Resource::CameraControlSystemManager>();
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

    core.RegisterSystem([](Engine::Core &core) {
      auto view1 =
          core.GetRegistry()
              .view<PlayerVehicle, Physics::Component::VehicleController>();
      if (view1.begin() == view1.end()) {
        throw GraphicExampleError(
            "No entity with PlayerVehicle and VehicleController found.");
      }
      Engine::Entity playerVehicle{core, *view1.begin()};
      const auto &vehicleTransform =
          playerVehicle.GetComponents<Object::Component::Transform>();

      auto view2 =
          core.GetRegistry().view<Object::Component::DirectionalLight>();
      if (view2.begin() == view2.end()) {
        throw GraphicExampleError("No entity with DirectionalLight found.");
      }
      Engine::Entity playerDirectionalLight{core, *view2.begin()};
      auto &directionalLightTransform =
          playerDirectionalLight.GetComponents<Object::Component::Transform>();

      directionalLightTransform.SetPosition(vehicleTransform.GetPosition() +
                                            glm::vec3(3.35, 7.12, -5.14) * 5.f);
    });

    // Drive audio updates on regular Update scheduler
    core.RegisterSystem<Engine::Scheduler::Update>(EngineAudioSystem);
    core.RegisterSystem<Engine::Scheduler::Update>([this](Engine::Core &core) { this->TogglePauseMenu(core); });
    core.RegisterSystem<Engine::Scheduler::Update>([this](Engine::Core &core) { this->ToggleUiDebugContrast(core); });

    auto &uiContext = core.GetResource<Rmlui::Resource::UIContext>();
    uiContext.SetFont("asset/font/Tomorrow-Medium.ttf");
    uiContext.SetFont("asset/font/airborne.ttf");
    //uiContext.EnableDebugger(true);
    uiContext.LoadDocument("asset/ui/race/game.rml");
    uiContext.LoadOverlayDocument("asset/ui/race/pause-menu.rml");
    auto *pauseMenu = uiContext.GetElementById("pause-menu");
    auto *resumeBtn = uiContext.GetElementById("resume-btn");
    auto *quitBtn = uiContext.GetElementById("quit-btn");
    if (pauseMenu != nullptr && resumeBtn != nullptr) {
      uiContext.RegisterEventListener(
          *resumeBtn, "click", [pauseMenu, this, &core](Rml::Event &event) {
            pauseMenu->SetProperty("visibility", "hidden");
            pauseMenu->SetProperty("animation", "none");
            _hasLastVehiclePosition = false;
            _smoothedSpeedKmh = 0.0f;
            SetFixedUpdateSystemsEnabled(core, true);
          });
    }
    if (quitBtn != nullptr) {
      uiContext.RegisterEventListener(
          *quitBtn, "click", [&core](Rml::Event &event) {
            core.Stop();
          });
    }
    AddChronoDisplay(core);
    AddSpeedometerDisplay(core);
    AddSpeedometerAnimation(core);
  }

  void _onDestroy(Engine::Core &core) final {}

private:
  GameChrono _gameChrono;
  StartupCircuitTimer _startupCircuitChrono;
  bool _IsCountingDown;
  mutable std::size_t _chronoSystemId = std::numeric_limits<std::size_t>::max();
  mutable std::size_t _speedometerSystemId =
      std::numeric_limits<std::size_t>::max();
  mutable std::size_t _speedometerAnimSystemId =
      std::numeric_limits<std::size_t>::max();
  glm::vec3 _lastVehiclePosition = glm::vec3(0.0f);
  bool _hasLastVehiclePosition = false;
  float _smoothedSpeedKmh = 0.0f;
  bool _debugUiContrastEnabled = false;

  template <typename TCallable>
  static FunctionUtils::FunctionID GetSystemId(TCallable callable) {
    return FunctionUtils::CallableFunction<TCallable, void, Engine::Core &>::
        GetCallableID(callable);
  }

  void SetFixedUpdateSystemsEnabled(Engine::Core &core, bool enable) {
    auto &fixedScheduler = core.GetScheduler<Engine::Scheduler::FixedTimeUpdate>();
    if (enable) {
      fixedScheduler.Enable(GetSystemId(VehicleInput));
      fixedScheduler.Enable(GetSystemId(ChildFollowParentSystem));
      fixedScheduler.Enable(GetSystemId(Physics::System::PhysicsUpdate));
      fixedScheduler.Enable(GetSystemId(Physics::System::VehicleControlSystem));
      fixedScheduler.Enable(GetSystemId(Physics::System::VehicleRPMUpdate));
      fixedScheduler.Enable(GetSystemId(Physics::System::SyncTransformWithPhysics));
      fixedScheduler.Enable(GetSystemId(Physics::System::SyncSoftBodyVertices));
      fixedScheduler.Enable(GetSystemId(Physics::System::WheelTransformSyncSystem));
    } else {
      fixedScheduler.Disable(GetSystemId(VehicleInput));
      fixedScheduler.Disable(GetSystemId(ChildFollowParentSystem));
      fixedScheduler.Disable(GetSystemId(Physics::System::PhysicsUpdate));
      fixedScheduler.Disable(GetSystemId(Physics::System::VehicleControlSystem));
      fixedScheduler.Disable(GetSystemId(Physics::System::VehicleRPMUpdate));
      fixedScheduler.Disable(GetSystemId(Physics::System::SyncTransformWithPhysics));
      fixedScheduler.Disable(GetSystemId(Physics::System::SyncSoftBodyVertices));
      fixedScheduler.Disable(GetSystemId(Physics::System::WheelTransformSyncSystem));
    }
  }

  void TogglePauseMenu(Engine::Core &core) {
    auto &uiResource = core.GetResource<Rmlui::Resource::UIContext>();
    auto &soundManager = core.GetResource<Sound::Resource::SoundManager>();
    static bool escapeWasPressed = false;

    if (uiResource.GetTitle() == "game") {
      const auto &visibility =
          uiResource.GetElementById("pause-menu")->GetProperty("visibility");
      bool escapeIsPressed =
          core.GetResource<Input::Resource::InputManager>().IsKeyPressed(
              GLFW_KEY_ESCAPE);

      if (escapeIsPressed && !escapeWasPressed) {
        if (visibility->Get<Rml::String>() == "1") {
          SetFixedUpdateSystemsEnabled(core, false);
          uiResource.GetElementById("pause-menu")
              ->SetProperty("animation", "0.2s quadratic-out 1 slide-in");
          uiResource.GetElementById("pause-menu")
              ->SetProperty("visibility", "visible");
          uiResource.RequestLateUpdate();
          if (soundManager.IsPlaying("pause-menu"))
            soundManager.Stop("pause-menu");
          soundManager.Play("pause-menu");
        } else if (visibility->Get<Rml::String>() == "0") {
          SetFixedUpdateSystemsEnabled(core, true);
          uiResource.GetElementById("pause-menu")
              ->SetProperty("visibility", "hidden");
          uiResource.RequestLateUpdate();
        }
      }
      escapeWasPressed = escapeIsPressed;
    }
  }

  void ToggleUiDebugContrast(Engine::Core &core) {
    auto &input = core.GetResource<Input::Resource::InputManager>();
    static bool f9WasPressed = false;
    const bool f9IsPressed = input.IsKeyPressed(GLFW_KEY_F9);

    if (f9IsPressed && !f9WasPressed) {
      _debugUiContrastEnabled = !_debugUiContrastEnabled;
      ApplyUiDebugContrast(core, _debugUiContrastEnabled);
      Log::Info(fmt::format("UI debug contrast: {}", _debugUiContrastEnabled ? "on" : "off"));
    }
    f9WasPressed = f9IsPressed;
  }

  void ApplyUiDebugContrast(Engine::Core &core, bool enable) {
    auto &ui = core.GetResource<Rmlui::Resource::UIContext>();
    auto apply = [enable](Rml::Element *element, const char *baseColor) {
      if (!element) {
        return;
      }
      if (enable) {
        element->SetProperty("color", "rgb(255, 255, 255)");
        element->SetProperty("background-color", "rgba(0, 0, 0, 0.9)");
        element->SetProperty("padding", "4px 6px");
      } else {
        element->SetProperty("color", baseColor);
        element->SetProperty("background-color", "transparent");
        element->SetProperty("padding", "0px");
      }
    };

    apply(ui.GetElementById("time-value"), "rgb(214, 214, 214)");
    apply(ui.GetElementById("value"), "inherit");
    apply(ui.GetElementById("gear-current"), "inherit");
    ui.RequestLateUpdate();
  }

  void StartupCircuitTimerUpdate(Engine::Core &core) {
    auto dt = core.GetScheduler<Engine::Scheduler::Update>().GetDeltaTime();

    _startupCircuitChrono.timer.Update(dt);
    if (_startupCircuitChrono.timer.JustCompleted()) {
      Log::Info(fmt::format("Circuit timer just completed after {} seconds",
                            _startupCircuitChrono.timer.elapsed));
    }
    if (_startupCircuitChrono.timer.Completed() && _IsCountingDown) {
      Log::Info(fmt::format("Circuit timer completed after {} seconds",
                            _startupCircuitChrono.timer.elapsed));
      core.RegisterSystem<Engine::Scheduler::Update>(
          [this](Engine::Core &core) { this->UpdateTextTime(core); });
      _IsCountingDown = false;
    }
  }

  void AddChronoDisplay(Engine::Core &core) {
    if (_chronoSystemId == std::numeric_limits<std::size_t>::max())
      _chronoSystemId =
          std::get<0>(core.RegisterSystem<Engine::Scheduler::Update>(
              [this](Engine::Core &core) {
                auto &uiResource =
                    core.GetResource<Rmlui::Resource::UIContext>();
                if (uiResource.GetTitle() == "game") {
                  if (IsPauseMenuHidden(uiResource))
                    this->StartupCircuitTimerUpdate(core);
                }
              }));
  }

  void UpdateTextTime(Engine::Core &core) {
    auto &uiResource = core.GetResource<Rmlui::Resource::UIContext>();

    if (uiResource.GetTitle() == "game") {
      if (IsPauseMenuHidden(uiResource)) {
        auto dt = core.GetScheduler<Engine::Scheduler::Update>().GetDeltaTime();
        _gameChrono.timer.Update(dt);

        double elapsed = _gameChrono.timer.elapsed;
        int minutes = static_cast<int>(elapsed / 60.0);
        int seconds = static_cast<int>(elapsed) % 60;
        int milliseconds =
            static_cast<int>((elapsed - static_cast<int>(elapsed)) * 1000);

        std::ostringstream timeStream;
        timeStream << std::setfill('0') << std::setw(2) << minutes << ":"
                   << std::setw(2) << seconds << ":" << std::setw(3)
                   << milliseconds;

        if (auto *timeValue = uiResource.GetElementById("time-value")) {
          timeValue->SetInnerRML(
              Rml::StringUtilities::EncodeRml(timeStream.str()));
          uiResource.RequestLateUpdate();
        }
      }
    }
  }

  void AddSpeedometerDisplay(Engine::Core &core) {
    if (_speedometerSystemId == std::numeric_limits<std::size_t>::max())
      _speedometerSystemId =
          std::get<0>(core.RegisterSystem<Engine::Scheduler::Update>(
              [this](Engine::Core &core) { this->UpdateSpeedometer(core); }));
  }

  void AddSpeedometerAnimation(Engine::Core &core) {
    if (_speedometerAnimSystemId == std::numeric_limits<std::size_t>::max())
      _speedometerAnimSystemId =
          std::get<0>(core.RegisterSystem<Engine::Scheduler::Update>(
              [this](Engine::Core &core) {
                this->UpdateSpeedometerRPM(core);
              }));
  }

  void UpdateSpeedometer(Engine::Core &core) {
    auto &uiResource = core.GetResource<Rmlui::Resource::UIContext>();
    if (uiResource.GetTitle() != "game" || !IsPauseMenuHidden(uiResource))
      return;

    auto speedValue = uiResource.GetElementById("value");
    auto gearValue = uiResource.GetElementById("gear-current");
    auto speedPointer = uiResource.GetElementById("speed-counter-pointer");
 
    if (!speedValue || !speedPointer) {
      Log::Error("speed value or speed pointer not found");
      return;
    }

    auto &registry = core.GetRegistry();
    auto view = registry.view<PlayerVehicle, Physics::Component::Vehicle,
                              Object::Component::Transform>();
    if (view.begin() == view.end()) {
      Log::Error("No player vehicle found");
      return;
    }

    for (auto entity : view) {
      auto &vehicle = view.get<Physics::Component::Vehicle>(entity);
      auto internal = registry.get<Physics::Component::VehicleInternal>(entity);
      auto controller = reinterpret_cast<JPH::WheeledVehicleController *>(internal.vehicleConstraint->GetController());
      auto &transform = view.get<Object::Component::Transform>(entity);

      const float dt =
          core.GetScheduler<Engine::Scheduler::Update>().GetDeltaTime();
      const auto currentPosition = transform.GetPosition();

      float instantSpeedKmh = 0.0f;
      if (_hasLastVehiclePosition && dt > 0.0f) {
        const float speedMps =
            glm::length(currentPosition - _lastVehiclePosition) / dt;
        instantSpeedKmh = speedMps * 3.6f;
      }
      _lastVehiclePosition = currentPosition;
      _hasLastVehiclePosition = true;

      constexpr float kSpeedSmoothingTau = 0.15f;
      const float smoothingAlpha =
          std::clamp(1.0f - std::exp(-dt / kSpeedSmoothingTau), 0.0f, 1.0f);
      _smoothedSpeedKmh =
          std::lerp(_smoothedSpeedKmh, instantSpeedKmh, smoothingAlpha);

      const int speedRounded = static_cast<int>(std::round(_smoothedSpeedKmh));
      speedValue->SetInnerRML(std::to_string(speedRounded));
      uiResource.RequestLateUpdate();
      if (gearValue != nullptr) {
        if (controller->GetTransmission().GetCurrentGear() < 0)
          gearValue->SetInnerRML("R");
        else if (controller->GetTransmission().GetCurrentGear() == 0)
          gearValue->SetInnerRML("N");
        else
          gearValue->SetInnerRML(std::to_string(controller->GetTransmission().GetCurrentGear()));
        uiResource.RequestLateUpdate();
      }

      constexpr float kMinNeedleAngle = 0.0f;
      constexpr float kMaxNeedleAngle = 230.0f;
      const float rpm = controller->GetEngine().GetCurrentRPM();
      const float maxRpm = vehicle.engine.maxRPM;
      const float t =
          (maxRpm > 0.0f) ? std::clamp(rpm / maxRpm, 0.0f, 1.0f) : 0.0f;
      const float angle = std::lerp(kMinNeedleAngle, kMaxNeedleAngle, t);
      speedPointer->SetProperty("transform",
                                fmt::format("rotate({:.1f}deg)", angle));
      uiResource.RequestLateUpdate();
      break;
    }
  }

  void UpdateSpeedometerRPM(Engine::Core &core) noexcept {
    auto &uiResource = core.GetResource<Rmlui::Resource::UIContext>();
    if (uiResource.GetTitle() != "game" || !IsPauseMenuHidden(uiResource)) {
      return;
    }

    auto &registry = core.GetRegistry();
    auto view = registry.view<PlayerVehicle>();
    if (view.begin() == view.end())
    {
      return;
    }

    auto &telemetry = core.GetResource<Physics::Resource::VehicleTelemetry>();
    constexpr int kLedCount = 41;

    for (auto entity : view) {
      if (!registry.all_of<Physics::Component::Vehicle>(entity))
        continue;
      auto &vehicle = registry.get<Physics::Component::Vehicle>(entity);
      auto internal = registry.get<Physics::Component::VehicleInternal>(entity);
      auto controller = reinterpret_cast<JPH::WheeledVehicleController *>(internal.vehicleConstraint->GetController());

      Engine::EntityId eid{static_cast<Engine::EntityId::ValueType>(entity)};
      float rpm = controller->GetEngine().GetCurrentRPM();
      float maxRpm = vehicle.engine.maxRPM;
      float ratio = (maxRpm > 0.0f) ? rpm / maxRpm : 0.0f;
      ratio = std::clamp(ratio, 0.0f, 1.0f);
      int activeLeds =
          static_cast<int>(std::round(ratio * static_cast<float>(kLedCount)));
      activeLeds = std::clamp(activeLeds, 0, kLedCount);

      for (int i = 1; i <= kLedCount; ++i) {
        auto *led = uiResource.GetElementById(fmt::format("sc-{:02d}", i));
        if (led == nullptr)
          continue;
        led->SetProperty("opacity", (i <= activeLeds) ? "1" : "0");
      }
      uiResource.RequestLateUpdate();
      break;
    }
  }

  bool IsPauseMenuHidden(Rmlui::Resource::UIContext &uiResource) const {
    auto pauseMenuDocument =
        uiResource.GetOverlayDocument("asset/ui/race/pause-menu.rml");
    if (pauseMenuDocument == nullptr)
      return true;
    auto *pauseMenu = pauseMenuDocument->GetElementById("pause-menu");
    if (pauseMenu == nullptr)
      return true;

    if (auto *visibilityProp = pauseMenu->GetProperty("visibility")) {
      auto visibility = visibilityProp->Get<Rml::String>();
      return visibility == "1";
    }
    return true;
  }
};
} // namespace Game
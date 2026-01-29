#pragma once

#include "Logger.hpp"
#include "component/Camera.hpp"
#include "component/PlayerVehicle.hpp"
#include "component/Transform.hpp"
#include "component/Vehicle.hpp"
#include "resource/CameraControlSystemManager.hpp"
#include "resource/CameraManager.hpp"
#include "resource/UIContext.hpp"
#include "resource/VehicleTelemetry.hpp"
#include "scenes/VehicleScene.hpp"
#include "system/ChildFollowParentSystem.hpp"
#include "system/VehicleInput.hpp"
#include "utils/AScene.hpp"
#include "utils/OrbitalChaseCameraBehavior.hpp"
#include "utils/Timer.hpp"

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
class CourseScene : public Scene::Utils::AScene {
public:
  CourseScene();
      /* : _gameChrono{GameChrono(Timer(1.0f).SetInfinite(true))},
        _startupCircuitChrono{StartupCircuitTimer(Timer(1.f).SetIterations(3))},
        _IsCountingDown(true) {}; */

protected:
  void _onCreate(Engine::Core &core) final {
    Log::Info("Creating CourseScene");

    CreateCheckeredFloor(core);
    auto vehicle = CreateVehicle(core);
    auto light = CreateLight(core);

    auto &cameraManager =
        core.GetResource<CameraMovement::Resource::CameraManager>();
    auto camera = cameraManager.GetActiveCamera();

    camera.GetComponents<Object::Component::Camera>().farPlane = 10000.0f;
    camera.GetComponents<Object::Component::Transform>().SetPosition(
        glm::vec3(0.0f, 1.0f, -10.0f));

    cameraManager.SetMovementSpeed(3.0f);

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

    auto &uiContext = core.GetResource<Rmlui::Resource::UIContext>();
    uiContext.LoadDocument("asset/ui/race/game.rml");
    /* auto *pauseMenu = uiContext.GetElementById("pause-menu");
    auto *resumeBtn = uiContext.GetElementById("resume-btn");
    auto *quitBtn = uiContext.GetElementById("quit-btn");
    if (pauseMenu != nullptr && resumeBtn != nullptr) {
      uiContext.RegisterEventListener(
          *resumeBtn, "click", [pauseMenu, this](Rml::Event &event) {
            pauseMenu->SetProperty("visibility", "hidden");
            pauseMenu->SetProperty("animation", "none");
            _hasLastVehiclePosition = false;
          });
    }
    if (quitBtn != nullptr) {
      uiContext.RegisterEventListener(
          *quitBtn, "click",
          [&core](Rml::Event & event) { core.Stop(); });
    } */
    /* AddChronoDisplay(core);
    AddSpeedometerDisplay(core);
    AddSpeedometerAnimation(core); */
  }

  void _onDestroy(Engine::Core &core) final {}

private:
  /* GameChrono _gameChrono;
  StartupCircuitTimer _startupCircuitChrono;
  bool _IsCountingDown;
  mutable std::size_t _chronoSystemId = std::numeric_limits<std::size_t>::max();
  mutable std::size_t _speedometerSystemId = std::numeric_limits<std::size_t>::max();
  mutable std::size_t _speedometerAnimSystemId = std::numeric_limits<std::size_t>::max();
  glm::vec3 _lastVehiclePosition = glm::vec3(0.0f);
  bool _hasLastVehiclePosition = false;

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

        if (auto *timeValue = uiResource.GetElementById("time-value"))
          timeValue->SetInnerRML(timeStream.str());
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
              [this](Engine::Core &core) { this->UpdateSpeedometerRPM(core); }));
  }

  void UpdateSpeedometer(Engine::Core &core) {
    auto &uiResource = core.GetResource<Rmlui::Resource::UIContext>();
    if (uiResource.GetTitle() != "game" || !IsPauseMenuHidden(uiResource))
      return;

    auto *speedValue = uiResource.GetElementById("value");
    auto *gearValue = uiResource.GetElementById("gear-current");
    auto *speedPointer = uiResource.GetElementById("speed-counter-pointer");
    if (!speedValue || !speedPointer)
      return;

    auto &registry = core.GetRegistry();
    auto view = registry.view<PlayerVehicle>();
    if (view.begin() == view.end())
      return;

    for (auto entity : view) {
      if (!registry.all_of<Physics::Component::Vehicle,
                           Object::Component::Transform>(entity))
        continue;
      auto &vehicle = registry.get<Physics::Component::Vehicle>(entity);
      auto &transform = registry.get<Object::Component::Transform>(entity);

      auto currentPosition = transform.GetPosition();
      float speedKmh = 0.0f;
      float dt = core.GetScheduler<Engine::Scheduler::Update>().GetDeltaTime();
      if (_hasLastVehiclePosition && dt > 0.0f) {
        float speedMps =
            glm::length(currentPosition - _lastVehiclePosition) / dt;
        speedKmh = speedMps * 3.6f;
      }
      _lastVehiclePosition = currentPosition;
      _hasLastVehiclePosition = true;
      int speedRounded = static_cast<int>(std::round(speedKmh));
      speedValue->SetInnerRML(std::to_string(speedRounded));
      if (gearValue != nullptr) {
        if (vehicle.gearbox.currentGear <= 0)
          gearValue->SetInnerRML("R");
        else
          gearValue->SetInnerRML(std::to_string(vehicle.gearbox.currentGear));
      }

      constexpr float kMinNeedleAngle = -130.0f;
      constexpr float kMaxNeedleAngle = 130.0f;
      constexpr float kMaxSpeedKmh = 260.0f;
      float clampedSpeed = std::clamp(speedKmh, 0.0f, kMaxSpeedKmh);
      float t = clampedSpeed / kMaxSpeedKmh;
      float angle = kMinNeedleAngle + (kMaxNeedleAngle - kMinNeedleAngle) * t;
      speedPointer->SetProperty("transform",
                                fmt::format("rotate({:.1f}deg)", angle));
      break;
    }
  }

  void UpdateSpeedometerRPM(Engine::Core &core) {
    auto &uiResource = core.GetResource<Rmlui::Resource::UIContext>();
    if (uiResource.GetTitle() != "game" || !IsPauseMenuHidden(uiResource))
      return;

    auto &registry = core.GetRegistry();
    auto view = registry.view<PlayerVehicle>();
    if (view.begin() == view.end())
      return;

    auto &telemetry =
        core.GetResource<Physics::Resource::VehicleTelemetry>();
    constexpr int kLedCount = 41;

    for (auto entity : view) {
      if (!registry.all_of<Physics::Component::Vehicle>(entity))
        continue;
      auto &vehicle = registry.get<Physics::Component::Vehicle>(entity);

      Engine::EntityId eid{
          static_cast<Engine::EntityId::ValueType>(entity)};
      float rpm = telemetry.GetRPM(eid).value_or(0.0f);
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
      break;
    }
  }

  bool IsPauseMenuHidden(Rmlui::Resource::UIContext &uiResource) const {
    auto *pauseMenu = uiResource.GetElementById("pause-menu");
    if (pauseMenu == nullptr)
      return true;

    if (auto *visibilityProp = pauseMenu->GetProperty("visibility")) {
      auto visibility = visibilityProp->Get<Rml::String>();
      return visibility == "hidden";
    }
    return true;
  } */
};
} // namespace Game
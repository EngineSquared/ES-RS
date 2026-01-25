#pragma once

#include "component/Camera.hpp"
#include "component/Transform.hpp"
#include "resource/CameraControlSystemManager.hpp"
#include "resource/CameraManager.hpp"
#include "scenes/VehicleScene.hpp"
#include "system/VehicleInput.hpp"
#include "system/ChildFollowParentSystem.hpp"
#include "utils/OrbitalChaseCameraBehavior.hpp"
#include "utils/AScene.hpp"
#include "Logger.hpp"

namespace Game
{
class CourseScene : public Scene::Utils::AScene {
public:
    CourseScene() {};

protected:
    void _onCreate(Engine::Core &core) final
    {
        Log::Info("Creating CourseScene");

        CreateCheckeredFloor(core);
        auto vehicle = CreateVehicle(core);
        auto light = CreateLight(core);

        /* auto camera = core.GetResource<CameraMovement::Resource::CameraManager>().GetActiveCamera();
 */
        /* camera.AddComponent<Object::Component::Transform>(glm::vec3(0.0f, 1.0f, -10.0f));
        camera.AddComponent<Object::Component::Camera>(); */

        auto &cameraManager = core.GetResource<CameraMovement::Resource::CameraManager>();
        /* cameraManager.SetActiveCamera(camera); */
        cameraManager.SetMovementSpeed(3.0f);

        core.RegisterSystem<Engine::Scheduler::FixedTimeUpdate>(VehicleInput);
        core.RegisterSystem<Engine::Scheduler::FixedTimeUpdate>(ChildFollowParentSystem);
    
        auto chaseBehavior = std::make_shared<OrbitalChaseCameraBehavior>(core, vehicle);
    cameraManager.SetBehavior(chaseBehavior);

    auto &fixedTimeScheduler = core.GetScheduler<Engine::Scheduler::FixedTimeUpdate>();
    fixedTimeScheduler.SetTickRate(1.0f / 120.0f);

    auto &cameraControlSystemManager = core.GetResource<CameraMovement::Resource::CameraControlSystemManager>();
    cameraControlSystemManager.SetCameraControlSystemScheduler<Engine::Scheduler::FixedTimeUpdate>(core);
    }

    void _onDestroy(Engine::Core &core) final
    {
        core.ClearEntities();
    }

private:
};
} // namespace Game
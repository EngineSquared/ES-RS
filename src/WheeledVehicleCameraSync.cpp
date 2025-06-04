#include "WheeledVehicleCameraSync.hpp"

#include "WheeledVehicle3D.hpp"
#include "Logger.hpp"
#include "OpenGL.hpp"

void WheeledVehicleCameraSync::operator()(ES::Engine::Core &core) const
{
    if (!entity.template HasComponents<ES::Plugin::Physics::Component::WheeledVehicle3D>(core))
    {
        ES::Utils::Log::Error(fmt::format("WheeledVehicleCameraSync component is not fully initialized for entity {}",
                                          static_cast<uint32_t>(entity)));
        return;
    }

    auto &wheeledVehicle = entity.template GetComponents<ES::Plugin::Physics::Component::WheeledVehicle3D>(core);
    auto &vehicleBodyTransform = entity.template GetComponents<ES::Plugin::Object::Component::Transform>(core);

    auto &camera = core.GetResource<ES::Plugin::OpenGL::Resource::Camera>();
    camera.viewer.centerAt(vehicleBodyTransform.position);
}
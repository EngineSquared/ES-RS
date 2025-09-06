#include "WheeledVehicleCameraSync.hpp"

#include "component/WheeledVehicle3D.hpp"
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
    const glm::vec3 cameraOffset(0.0f, 3.0f, -8.0f);
    static glm::quat smoothedRotation = vehicleBodyTransform.getRotation();

    smoothedRotation = glm::slerp(smoothedRotation, vehicleBodyTransform.getRotation(), 0.01f);

    camera.viewer.centerAt(vehicleBodyTransform.position);
    camera.viewer.rotate(smoothedRotation, cameraOffset);
}
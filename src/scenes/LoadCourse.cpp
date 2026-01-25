#include "scenes/LoadCourse.hpp"
#include "scenes/SceneUtils.hpp"

#include "Graphic.hpp"
#include "Object.hpp"
#include "Physics.hpp"

#include <glm/glm.hpp>
#include "Logger.hpp"
#include "spdlog/fmt/fmt.h"

/**
 * @brief Load a course from a file
 * @param core The core instance
 */
void LoadCourse(Engine::Core &core)
{
    Log::Info(fmt::format("Loading course model"));
    std::array<Object::OBJLoader, 4> courseLoaders = {
        Object::OBJLoader("asset/course/course_props.obj"),
        Object::OBJLoader("asset/course/course_details.obj"),
        Object::OBJLoader("asset/course/course_ground.obj"),
        Object::OBJLoader("asset/course/course_misc.obj")
    };
    const auto courseOffset = glm::vec3(10.0f, 0.0f, 1530.0f);
    const auto courseScale = glm::vec3(2.5f, 2.5f, 2.5f);
    const auto courseRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

    Object::OBJLoader courseCollider("asset/course/collisions.obj");

    for (auto &loader : courseLoaders)
    {
        auto shapes = loader.GetShapes();
        for (auto &shape : shapes)
        {
            auto coursePart = core.CreateEntity();
            Object::Component::Material shapeMaterial(shape.GetMaterial());
            if (!shapeMaterial.diffuseTexName.empty())
            {
                const std::filesystem::path textureFile = std::filesystem::path(shapeMaterial.diffuseTexName).filename();
                shapeMaterial.diffuseTexName = ("asset/textures/" + textureFile.string()).c_str();
            }
            else
            {
                shapeMaterial.diffuseTexName = Graphic::Utils::DEFAULT_TEXTURE_NAME;
            }

            coursePart.AddComponent<Object::Component::Mesh>(shape.GetMesh());
            coursePart.AddComponent<Object::Component::Material>(shapeMaterial);
            coursePart.AddComponent<Object::Component::Transform>(courseOffset, courseScale, courseRotation);
        }
    }

    Log::Info(fmt::format("Creating course collider"));
    auto colliderEntity = core.CreateEntity();
    colliderEntity.AddComponent<Object::Component::Mesh>(courseCollider.GetMesh());
    colliderEntity.AddComponent<Object::Component::Transform>(courseOffset, courseScale, courseRotation);
    
    Physics::Component::MeshCollider meshCollider;
    meshCollider.activeEdgeCosThresholdAngle = 0.9848077f; // cos(10°) for smoother sliding
    colliderEntity.AddComponent<Physics::Component::MeshCollider>(meshCollider);
    
    colliderEntity.AddComponent<Physics::Component::RigidBody>(Physics::Component::RigidBody::CreateStatic());
}
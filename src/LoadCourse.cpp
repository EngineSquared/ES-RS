#include "LoadCourse.hpp"

#include "Entity.hpp"
#include "Mesh.hpp"
#include "OpenGL.hpp"
#include "Object.hpp"
#include "RigidBody3D.hpp"

#include "JoltPhysics.hpp"
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <filesystem>
#include <iostream>
#include <string>

void LoadCourseModels(ES::Engine::Core &core, const glm::vec3 &position, const glm::quat &rotation, const glm::vec3 &size)
{
    namespace fs = std::filesystem;

    const std::string directoryPath = "asset/course_model/";
    auto &textureManager = core.GetResource<ES::Plugin::OpenGL::Resource::TextureManager>();

    for (const auto &entry : fs::directory_iterator(directoryPath))
    {
        if (!entry.is_regular_file())
            continue;

        const std::string filePath = entry.path().string();
        if (entry.path().extension() != ".obj")
            continue;

        const std::string filename = entry.path().stem().string();

        ES::Plugin::Object::Component::Mesh objMesh;
        if (!ES::Plugin::Object::Resource::OBJLoader::loadModel(
            filePath,
            objMesh.vertices,
            objMesh.normals,
            objMesh.texCoords,
            objMesh.indices
        )) {
            ES::Utils::Log::Error(fmt::format("Failed to load model: {}", filePath));
            continue;
        }

        ES::Engine::Entity entity = core.CreateEntity();
        entity.AddComponent<ES::Plugin::Object::Component::Mesh>(core, objMesh);
        entity.AddComponent<ES::Plugin::Object::Component::Transform>(core, position, size, rotation);

        std::string textureName = "tex_" + filename;
        if (textureManager.Contains(entt::hashed_string{textureName.c_str()})) {
            entity.AddComponent<ES::Plugin::OpenGL::Component::TextureHandle>(core, textureName);
            ES::Utils::Log::Info(fmt::format("Course OBJ {} loaded with texture {}", filename, textureName));
        }
        else {
            entity.AddComponent<ES::Plugin::OpenGL::Component::TextureHandle>(core, "tex_PLACEHOLDER");
            ES::Utils::Log::Error(fmt::format("No texture found for model: {}", filename));
        }

        entity.AddComponent<ES::Plugin::OpenGL::Component::ShaderHandle>(core, "texture");
        entity.AddComponent<ES::Plugin::OpenGL::Component::MaterialHandle>(core, "level");
        entity.AddComponent<ES::Plugin::OpenGL::Component::ModelHandle>(core, fmt::format("level_{}", filename).c_str());
    }
    LoadCourseCollision(core, size);
}

void LoadCourseCollision(ES::Engine::Core &core, const glm::vec3 &size)
{
    const std::string colPath = "asset/collisions/level_collision.obj";
    ES::Engine::Entity entity = core.CreateEntity();
    ES::Plugin::Object::Component::Mesh objMesh;

    if (!ES::Plugin::Object::Resource::OBJLoader::loadModel(
        colPath,
        objMesh.vertices,
        objMesh.normals,
        objMesh.texCoords,
        objMesh.indices
    )) {
        ES::Utils::Log::Error(fmt::format("Failed to load model: {}", colPath));
    }

    JPH::VertexList vertexList;
    vertexList.reserve(objMesh.vertices.size());
    for (const auto &v : objMesh.vertices) {
        vertexList.emplace_back(v.x * size.x, v.y * size.y, v.z * size.z);
    }

    JPH::IndexedTriangleList triangleList;
    for (size_t i = 0; i + 2 < objMesh.indices.size(); i += 3) {
        triangleList.emplace_back(
            static_cast<uint32_t>(objMesh.indices[i]),
            static_cast<uint32_t>(objMesh.indices[i + 1]),
            static_cast<uint32_t>(objMesh.indices[i + 2])
        );
    }

    auto meshShapeSettings = std::make_shared<JPH::MeshShapeSettings>(vertexList, triangleList);

    entity.AddComponent<ES::Plugin::Physics::Component::RigidBody3D>(core, meshShapeSettings, JPH::EMotionType::Static, ES::Plugin::Physics::Utils::Layers::NON_MOVING);
    ES::Utils::Log::Info("Loaded course collision");
}

void LoadCourseTextures(ES::Engine::Core &core)
{
    auto &textureManager = core.GetResource<ES::Plugin::OpenGL::Resource::TextureManager>();

    for (auto &textureData : textureList)
    {
        textureManager.Add(
            entt::hashed_string{std::get<0>(textureData).c_str()},
            std::get<1>(textureData).c_str(),
            std::get<2>(textureData)
        );
    }
}

void UnloadCourseTextures(ES::Engine::Core &core)
{
    auto &textureManager = core.GetResource<ES::Plugin::OpenGL::Resource::TextureManager>();

    for (auto &textureData : textureList)
    {
        textureManager.Remove(
            entt::hashed_string{std::get<0>(textureData).c_str()}
        );
    }
}
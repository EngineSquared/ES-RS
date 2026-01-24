#include "scenes/VehicleScene.hpp"
#include "component/Camera.hpp"
#include "component/PlayerVehicle.hpp"
#include "component/ChildOffset.hpp"

#include "Graphic.hpp"
#include "Object.hpp"
#include "Physics.hpp"
#include "Relationship.hpp"
#include "builder/VehicleBuilder.hpp"
#include "component/Transform.hpp"
#include "component/VehicleController.hpp"
#include "resource/OBJLoader.hpp"
#include "utils/BoxGenerator.hpp"

#include <glm/glm.hpp>
#include "Logger.hpp"
#include "spdlog/fmt/fmt.h"

namespace {
constexpr float kCourseSurfaceY = 50.0f;
constexpr float kVehicleSpawnYOffset = 2.0f;
} // namespace

/**
 * @brief Create a checkered floor (200x200 meters) with alternating grey tiles
 *
 * Uses a single large physics body to avoid ghost collisions at tile edges,
 * while creating separate visual tiles for the checkered pattern.
 */
void CreateCheckeredFloor(Engine::Core &core)
{
    const float tileSize = 10.0f;
    const int tilesPerSide = 20; // 20 tiles * 10m = 200m
    const float totalSize = tileSize * tilesPerSide;
    const float startOffset = -totalSize / 2.0f;

    Log::Info(fmt::format("Creating {}x{} checkered floor...", tilesPerSide, tilesPerSide));

    // Create a single large physics floor to avoid ghost collisions at tile edges
    auto floorPhysics = core.CreateEntity();
    floorPhysics.AddComponent<Object::Component::Transform>(glm::vec3(0.0f, kCourseSurfaceY, 0.0f));
    auto floorCollider = Physics::Component::BoxCollider(glm::vec3(totalSize / 2.0f, 0.1f, totalSize / 2.0f));
    floorPhysics.AddComponent<Physics::Component::BoxCollider>(floorCollider);
    floorPhysics.AddComponent<Physics::Component::RigidBody>(Physics::Component::RigidBody::CreateStatic());

    // Create visual tiles (no physics) for the checkered pattern
    for (int x = 0; x < tilesPerSide; ++x)
    {
        for (int z = 0; z < tilesPerSide; ++z)
        {
            float posX = startOffset + (x * tileSize) + (tileSize / 2.0f);
            float posZ = startOffset + (z * tileSize) + (tileSize / 2.0f);

            bool isLightTile = (x + z) % 2 == 0;
            // Use a dark / light grey checker pattern
            glm::vec3 color = isLightTile ? glm::vec3(0.6f, 0.6f, 0.6f) : glm::vec3(0.2f, 0.2f, 0.2f);

            auto tile = Object::Helper::CreatePlane(core, {.width = tileSize,
                                                           .depth = tileSize,
                                                           .position = glm::vec3(posX, kCourseSurfaceY, posZ),
                                                           .rotation = glm::vec3(0.0f, 0.0f, 0.0f)});

            Object::Component::Material tileMaterial;
            tileMaterial.diffuse = color;
            tileMaterial.ambient = color * 0.3f;
            tileMaterial.specular = glm::vec3(0.1f);
            tileMaterial.shininess = 16.0f;
            // Use engine default texture (1x1) so material color is used without external textures
            //tileMaterial.ambientTexName = Graphic::Utils::DEFAULT_TEXTURE_NAME;
            tile.AddComponent<Object::Component::Material>(tileMaterial);

            // No physics on visual tiles - single floor body handles collision
        }
    }
}


/**
 * @brief Adjust a mesh position by a given offset
 * 
 * @param mesh  Mesh to adjust
 * @param offset Offset to apply
 */
void AdjustMeshPosition(Object::Component::Mesh &mesh, const glm::vec3 &offset)
{
    auto &vertices = mesh.GetVertices();
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        mesh.SetVertexAt(i, vertices[i] + offset);
    }
}

/**
 * @brief Load a course from a file
 * @param core The core instance
 * @param filePath The path to the course file to load
 */
void LoadCourse(Engine::Core &core, const std::string &modelPath, const std::string &colliderPath)
{
    std::cout << "Loading course: " << modelPath << " with collider: " << colliderPath << std::endl;
    Object::OBJLoader courseModel(modelPath);
    Object::OBJLoader courseCollider(colliderPath);
    auto mesh = courseModel.GetMesh();
    auto materials = courseModel.GetMaterials();
    auto colliderShapes = courseCollider.GetShapes();
    auto courseEntity = core.CreateEntity();

    courseEntity.AddComponent<Object::Component::Mesh>(mesh);
    courseEntity.AddComponent<Object::Component::Material>(materials[0]);
    courseEntity.AddComponent<Physics::Component::BoxCollider>(Physics::Component::BoxCollider(glm::vec3(20.0f, 0.1f, 20.0f)));
    courseEntity.AddComponent<Physics::Component::RigidBody>(Physics::Component::RigidBody::CreateStatic());
}

/**
 * @brief Create a drivable vehicle using VehicleBuilder
 *
 * Loads all shapes from the car OBJ file. The main body shape is used for physics,
 * while other shapes are created as child entities that follow the main body.
 */
Engine::Entity CreateVehicle(Engine::Core &core)
{
    using enum Physics::Component::WheelIndex;
    using enum Physics::Component::DrivetrainType;

    Object::Component::Mesh chassisMesh;
    Object::Component::Material chassisMaterial;
    std::vector<Object::Resource::Shape> otherShapes;

    chassisMaterial.diffuse = glm::vec3(0.4f, 0.7f, 0.95f);
    chassisMaterial.ambient = chassisMaterial.diffuse * 0.3f;
    chassisMaterial.specular = glm::vec3(0.3f);
    chassisMaterial.shininess = 32.0f;

    try
    {
        Object::OBJLoader loader("asset/car/gt3rs_test.obj");
        bool foundShape = false;

        for (auto &shape : loader.GetShapes())
        {
            if (shape.GetName() == "GEO_Body_SUB3")
            {
                Log::Debug(fmt::format("Found vehicle body shape: {}", shape.GetName()));
                chassisMesh = shape.GetMesh();
                chassisMaterial = shape.GetMaterial();
                Log::Warn(fmt::format("chassis texture name: {}", chassisMaterial.ambientTexName));
                foundShape = true;
            }
            else
            {
                otherShapes.push_back(shape);
                Log::Debug(fmt::format("Found additional vehicle shape: {}", shape.GetName()));
            }
        }

        if (!foundShape)
        {
            Log::Warn("Shape GEO_Body_SUB3 not found, using full mesh.");
            chassisMesh = loader.GetMesh();
        }
    }
    catch (const Object::OBJLoaderError &e)
    {
        Log::Error(fmt::format("Failed to load vehicle chassis mesh: {}", e.what()));
        chassisMesh = Object::Utils::GenerateBoxMesh(1.0f, 0.8f, 2.0f);
    }

    // Adjust chassis mesh and shapes mesh with a Y offset of -0.69 to set the origin at the bottom of the chassis
    AdjustMeshPosition(chassisMesh, glm::vec3(0.0f, -0.69f, 0.0f));
    for (auto &shape : otherShapes)
    {
        auto &mesh = shape.GetMesh();
        AdjustMeshPosition(mesh, glm::vec3(0.0f, -0.69f, 0.0f));
    }

    chassisMaterial.ambientTexName = Graphic::Utils::DEFAULT_TEXTURE_NAME; // Temp fix while diffuse textures are not available
    float wheelRadius = 0.3f;
    float wheelWidth = 0.3f;

    Object::Component::Mesh wheelMesh = Object::Utils::GenerateWheelMesh(wheelRadius, wheelWidth);

    Physics::Component::WheelSettings frontWheel = Physics::Component::WheelSettings::CreateFrontWheel();
    frontWheel.radius = wheelRadius;
    frontWheel.width = wheelWidth;
    frontWheel.longitudinalFriction = 2.5f;
    frontWheel.lateralFriction = 2.0f;
    frontWheel.suspensionMinLength = 0.1f;
    frontWheel.suspensionMaxLength = 0.3f;

    Physics::Component::WheelSettings rearWheel = Physics::Component::WheelSettings::CreateRearWheel();
    rearWheel.radius = wheelRadius;
    rearWheel.width = wheelWidth;
    rearWheel.longitudinalFriction = 2.5f;
    rearWheel.lateralFriction = 2.0f;
    rearWheel.suspensionMinLength = 0.1f;
    rearWheel.suspensionMaxLength = 0.3f;
    
    glm::vec3 frontLeftWheelPos = glm::vec3(-1.0f, -0.5f, 1.5f);
    glm::vec3 frontRightWheelPos = glm::vec3(1.0f, -0.5f, 1.5f);
    glm::vec3 rearLeftWheelPos = glm::vec3(-1.0f, -0.5f, -1.5f);
    glm::vec3 rearRightWheelPos = glm::vec3(1.0f, -0.5f, -1.5f);
    glm::vec3 chassisPos = glm::vec3(0.0f, 3.0f, 0.0f);

    Physics::Builder::VehicleBuilder<4> builder;
    auto vehicleEntity =
        builder.SetChassisMesh(chassisMesh, glm::vec3(0.0f, kCourseSurfaceY + kVehicleSpawnYOffset, 0.0f))
                             .SetWheelMesh(FrontLeft, wheelMesh)
                             .SetWheelMesh(FrontRight, wheelMesh)
                             .SetWheelMesh(RearLeft, wheelMesh)
                             .SetWheelMesh(RearRight, wheelMesh)
                             .SetWheelSettings(FrontLeft, frontWheel)
                             .SetWheelSettings(FrontRight, frontWheel)
                             .SetWheelSettings(RearLeft, rearWheel)
                             .SetWheelSettings(RearRight, rearWheel)
                             .SetDrivetrain(RWD)
                             //.SetWheelPositions(frontLeftWheelPos, frontRightWheelPos, rearLeftWheelPos, rearRightWheelPos)
                             .SetChassisMass(500.0f)
                             .SetChassisHalfExtents(glm::vec3(0.5f, 0.4f, 1.0f))
                             .Build(core);

    Object::Component::Material chassisMaterial;
    // Light blue car color
    chassisMaterial.diffuse = glm::vec3(0.4f, 0.7f, 0.95f);
    chassisMaterial.ambient = chassisMaterial.diffuse * 0.3f;
    chassisMaterial.specular = glm::vec3(0.3f);
    chassisMaterial.shininess = 32.0f;
    // Use engine default texture to avoid missing texture warnings while keeping a plain material
    //chassisMaterial.ambientTexName = Graphic::Utils::DEFAULT_TEXTURE_NAME;
    vehicleEntity.AddComponent<Object::Component::Material>(chassisMaterial);
    vehicleEntity.AddComponent<PlayerVehicle>();

    vehicleEntity.AddComponent<Relationship::Component::Relationship>();

    for (auto &shape : otherShapes)
    {
        auto childEntity = core.CreateEntity();

        childEntity.AddComponent<Object::Component::Mesh>(shape.GetMesh());
        
        auto &shapeMaterial = shape.GetMaterial();
        shapeMaterial.ambientTexName = Graphic::Utils::DEFAULT_TEXTURE_NAME;
        childEntity.AddComponent<Object::Component::Material>(shapeMaterial);

        childEntity.AddComponent<Object::Component::Transform>(glm::vec3(0.0f, 2.0f, 0.0f));

        childEntity.AddComponent<ChildOffset>();

        childEntity.AddComponent<Relationship::Component::Relationship>();

        Relationship::Utils::SetChildOf(childEntity, vehicleEntity);

        Log::Debug(fmt::format("Created child entity for shape: {}", shape.GetName()));
    }

    Log::Info(fmt::format("Vehicle created with {} child shapes", otherShapes.size()));

    return vehicleEntity;
}

Engine::Entity CreateLight(Engine::Core &core)
{
    auto pointLight = core.CreateEntity();
    pointLight.AddComponent<Object::Component::Transform>(glm::vec3(-2.0f, 7.0f, -1.0f));
    pointLight.AddComponent<Object::Component::PointLight>(
        Object::Component::PointLight{.color = glm::vec3(1.0f, 1.0f, 1.0f),
                                      .intensity = 3.0f,
                                      .radius = 100.0f,
                                      .falloff = 1.0f});
}
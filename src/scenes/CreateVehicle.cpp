#include "scenes/CreateVehicle.hpp"
#include "scenes/SceneUtils.hpp"

#include "component/PlayerVehicle.hpp"
#include "component/ChildOffset.hpp"

#include "Graphic.hpp"
#include "Object.hpp"
#include "Physics.hpp"
#include "Relationship.hpp"
#include "builder/VehicleBuilder.hpp"
#include "component/Transform.hpp"
#include "component/VehicleController.hpp"

#include <glm/glm.hpp>
#include "Logger.hpp"
#include "spdlog/fmt/fmt.h"

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

    std::string shapeName = "TwiXeR_992_underbody_gt3rs";

    glm::vec3 meshOffset = glm::vec3(0.0f, -0.6f, 0.0f);

    try
    {
        Object::OBJLoader loader("asset/car2/GT3_RS.obj");
        bool foundShape = false;

        for (auto &shape : loader.GetShapes())
        {
            if (shape.GetName().starts_with(shapeName))
            {
                Log::Debug(fmt::format("Found vehicle body shape: {}", shape.GetName()));
                chassisMesh = shape.GetMesh();
                chassisMaterial = shape.GetMaterial();
                if (!chassisMaterial.diffuseTexName.empty())
                {
                    chassisMaterial.diffuseTexName = "asset/car2/" + chassisMaterial.diffuseTexName;
                }
                else {
                    chassisMaterial.diffuseTexName = Graphic::Utils::DEFAULT_TEXTURE_NAME;
                }

                Log::Debug(fmt::format("chassis texture name: {}", chassisMaterial.diffuseTexName));
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
            Log::Warn(fmt::format("Vehicle body shape '{}' not found in OBJ file, using entire mesh as chassis", shapeName));
            chassisMesh = loader.GetMesh();
        }
    }
    catch (const Object::OBJLoaderError &e)
    {
        Log::Error(fmt::format("Failed to load vehicle chassis mesh: {}", e.what()));
        chassisMesh = Object::Utils::GenerateBoxMesh(1.0f, 0.8f, 2.0f);
    }

    // Adjust chassis mesh and shapes mesh with a Y offset to set the origin at the bottom of the chassis
    AdjustMeshPosition(chassisMesh, meshOffset);
    chassisMesh = InvertMeshX(chassisMesh);
    InvertMeshUVs(chassisMesh, true, false);

    for (auto &shape : otherShapes)
    {
        auto &mesh = shape.GetMesh();
        AdjustMeshPosition(mesh, meshOffset);
        shape.mesh = InvertMeshX(mesh);
        InvertMeshUVs(shape.mesh, true, false);
    }

    chassisMaterial.diffuseTexName = Graphic::Utils::DEFAULT_TEXTURE_NAME; // Temp fix while diffuse textures are not available
    float wheelRadius = 0.692f / 2.0f;
    float wheelWidth = 0.277f;

    Object::Component::Mesh wheelMesh = Object::Utils::GenerateWheelMesh(wheelRadius, wheelWidth);

    Physics::Component::WheelSettings frontWheel = Physics::Component::WheelSettings::CreateFrontWheel();
    frontWheel.radius = wheelRadius;
    frontWheel.width = wheelWidth;
    // High-performance tire friction curves
    frontWheel.longitudinalFriction = {
        {0.0f, 0.0f},
        {0.05f, 1.4f},   // Racing tire peak grip
        {0.15f, 1.2f}
    };
    frontWheel.lateralFriction = {
        {0.0f, 0.0f},
        {2.5f, 1.4f},    // High cornering grip
        {15.0f, 1.1f}
    };
    frontWheel.suspensionMinLength = 0.1f;
    frontWheel.suspensionMaxLength = 0.3f;

    Physics::Component::WheelSettings rearWheel = Physics::Component::WheelSettings::CreateRearWheel();
    rearWheel.radius = wheelRadius;
    rearWheel.width = wheelWidth;
    // High-performance tire friction curves
    rearWheel.longitudinalFriction = {
        {0.0f, 0.1f},
        {0.1f, 2.5f},
        {0.5f, 2.2f},
        {1.0f, 2.0f}
    };
    rearWheel.lateralFriction = {
        {0.0f, 0.1f},
        {5.0f, 2.5f},
        {20.0f, 2.0f}
    };
    rearWheel.suspensionMinLength = 0.1f;
    rearWheel.suspensionMaxLength = 0.3f;

    glm::vec3 frontLeftWheelPos = glm::vec3(-0.9f, -0.3f, 1.1f);
    glm::vec3 frontRightWheelPos = glm::vec3(0.9f, -0.3f, 1.1f);
    glm::vec3 rearLeftWheelPos = glm::vec3(-0.9f, -0.3f, -1.35f);
    glm::vec3 rearRightWheelPos = glm::vec3(0.9f, -0.3f, -1.35f);
    glm::vec3 chassisPos = glm::vec3(0.0f, 300.0f, 0.0f);

    // GT3 RS
    Physics::Component::EngineSettings engineSettings;
    engineSettings.maxRPM = 9000.0f;
    engineSettings.minRPM = 950.0f;
    engineSettings.maxTorque = 465.0f; // Nm
    engineSettings.inertia = 0.25f;
    engineSettings.angularDamping = 0.15f;

    float chassisMass = 1450.0f; // kg

    // Gearbox - GT3 RS 7-speed PDK
    Physics::Component::GearboxSettings gearboxSettings;
    gearboxSettings.forwardGearRatios = {3.66f, 2.43f, 1.69f, 1.31f, 1.0f, 0.84f, 0.69f};
    gearboxSettings.reverseGearRatios = {-3.91f};
    gearboxSettings.clutchReleaseTime = 0.2f;
    gearboxSettings.switchTime = 0.10f;  // Fast dual-clutch
    gearboxSettings.shiftUpRPM = 6500.0f; // Usually it's higher, this is between comfort and sport mode
    gearboxSettings.shiftDownRPM = 2000.0f;

    // Torque curve
    engineSettings.normalizedTorque = {
        {0.0f,  0.7f}, // 70% torque at minRPM
        {0.3f,  0.9f},
        {0.6f,  1.0f}, // 100% torque at 60% of RPM range (peak)
        {0.85f, 0.95f},
        {1.0f,  0.8f}  // 80% torque at maxRPM
    };

    Physics::Builder::VehicleBuilder<4> builder;
    auto vehicleEntity = builder.SetChassisMesh(chassisMesh, chassisPos)
                             .SetWheelMesh(FrontLeft, wheelMesh)
                             .SetWheelMesh(FrontRight, wheelMesh)
                             .SetWheelMesh(RearLeft, wheelMesh)
                             .SetWheelMesh(RearRight, wheelMesh)
                             .SetWheelSettings(FrontLeft, frontWheel)
                             .SetWheelSettings(FrontRight, frontWheel)
                             .SetWheelSettings(RearLeft, rearWheel)
                             .SetWheelSettings(RearRight, rearWheel)
                             .SetDrivetrain(RWD)
                             .SetWheelPositions(frontLeftWheelPos, frontRightWheelPos, rearLeftWheelPos, rearRightWheelPos)
                             .SetChassisMass(chassisMass)
                             .SetEngine(engineSettings)
                             .SetGearbox(gearboxSettings)
                             .Build(core);

    vehicleEntity.AddComponent<Object::Component::Material>(chassisMaterial);
    vehicleEntity.AddComponent<PlayerVehicle>();

    vehicleEntity.AddComponent<Relationship::Component::Relationship>();

    for (auto &shape : otherShapes)
    {
        auto childEntity = core.CreateEntity();

        childEntity.AddComponent<Object::Component::Mesh>(shape.GetMesh());

        {
            auto &shapeMaterial = shape.GetMaterial();
            if (!shapeMaterial.diffuseTexName.empty())
            {
                shapeMaterial.diffuseTexName = "asset/car2/" + shapeMaterial.diffuseTexName;
            }
            else {
                shapeMaterial.diffuseTexName = Graphic::Utils::DEFAULT_TEXTURE_NAME;
            }

            childEntity.AddComponent<Object::Component::Material>(shapeMaterial);
        }

        childEntity.AddComponent<Object::Component::Transform>(glm::vec3(0.0f, 2.0f, 0.0f));

        childEntity.AddComponent<ChildOffset>();

        childEntity.AddComponent<Relationship::Component::Relationship>();

        Relationship::Utils::SetChildOf(childEntity, vehicleEntity);

        Log::Debug(fmt::format("Created child entity for shape: {}", shape.GetName()));
    }

    Log::Info(fmt::format("Vehicle created with {} child shapes", otherShapes.size()));

    return vehicleEntity;
}
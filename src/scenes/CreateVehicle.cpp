#include "scenes/CreateVehicle.hpp"
#include "scenes/SceneUtils.hpp"

#include "component/ChildOffset.hpp"
#include "component/PlayerVehicle.hpp"

#include "Graphic.hpp"
#include "Object.hpp"
#include "Physics.hpp"
#include "Relationship.hpp"
#include "builder/VehicleBuilder.hpp"
#include "component/Transform.hpp"
#include "component/VehicleController.hpp"

#include "Logger.hpp"
#include "spdlog/fmt/fmt.h"
#include <glm/glm.hpp>

/**
 * @brief Create a drivable vehicle using VehicleBuilder
 *
 * Loads all shapes from the car OBJ file. The main body shape is used for
 * physics, while other shapes are created as child entities that follow the
 * main body.
 */
Engine::Entity CreateVehicle(Engine::Core &core) {
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

  try {
    Object::OBJLoader loader("asset/car2/GT3_RS.obj");
    bool foundShape = false;

    for (auto &shape : loader.GetShapes()) {
      if (shape.GetName().starts_with(shapeName)) {
        Log::Debug(
            fmt::format("Found vehicle body shape: {}", shape.GetName()));
        chassisMesh = shape.GetMesh();
        chassisMaterial = shape.GetMaterial();
        if (!chassisMaterial.diffuseTexName.empty()) {
          chassisMaterial.diffuseTexName =
              "asset/car2/" + chassisMaterial.diffuseTexName;
        } else {
          chassisMaterial.diffuseTexName = Graphic::Utils::DEFAULT_TEXTURE_NAME;
        }

        Log::Debug(fmt::format("chassis texture name: {}",
                               chassisMaterial.diffuseTexName));
        foundShape = true;
      } else {
        otherShapes.push_back(shape);
        Log::Debug(
            fmt::format("Found additional vehicle shape: {}", shape.GetName()));
      }
    }

    if (!foundShape) {
      Log::Warning(fmt::format("Vehicle body shape '{}' not found in OBJ file, "
                            "using entire mesh as chassis",
                            shapeName));
      chassisMesh = loader.GetMesh();
    }
  } catch (const Object::OBJLoaderError &e) {
    Log::Error(
        fmt::format("Failed to load vehicle chassis mesh: {}", e.what()));
    chassisMesh = Object::Utils::GenerateBoxMesh(1.0f, 0.8f, 2.0f);
  }

  // Adjust chassis mesh and shapes mesh with a Y offset to set the origin at
  // the bottom of the chassis
  AdjustMeshPosition(chassisMesh, meshOffset);
  chassisMesh = InvertMeshX(chassisMesh);
  InvertMeshUVs(chassisMesh, true, false);

  for (auto &shape : otherShapes) {
    auto &mesh = shape.GetMesh();
    AdjustMeshPosition(mesh, meshOffset);
    shape.mesh = InvertMeshX(mesh);
    InvertMeshUVs(shape.mesh, true, false);
  }

  chassisMaterial.diffuseTexName =
      Graphic::Utils::DEFAULT_TEXTURE_NAME; // Temp fix while diffuse textures
                                            // are not available
  float wheelRadius = 0.692f / 2.0f;
  float wheelWidth = 0.277f;

  Object::Component::Mesh wheelMesh =
      Object::Utils::GenerateWheelMesh(wheelRadius, wheelWidth);

  Physics::Component::WheelSettings frontWheel =
      Physics::Component::WheelSettings::CreateFrontWheel();
  frontWheel.radius = wheelRadius;
  frontWheel.width = wheelWidth;
  // FLAT friction curve - prevents wheelspin feedback loop
  // High constant friction at all slip levels
  frontWheel.longitudinalFriction = {
      {0.0f, 2.0f}, // Very high friction at all times
      {1.0f, 2.0f}  // Stays constant - no drop off
  };
  // Front lateral friction - high for steering
  frontWheel.lateralFriction = {
      {0.0f, 2.0f}, {3.0f, 2.0f}, {10.0f, 1.8f}, {30.0f, 1.5f}};
  frontWheel.suspensionMinLength = 0.05f;
  frontWheel.suspensionMaxLength = 0.35f;
  frontWheel.suspensionFrequency = 1.8f;
  frontWheel.suspensionDamping = 0.4f;
  frontWheel.maxBrakeTorque = 4000.0f;
  frontWheel.inertia = 0.9f;        // Higher inertia to resist spinning
  frontWheel.angularDamping = 0.5f; // High damping to prevent wheelspin
  frontWheel.maxSteerAngle = 0.7f;  // ~40 degrees - tighter turn radius

  Physics::Component::WheelSettings rearWheel =
      Physics::Component::WheelSettings::CreateRearWheel();
  rearWheel.radius = wheelRadius;
  rearWheel.width = wheelWidth;
  // FLAT friction curve for rear - maximum traction, no wheelspin
  rearWheel.longitudinalFriction = {
      {0.0f, 2.5f}, // Even higher friction for drive wheels
      {0.5f, 2.5f}, // Constant high friction
      {1.0f, 2.5f}  // No drop - tires ALWAYS grip
  };
  // Rear lateral friction
  rearWheel.lateralFriction = {
      {0.0f, 1.8f}, {5.0f, 1.8f}, {15.0f, 1.5f}, {35.0f, 1.2f}};
  rearWheel.suspensionMinLength = 0.05f;
  rearWheel.suspensionMaxLength = 0.35f;
  rearWheel.suspensionFrequency = 1.6f;
  rearWheel.suspensionDamping = 0.45f;
  rearWheel.maxBrakeTorque = 3000.0f;
  rearWheel.maxHandBrakeTorque = 6000.0f;
  rearWheel.inertia = 1.2f;        // Higher inertia to resist spinning
  rearWheel.angularDamping = 0.5f; // High damping prevents wheelspin

  glm::vec3 frontLeftWheelPos = glm::vec3(-0.9f, -0.3f, 1.1f);
  glm::vec3 frontRightWheelPos = glm::vec3(0.9f, -0.3f, 1.1f);
  glm::vec3 rearLeftWheelPos = glm::vec3(-0.9f, -0.3f, -1.35f);
  glm::vec3 rearRightWheelPos = glm::vec3(0.9f, -0.3f, -1.35f);
  glm::vec3 chassisPos = glm::vec3(20.0f, 300.0f, 0.0f);

  // GT3 RS - tuned for quick acceleration (arcade-style)
  Physics::Component::EngineSettings engineSettings;
  engineSettings.maxRPM = 9000.0f;
  engineSettings.minRPM = 950.0f;
  engineSettings.maxTorque = 800.0f;     // High torque for quick acceleration
  engineSettings.inertia = 0.15f;        // Very light engine
  engineSettings.angularDamping = 0.05f; // Minimal resistance

  float chassisMass = 1350.0f; // kg - lighter for faster acceleration

  // Gearbox - GT3 RS 7-speed PDK
  Physics::Component::GearboxSettings gearboxSettings;
  gearboxSettings.forwardGearRatios = {3.66f, 2.43f, 1.69f, 1.31f,
                                       1.0f,  0.84f, 0.69f};
  gearboxSettings.reverseGearRatios = {-3.91f};
  gearboxSettings.clutchReleaseTime = 0.1f; // Very fast clutch
  gearboxSettings.switchTime = 0.05f;       // Instant shifts
  gearboxSettings.shiftUpRPM = 8500.0f;     // Shift near redline
  gearboxSettings.shiftDownRPM = 3500.0f;   // Keep in power band
  gearboxSettings.clutchStrength = 40.0f;   // Very strong clutch

  // Torque curve
  engineSettings.normalizedTorque = {
      {0.0f, 0.7f}, // 70% torque at minRPM
      {0.3f, 0.9f},
      {0.6f, 1.0f}, // 100% torque at 60% of RPM range (peak)
      {0.85f, 0.95f},
      {0.95f, 0.8f}, // 80% torque at maxRPM
      {1.0f, 0.0f}
  };

  Physics::Builder::VehicleBuilder<4> builder;
  auto vehicleEntity =
      builder.SetChassisMesh(chassisMesh, chassisPos)
          .SetWheelMesh(FrontLeft, wheelMesh)
          .SetWheelMesh(FrontRight, wheelMesh)
          .SetWheelMesh(RearLeft, wheelMesh)
          .SetWheelMesh(RearRight, wheelMesh)
          .SetWheelSettings(FrontLeft, frontWheel)
          .SetWheelSettings(FrontRight, frontWheel)
          .SetWheelSettings(RearLeft, rearWheel)
          .SetWheelSettings(RearRight, rearWheel)
          .SetDrivetrain(RWD)
          .SetWheelPositions(frontLeftWheelPos, frontRightWheelPos,
                             rearLeftWheelPos, rearRightWheelPos)
          .SetCollisionTesterType(
              Physics::Component::CollisionTesterType::CastCylinder)
          .SetConvexRadiusFraction(
              0.1f) // Lower value = smoother terrain transitions
          .SetChassisMass(chassisMass)
          .SetEngine(engineSettings)
          .SetGearbox(gearboxSettings)
          .Build(core);

  vehicleEntity.AddComponent<Object::Component::Material>(chassisMaterial);
  vehicleEntity.AddComponent<PlayerVehicle>();

  vehicleEntity.AddComponent<Relationship::Component::Relationship>();

  for (auto &shape : otherShapes) {
    auto childEntity = core.CreateEntity();

    childEntity.AddComponent<Object::Component::Mesh>(shape.GetMesh());

    {
      auto &shapeMaterial = shape.GetMaterial();
      if (!shapeMaterial.diffuseTexName.empty()) {
        shapeMaterial.diffuseTexName =
            "asset/car2/" + shapeMaterial.diffuseTexName;
      } else {
        shapeMaterial.diffuseTexName = Graphic::Utils::DEFAULT_TEXTURE_NAME;
      }

      childEntity.AddComponent<Object::Component::Material>(shapeMaterial);
    }

    childEntity.AddComponent<Object::Component::Transform>(
        glm::vec3(0.0f, 2.0f, 0.0f));

    childEntity.AddComponent<ChildOffset>();

    childEntity.AddComponent<Relationship::Component::Relationship>();

    Relationship::Utils::SetChildOf(childEntity, vehicleEntity);

    Log::Debug(
        fmt::format("Created child entity for shape: {}", shape.GetName()));
  }

  Log::Info(
      fmt::format("Vehicle created with {} child shapes", otherShapes.size()));

  return vehicleEntity;
}

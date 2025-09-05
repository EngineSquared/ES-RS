#include "CreateVehicle.hpp"

#include "CreateBox.hpp"
#include "CreateCylinder.hpp"
#include "JoltPhysics.hpp"
#include "OpenGL.hpp"
#include "WheeledVehicleKeyboardMovement.hpp"
#include "WheeledVehicleControllerMovement.hpp"
#include "WheeledVehicleCameraSync.hpp"

#include <Jolt/Physics/Vehicle/VehicleAntiRollBar.h>
#include <Jolt/Physics/Vehicle/VehicleCollisionTester.h>
#include <Jolt/Physics/Vehicle/VehicleConstraint.h>
#include <Jolt/Physics/Vehicle/VehicleController.h>
#include <Jolt/Physics/Vehicle/VehicleDifferential.h>
#include <Jolt/Physics/Vehicle/VehicleEngine.h>
#include <Jolt/Physics/Vehicle/VehicleTrack.h>
#include <Jolt/Physics/Vehicle/VehicleTransmission.h>
#include <Jolt/Physics/Vehicle/Wheel.h>
#include <Jolt/Physics/Vehicle/WheeledVehicleController.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>

ES::Utils::FunctionContainer::FunctionID movementSystemId = std::numeric_limits<std::size_t>::max();
ES::Utils::FunctionContainer::FunctionID controllerSystemId = std::numeric_limits<std::size_t>::max();
ES::Utils::FunctionContainer::FunctionID cameraSystemId = std::numeric_limits<std::size_t>::max();

static ES::Engine::Entity CreateVehicleBody(
    ES::Engine::Core &core,
    const glm::vec3 &position,
    float halfVehicleLength,
    float halfVehicleWidth,
    float halfVehicleHeight
)
{
    std::shared_ptr<JPH::ShapeSettings> carShapeSettings = std::make_shared<JPH::BoxShapeSettings>(
        JPH::Vec3(halfVehicleLength, halfVehicleHeight, halfVehicleWidth)
    );
    carShapeSettings->SetEmbedded();

    ES::Engine::Entity vehicleBody = core.CreateEntity();
    vehicleBody.AddComponent<ES::Plugin::Object::Component::Transform>(core, position);
    vehicleBody.AddComponent<ES::Plugin::Physics::Component::RigidBody3D>(
        core,
        carShapeSettings,
        JPH::EMotionType::Dynamic,
        ES::Plugin::Physics::Utils::Layers::MOVING);

    vehicleBody.AddComponent<ES::Plugin::OpenGL::Component::ShaderHandle>(core, "noTextureLightShadow");
    vehicleBody.AddComponent<ES::Plugin::OpenGL::Component::MaterialHandle>(core, "car_body");
    vehicleBody.AddComponent<ES::Plugin::OpenGL::Component::ModelHandle>(core, "car_body");
    vehicleBody.AddComponent<ES::Plugin::Object::Component::Mesh>(core, CreateBoxMesh(
        glm::vec3(halfVehicleWidth, halfVehicleHeight, halfVehicleLength)
    ));

    return vehicleBody;
}

static ES::Engine::Entity CreateVehicleWheel(
    ES::Engine::Core &core,
    const glm::vec3 &position,
    float radius,
    float width
)
{
    // Rotate the wheel 90 degrees around the Z axis so it's oriented correctly
    glm::quat rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(1, 0, 0));
    ES::Engine::Entity wheel = core.CreateEntity();
    wheel.AddComponent<ES::Plugin::Object::Component::Transform>(core, position, glm::vec3(1.0f), rotation);
    wheel.AddComponent<ES::Plugin::Object::Component::Mesh>(core, CreateCylinderMesh(
        glm::vec3(radius, width, radius),
        16,
        glm::vec3(1.0f, 0.0f, 0.0f)
    ));
    wheel.AddComponent<ES::Plugin::OpenGL::Component::ShaderHandle>(core, "noTextureLightShadow");
    wheel.AddComponent<ES::Plugin::OpenGL::Component::MaterialHandle>(core, "car_wheel");
    wheel.AddComponent<ES::Plugin::OpenGL::Component::ModelHandle>(core, "car_wheel");

    return wheel;
}

static glm::vec3 GetMeshBoundingBoxSize(
    const ES::Plugin::Object::Component::Mesh &mesh
) {
    if (mesh.vertices.empty()) {
        return glm::vec3(0.0f);
    }

    glm::vec3 minPoint = mesh.vertices[0];
    glm::vec3 maxPoint = mesh.vertices[0];

    for (const auto &vertex : mesh.vertices) {
        minPoint = glm::min(minPoint, vertex);
        maxPoint = glm::max(maxPoint, vertex);
    }

    return maxPoint - minPoint;
}

static void RotateMesh(
    ES::Plugin::Object::Component::Mesh &mesh,
    const glm::vec3 &rotationAxis,
    float angle
) {
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, rotationAxis);
    for (auto &vertex : mesh.vertices) {
        vertex = glm::vec3(rotationMatrix * glm::vec4(vertex, 1.0f));
    }
}

float RPMToNorm(float rpm, float minRPM, float maxRPM)
{
    if (rpm < minRPM) return 0.0f;
    if (rpm > maxRPM) return 1.0f;
    return (rpm - minRPM) / (maxRPM - minRPM);
}

ES::Engine::Entity CreateVehicle(ES::Engine::Core &core)
{
    const std::string modelPath = "asset/Porsche_911_GT3_992_reduced.obj";
    ES::Plugin::Object::Component::Mesh vehicleBodyMesh;

    if (!ES::Plugin::Object::Resource::OBJLoader::loadModel(
        modelPath,
        vehicleBodyMesh.vertices,
        vehicleBodyMesh.normals,
        vehicleBodyMesh.texCoords,
        vehicleBodyMesh.indices
    )) {
        throw std::runtime_error("Failed to load vehicle model from " + modelPath);
    }

    // Model exported from Blender is wrongly oriented, so we need to rotate it
    RotateMesh(vehicleBodyMesh, glm::vec3(0.0f, 1.0f, 0.0f), glm::radians(90.0f));

    glm::vec3 boundingBoxSize = GetMeshBoundingBoxSize(vehicleBodyMesh);

    printf("Vehicle body bounding box size: %.2f x %.2f x %.2f\n",
           boundingBoxSize.x, boundingBoxSize.y, boundingBoxSize.z);

    // consts
    glm::vec3 bodyPosition(0.0f, 30.0f, 0.0f);

    float wheelRadius = 0.689 / 2.0f;
    float wheelWidth = 0.285f;
    float halfVehicleLength = boundingBoxSize.z / 2.0f;
    float halfVehicleWidth = boundingBoxSize.x / 2.0f;
    float halfVehicleHeight = boundingBoxSize.y / 2.0f;
    float suspensionMinLength = 0.3f;
    float suspensionMaxLength = 0.5f;
    float maxSteerAngle = 0.52f; // in radians, ~30 degrees
    bool fourWheelDrive = false; // GT3 RS is RWD
    float frontBackLimitedSlipRatio = 1.4f;
    float leftRightLimitedSlipRatio = 1.4f;
    bool antiRollBar = true;

    float vehicleMass = 1430.0f; // Actual GT3 RS weight
    float maxEngineTorque = 520.0f; // GT3 RS naturally aspirated engine

    ES::Engine::Entity vehicleEntity;

    {
        // In a separate scope to make sure the builder releases the memory properly
        auto vehicleBuilder = ES::Plugin::Physics::Utils::WheeledVehicleBuilder(core);

        // TODO: fix in ESQ, initial position should take into account the vehicle body mesh AND tires
        // right now it just sets the position of the vehicle body
        vehicleBuilder.SetInitialPosition(bodyPosition);
        vehicleBuilder.SetMass(vehicleMass);
        vehicleBuilder.SetBodyMesh(
            vehicleBodyMesh
        );
        vehicleBuilder.SetWheelMesh(
            CreateCylinderMesh(glm::vec3(wheelRadius, wheelWidth, wheelRadius), 16, glm::vec3(1.0f, 0.0f, 0.0f))
        );
        vehicleBuilder.SetWheelCallbackFn([](ES::Engine::Core &c, ES::Engine::Entity &entity) {
            entity.AddComponent<ES::Plugin::OpenGL::Component::ShaderHandle>(c, "noTextureLightShadow");
            entity.AddComponent<ES::Plugin::OpenGL::Component::MaterialHandle>(c, "car_wheel");
            entity.AddComponent<ES::Plugin::OpenGL::Component::ModelHandle>(c, "car_wheel");
        });
        vehicleBuilder.SetVehicleCallbackFn([](ES::Engine::Core &c, ES::Engine::Entity &entity) {
            entity.AddComponent<ES::Plugin::OpenGL::Component::ShaderHandle>(c, "noTextureLightShadow");
            entity.AddComponent<ES::Plugin::OpenGL::Component::MaterialHandle>(c, "car_body");
            entity.AddComponent<ES::Plugin::OpenGL::Component::ModelHandle>(c, "car_body");
        });
        vehicleBuilder.SetOffsetCenterOfMass(glm::vec3(0.0f, -halfVehicleHeight, 0.0f));
        vehicleBuilder.SetWheelOffset(0, glm::vec3(0.92, 0.667, 1.24));
        vehicleBuilder.SetWheelOffset(1, glm::vec3(-0.92, 0.667, 1.24));
        vehicleBuilder.SetWheelOffset(2, glm::vec3(0.92, 0.667, -1.21));
        vehicleBuilder.SetWheelOffset(3, glm::vec3(-0.92, 0.667, -1.21));
        vehicleBuilder.SetVehicleControllerSettingsFn([&](JPH::WheeledVehicleControllerSettings &settings) {
            settings.mEngine.mMaxRPM = 9000.0f;
            settings.mEngine.mMinRPM = 800.0f;
            settings.mEngine.mMaxTorque = maxEngineTorque;

            // GT3 RS naturally aspirated engine - more responsive torque curve
            settings.mEngine.mNormalizedTorque.Clear();
            settings.mEngine.mNormalizedTorque.AddPoint(0.0f, 0.3f);
            settings.mEngine.mNormalizedTorque.AddPoint(RPMToNorm(1000.0f, 800.0f, 9000.0f), 0.7f);  // Idle
            settings.mEngine.mNormalizedTorque.AddPoint(RPMToNorm(2000.0f, 800.0f, 9000.0f), 0.8f);   // Early response
            settings.mEngine.mNormalizedTorque.AddPoint(RPMToNorm(5000.0f, 800.0f, 9000.0f), 1.0f);  // Building up
            settings.mEngine.mNormalizedTorque.AddPoint(RPMToNorm(7000.0f, 800.0f, 9000.0f), 1.0f);   // Peak torque
            settings.mEngine.mNormalizedTorque.AddPoint(RPMToNorm(8000.0f, 800.0f, 9000.0f), 0.92f);  // High revs
            settings.mEngine.mNormalizedTorque.AddPoint(RPMToNorm(8500.0f, 800.0f, 9000.0f), 0.88f);  // High revs
            settings.mEngine.mNormalizedTorque.AddPoint(1.0f, 0.1f);
            // Drop RPM almost entirely at redline, otherwise Jolt considers we are slipping and doesn't want to shift

            settings.mEngine.mInertia = 0.28f; // Lower inertia for more responsive engine
            settings.mEngine.mAngularDamping = 0.05f;

            settings.mTransmission.mSwitchLatency = 0.05f;
            settings.mTransmission.mSwitchTime = 0.2f;
            settings.mTransmission.mClutchReleaseTime = 0.03f; // Faster clutch for sportier feel
            settings.mTransmission.mClutchStrength = 30.0f;

            // GT3 RS 7-speed PDK gearing
            settings.mTransmission.mGearRatios.clear();
            settings.mTransmission.mGearRatios.push_back(3.91f);  // 1st
            settings.mTransmission.mGearRatios.push_back(2.29f);  // 2nd
            settings.mTransmission.mGearRatios.push_back(1.58f);  // 3rd
            settings.mTransmission.mGearRatios.push_back(1.18f);  // 4th
            settings.mTransmission.mGearRatios.push_back(0.94f);  // 5th
            settings.mTransmission.mGearRatios.push_back(0.79f);  // 6th
            settings.mTransmission.mGearRatios.push_back(0.62f);  // 7th

            settings.mTransmission.mReverseGearRatios.clear();
            settings.mTransmission.mReverseGearRatios.push_back(-3.55f); // Reverse

            settings.mTransmission.mShiftUpRPM = 7500.0f;
            settings.mTransmission.mShiftDownRPM = 2000.0f;

            settings.mDifferentialLimitedSlipRatio = 1.3f; // More reasonable LSD
            
        });
        vehicleBuilder.EditWheel(0, [&](JPH::WheelSettingsWV &wheel) {
            wheel.mRadius = wheelRadius;
            wheel.mWidth = wheelWidth;
            wheel.mSuspensionMinLength = suspensionMinLength;
            wheel.mSuspensionMaxLength = suspensionMaxLength;
            wheel.mMaxSteerAngle = maxSteerAngle;
            wheel.mMaxBrakeTorque = 5000.0f;
            wheel.mMaxHandBrakeTorque = 0.0f; // Front wheels doesn't have handbrake

            // GT3 RS Michelin Cup 2 tire characteristics - front
            wheel.mLongitudinalFriction.Clear();
            wheel.mLongitudinalFriction.AddPoint(0.0f, 1.1f);   // Good initial grip
            wheel.mLongitudinalFriction.AddPoint(0.05f, 1.3f);  // Peak grip at low slip
            wheel.mLongitudinalFriction.AddPoint(0.15f, 1.2f);  // Maintain grip
            wheel.mLongitudinalFriction.AddPoint(0.4f, 0.9f);   // Gradual falloff
            wheel.mLongitudinalFriction.AddPoint(1.0f, 0.85f);   // Gradual falloff
            wheel.mLongitudinalFriction.AddPoint(100.0f, 0.85f);   // Safety to prevent issues at high speed

            wheel.mLateralFriction.Clear();
            wheel.mLateralFriction.AddPoint(0.0f, 0.6f);
            wheel.mLateralFriction.AddPoint(3.0f, 1.3f);
            wheel.mLateralFriction.AddPoint(20.0f, 1.0f);
            wheel.mLateralFriction.AddPoint(2000.0f, 1.0f);// Safety to prevent issues at high speed

        });
        vehicleBuilder.EditWheel(1, [&](JPH::WheelSettingsWV &wheel) {
            wheel.mRadius = wheelRadius;
            wheel.mWidth = wheelWidth;
            wheel.mSuspensionMinLength = suspensionMinLength;
            wheel.mSuspensionMaxLength = suspensionMaxLength;
            wheel.mMaxSteerAngle = maxSteerAngle;
            wheel.mMaxBrakeTorque = 5000.0f;
            wheel.mMaxHandBrakeTorque = 0.0f; // Front wheels doesn't have handbrake

            // GT3 RS Michelin Cup 2 tire characteristics - front
            wheel.mLongitudinalFriction.Clear();
            wheel.mLongitudinalFriction.AddPoint(0.0f, 1.1f);   // Good initial grip
            wheel.mLongitudinalFriction.AddPoint(0.05f, 1.3f);  // Peak grip at low slip
            wheel.mLongitudinalFriction.AddPoint(0.15f, 1.2f);  // Maintain grip
            wheel.mLongitudinalFriction.AddPoint(0.4f, 0.9f);   // Gradual falloff
            wheel.mLongitudinalFriction.AddPoint(1.0f, 0.85f);   // Gradual falloff
            wheel.mLongitudinalFriction.AddPoint(100.0f, 0.85f);   // Safety to prevent issues at high speed

            wheel.mLateralFriction.Clear();
            wheel.mLateralFriction.AddPoint(0.0f, 0.6f);
            wheel.mLateralFriction.AddPoint(3.0f, 1.3f);
            wheel.mLateralFriction.AddPoint(20.0f, 1.0f);
            wheel.mLateralFriction.AddPoint(2000.0f, 1.0f);// Safety to prevent issues at high speed

        });
        vehicleBuilder.EditWheel(2, [&](JPH::WheelSettingsWV &wheel) {
            wheel.mRadius = wheelRadius;
            wheel.mWidth = wheelWidth;
            wheel.mSuspensionMinLength = suspensionMinLength;
            wheel.mSuspensionMaxLength = suspensionMaxLength;
            wheel.mMaxBrakeTorque = 3200.0f;
            wheel.mMaxHandBrakeTorque = 5000.0f;
            wheel.mMaxSteerAngle = 0.0f; // Rear wheels doesn't have steering

            // GT3 RS Michelin Cup 2 tire characteristics - rear (slightly more grip for RWD)
            wheel.mLongitudinalFriction.Clear();
            wheel.mLongitudinalFriction.AddPoint(0.0f, 1.15f);  // Slightly better initial grip
            wheel.mLongitudinalFriction.AddPoint(0.05f, 1.4f);  // Higher peak for power delivery
            wheel.mLongitudinalFriction.AddPoint(0.15f, 1.25f); // Good mid-range grip
            wheel.mLongitudinalFriction.AddPoint(0.4f, 0.95f);  // Controlled falloff
            wheel.mLongitudinalFriction.AddPoint(1.0f, 0.92f);
            wheel.mLongitudinalFriction.AddPoint(100.0f, 0.85f);   // Safety to prevent issues at high speed
        
            wheel.mLateralFriction.Clear();
            wheel.mLateralFriction.AddPoint(0.0f, 0.6f);
            wheel.mLateralFriction.AddPoint(3.0f, 1.3f);
            wheel.mLateralFriction.AddPoint(20.0f, 1.0f);
            wheel.mLateralFriction.AddPoint(2000.0f, 1.0f);// Safety to prevent issues at high speed
        });
        vehicleBuilder.EditWheel(3, [&](JPH::WheelSettingsWV &wheel) {
            wheel.mRadius = wheelRadius;
            wheel.mWidth = wheelWidth;
            wheel.mSuspensionMinLength = suspensionMinLength;
            wheel.mSuspensionMaxLength = suspensionMaxLength;
            wheel.mMaxBrakeTorque = 3200.0f;
            wheel.mMaxHandBrakeTorque = 5000.0f;
            wheel.mMaxSteerAngle = 0.0f; // Rear wheels doesn't have steering

            // GT3 RS Michelin Cup 2 tire characteristics - rear (slightly more grip for RWD)
            wheel.mLongitudinalFriction.Clear();
            wheel.mLongitudinalFriction.AddPoint(0.0f, 1.15f);  // Slightly better initial grip
            wheel.mLongitudinalFriction.AddPoint(0.05f, 1.4f);  // Higher peak for power delivery
            wheel.mLongitudinalFriction.AddPoint(0.15f, 1.25f); // Good mid-range grip
            wheel.mLongitudinalFriction.AddPoint(0.4f, 0.95f);  // Controlled falloff
            wheel.mLongitudinalFriction.AddPoint(1.0f, 0.92f);
            wheel.mLongitudinalFriction.AddPoint(100.0f, 0.85f);   // Safety to prevent issues at high speed

            wheel.mLateralFriction.Clear();
            wheel.mLateralFriction.AddPoint(0.0f, 0.6f);
            wheel.mLateralFriction.AddPoint(3.0f, 1.3f);
            wheel.mLateralFriction.AddPoint(20.0f, 1.0f);

            wheel.mLateralFriction.AddPoint(2000.0f, 1.0f);// Safety to prevent issues at high speed
        });

        vehicleBuilder.CreateDifferential().EditDifferential(0, [&](JPH::VehicleDifferentialSettings &differential) {
            differential.mLeftWheel = 2; // Rear left wheel (RWD setup)
            differential.mRightWheel = 3; // Rear right wheel (RWD setup)
            differential.mLimitedSlipRatio = leftRightLimitedSlipRatio;
            differential.mEngineTorqueRatio = 1.0f; // All torque to rear (RWD)
            differential.mDifferentialRatio = 3.3f;
        });

        // Remove front differential for RWD setup
        /*
        if (fourWheelDrive) {
            vehicleBuilder.CreateDifferential().EditDifferential(1, [&](JPH::VehicleDifferentialSettings &differential) {
                differential.mLeftWheel = 2;
                differential.mRightWheel = 3;
                differential.mLimitedSlipRatio = leftRightLimitedSlipRatio;
                differential.mEngineTorqueRatio = 0.5f;
            });
        }
        */

        if (antiRollBar) {
            vehicleBuilder.CreateAntiRollBar().EditAntiRollBar(0, [&](JPH::VehicleAntiRollBar &antiRollBar) {
                antiRollBar.mLeftWheel = 0;
                antiRollBar.mRightWheel = 1;
            });
            vehicleBuilder.CreateAntiRollBar().EditAntiRollBar(1, [&](JPH::VehicleAntiRollBar &antiRollBar) {
                antiRollBar.mLeftWheel = 2;
                antiRollBar.mRightWheel = 3;
            });
        }

        

        vehicleEntity = vehicleBuilder.Build();
    }

    // This system is a class, which is why it is added here instead of being integrated into ESQ
    auto movementSystem = WheeledVehicleKeyboardMovement(vehicleEntity);
    auto controllerSystem = WheeledVehicleControllerMovement(vehicleEntity);
    auto cameraSystem = WheeledVehicleCameraSync(vehicleEntity);

    movementSystemId = std::get<0>(core.RegisterSystem<ES::Engine::Scheduler::FixedTimeUpdate>(movementSystem));
    controllerSystemId = std::get<0>(core.RegisterSystem<ES::Engine::Scheduler::FixedTimeUpdate>(controllerSystem));
    cameraSystemId = std::get<0>(core.RegisterSystem<ES::Engine::Scheduler::FixedTimeUpdate>(cameraSystem));

    return vehicleEntity;
}
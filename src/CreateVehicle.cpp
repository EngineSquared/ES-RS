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

static ES::Engine::Entity CreateVehicleBody(
    ES::Engine::Core &core,
    const glm::vec3 &position,
    float halfVehicleLength,
    float halfVehicleWidth,
    float halfVehicleHeight
)
{
    std::shared_ptr<JPH::ShapeSettings> carShapeSettingsBox = std::make_shared<JPH::BoxShapeSettings>(
        JPH::Vec3(halfVehicleLength, halfVehicleHeight, halfVehicleWidth)
    );
    carShapeSettingsBox->SetEmbedded();
    std::shared_ptr<JPH::ShapeSettings> carShapeSettings = std::make_shared<JPH::OffsetCenterOfMassShapeSettings>(
        JPH::Vec3(0.0f, -halfVehicleHeight, 0.0f),
        carShapeSettingsBox.get()
    );
	carShapeSettings->SetEmbedded();

    ES::Engine::Entity vehicleBody = core.CreateEntity();
    vehicleBody.AddComponent<ES::Plugin::Object::Component::Transform>(core, position);
    vehicleBody.AddComponent<ES::Plugin::Physics::Component::RigidBody3D>(
        core,
        carShapeSettings,
        JPH::EMotionType::Dynamic,
        ES::Plugin::Physics::Utils::Layers::MOVING);

    vehicleBody.AddComponent<ES::Plugin::OpenGL::Component::ShaderHandle>(core, "no_light");
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
    wheel.AddComponent<ES::Plugin::OpenGL::Component::ShaderHandle>(core, "no_light");
    wheel.AddComponent<ES::Plugin::OpenGL::Component::MaterialHandle>(core, "car_wheel");
    wheel.AddComponent<ES::Plugin::OpenGL::Component::ModelHandle>(core, "car_wheel");

    return wheel;
}

ES::Engine::Entity CreateVehicle(ES::Engine::Core &core)
{
    // consts
    glm::vec3 bodyPosition(0.0f, 6.0f, 0.0f);

    float wheelRadius = 0.3f;
    float wheelWidth = 0.1f;
    float halfVehicleLength = 2.0f;
    float halfVehicleWidth = 0.9f;
    float halfVehicleHeight = 0.2f;
    float wheelOffsetHorizontal = 1.4f;
    float wheelOffsetVertical = 0.18f;
    float suspensionMinLength = 0.3f;
    float suspensionMaxLength = 0.5f;
    float maxSteerAngle = 0.52f; // in radians, ~30 degrees
    bool fourWheelDrive = true;
    float frontBackLimitedSlipRatio = 1.4f;
    float leftRightLimitedSlipRatio = 1.4f;
    bool antiRollBar = true;

    float vehicleMass = 1500.0f;
    float maxEngineTorque = 500.0f;
    float clutchStrength = 10.0f;

    ES::Engine::Entity vehicleEntity;

    {
        // In a separate scope to make sure the builder releases the memory properly
        auto vehicleBuilder = ES::Plugin::Physics::Utils::WheeledVehicleBuilder(core);

        vehicleBuilder.SetInitialPosition(bodyPosition);
        vehicleBuilder.SetBodyMesh(
            CreateBoxMesh(glm::vec3(halfVehicleWidth, halfVehicleHeight, halfVehicleLength))
        );
        vehicleBuilder.SetWheelMesh(
            CreateCylinderMesh(glm::vec3(wheelRadius, wheelWidth, wheelRadius), 16, glm::vec3(1.0f, 0.0f, 0.0f))
        );
        vehicleBuilder.SetWheelCallbackFn([](ES::Engine::Core &c, ES::Engine::Entity &entity) {
            entity.AddComponent<ES::Plugin::OpenGL::Component::ShaderHandle>(c, "no_light");
            entity.AddComponent<ES::Plugin::OpenGL::Component::MaterialHandle>(c, "car_wheel");
            entity.AddComponent<ES::Plugin::OpenGL::Component::ModelHandle>(c, "car_wheel");
        });
        vehicleBuilder.SetVehicleCallbackFn([](ES::Engine::Core &c, ES::Engine::Entity &entity) {
            entity.AddComponent<ES::Plugin::OpenGL::Component::ShaderHandle>(c, "no_light");
            entity.AddComponent<ES::Plugin::OpenGL::Component::MaterialHandle>(c, "car_body");
            entity.AddComponent<ES::Plugin::OpenGL::Component::ModelHandle>(c, "car_body");
        });
        vehicleBuilder.SetOffsetCenterOfMass(glm::vec3(0.0f, -halfVehicleHeight, 0.0f));
        vehicleBuilder.EditWheel(0, [&](JPH::WheelSettingsWV &wheel) {
            wheel.mPosition = JPH::Vec3(halfVehicleWidth, -wheelOffsetVertical, wheelOffsetHorizontal);
            wheel.mRadius = wheelRadius;
            wheel.mWidth = wheelWidth;
            wheel.mSuspensionMinLength = suspensionMinLength;
            wheel.mSuspensionMaxLength = suspensionMaxLength;
            wheel.mMaxSteerAngle = maxSteerAngle;
            wheel.mMaxHandBrakeTorque = 0.0f; // Front wheels doesn't have handbrake
        });
        vehicleBuilder.EditWheel(1, [&](JPH::WheelSettingsWV &wheel) {
            wheel.mPosition = JPH::Vec3(-halfVehicleWidth, -wheelOffsetVertical, wheelOffsetHorizontal);
            wheel.mRadius = wheelRadius;
            wheel.mWidth = wheelWidth;
            wheel.mSuspensionMinLength = suspensionMinLength;
            wheel.mSuspensionMaxLength = suspensionMaxLength;
            wheel.mMaxSteerAngle = maxSteerAngle;
            wheel.mMaxHandBrakeTorque = 0.0f; // Front wheels doesn't have handbrake
        });
        vehicleBuilder.EditWheel(2, [&](JPH::WheelSettingsWV &wheel) {
            wheel.mPosition = JPH::Vec3(halfVehicleWidth, -wheelOffsetVertical, -wheelOffsetHorizontal);
            wheel.mRadius = wheelRadius;
            wheel.mWidth = wheelWidth;
            wheel.mSuspensionMinLength = suspensionMinLength;
            wheel.mSuspensionMaxLength = suspensionMaxLength;
            wheel.mMaxSteerAngle = 0.0f; // Rear wheels doesn't have steering
        });
        vehicleBuilder.EditWheel(3, [&](JPH::WheelSettingsWV &wheel) {
            wheel.mPosition = JPH::Vec3(-halfVehicleWidth, -wheelOffsetVertical, -wheelOffsetHorizontal);
            wheel.mRadius = wheelRadius;
            wheel.mWidth = wheelWidth;
            wheel.mSuspensionMinLength = suspensionMinLength;
            wheel.mSuspensionMaxLength = suspensionMaxLength;
            wheel.mMaxSteerAngle = 0.0f; // Rear wheels doesn't have steering
        });

        vehicleBuilder.CreateDifferential().EditDifferential(0, [&](JPH::VehicleDifferentialSettings &differential) {
            differential.mLeftWheel = 0;
            differential.mRightWheel = 1;
            differential.mLimitedSlipRatio = leftRightLimitedSlipRatio;
            if (fourWheelDrive) {
                differential.mEngineTorqueRatio = 0.5f;
            }
        });

        if (fourWheelDrive) {
            vehicleBuilder.CreateDifferential().EditDifferential(1, [&](JPH::VehicleDifferentialSettings &differential) {
                differential.mLeftWheel = 2;
                differential.mRightWheel = 3;
                differential.mLimitedSlipRatio = leftRightLimitedSlipRatio;
                differential.mEngineTorqueRatio = 0.5f;
            });
        }

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
    core.RegisterSystem<ES::Engine::Scheduler::FixedTimeUpdate>(movementSystem);
    auto controllerSystem = WheeledVehicleControllerMovement(vehicleEntity);
    core.RegisterSystem<ES::Engine::Scheduler::FixedTimeUpdate>(controllerSystem);
    auto cameraSystem = WheeledVehicleCameraSync(vehicleEntity);
    core.RegisterSystem<ES::Engine::Scheduler::FixedTimeUpdate>(cameraSystem);

    return vehicleEntity;
}
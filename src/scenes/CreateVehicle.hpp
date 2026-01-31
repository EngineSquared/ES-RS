#pragma once

#include "Engine.hpp"

/**
 * @brief Global toggle for SoftBody chassis
 *
 * When true, vehicles will use deformable SoftBody chassis that can
 * be dented on collision. When false, uses standard RigidBody chassis.
 *
 * @note Set to false if experiencing performance issues
 */
constexpr bool USE_SOFTBODY_CHASSIS = true;

/**
 * @brief Create a drivable vehicle
 *
 * @param core Engine core reference
 * @param useSoftBodyChassis Override for SoftBody chassis (defaults to USE_SOFTBODY_CHASSIS)
 * @return The created vehicle entity
 */
Engine::Entity CreateVehicle(Engine::Core &core, bool useSoftBodyChassis = USE_SOFTBODY_CHASSIS);

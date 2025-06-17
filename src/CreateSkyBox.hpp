#pragma once

#include "Engine.hpp"
#include "Object.hpp"

#include <array>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <string_view>

/**
 * @brief Creates a skybox entity from a cross-layout texture with automatic resource management
 *
 * Generates a complete skybox entity with mesh, texture, and proper resource management from a cross-format
 * image (cubemap unfolded in cross shape layout). Uses RAII principles for resource safety.
 *
 * @param core Reference to the engine core
 * @param path Path to the cross-layout texture image (supports common formats: PNG, JPG, TGA, HDR)
 * @param position 3D position of the skybox in world space (default: origin)
 * @param rotation Quaternion rotation of the skybox (default: identity)
 * @param size Skybox dimensions (default: unit cube)
 * @return ES::Engine::Entity Configured skybox entity with all components
 *
 * @throws std::runtime_error If the texture file doesn't exist or cannot be loaded
 * @throws std::invalid_argument If size contains non-positive values
 * @note Skybox is a visual-only entity optimized for performance (no physics collision)
 * @note Supported texture format: cross layout (6 faces arranged in cross pattern)
 * @note Texture should have aspect ratio 4:3 or 3:4 for proper cross layout
 * @note Automatically generates unique resource ID based on filename
 * @note Uses modern resource management with RAII principles
 *
 * @code
 * // Basic usage with default parameters
 * auto skybox = CreateSkyBox(core, "textures/skybox_cross.png");
 *
 * // Custom positioning and sizing
 * auto skybox = CreateSkyBox(core, "textures/night_sky.hdr",
 *                           glm::vec3(0.0f, 0.0f, 0.0f),
 *                           glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
 *                           glm::vec3(100.0f, 100.0f, 100.0f));
 *
 * // HDR skybox for advanced lighting
 * auto hdr_skybox = CreateSkyBox(core, "textures/sunset.hdr",
 *                               glm::vec3(0.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
 *                               glm::vec3(200.0f));
 * @endcode
 */
ES::Engine::Entity CreateSkyBox(ES::Engine::Core &core, std::string_view texture_path,
                                const glm::vec3 &world_position = glm::vec3{0.0f},
                                const glm::quat &world_rotation = glm::quat{1.0f, 0.0f, 0.0f, 0.0f},
                                const glm::vec3 &skybox_dimensions = glm::vec3{1.0f});

/**
 * @brief Creates a skybox entity from 6 separate texture files with validation
 *
 * Generates a skybox from 6 distinct images for each cube face with comprehensive validation.
 * Texture order follows OpenGL standard cubemap convention with proper error handling.
 *
 * @param core Reference to the engine core
 * @param paths Array of exactly 6 texture file paths for cube faces
 *              Order: [Right(+X), Left(-X), Top(+Y), Bottom(-Y), Front(+Z), Back(-Z)]
 * @param position 3D position of the skybox in world space (default: origin)
 * @param rotation Quaternion rotation of the skybox (default: identity)
 * @param size Skybox dimensions (default: unit cube)
 * @return ES::Engine::Entity Configured skybox entity with cubemap texture
 *
 * @throws std::runtime_error If any texture file doesn't exist or cannot be loaded
 * @throws std::invalid_argument If size contains non-positive values
 * @note Skybox is a visual-only entity optimized for performance (no physics collision)
 * @note Face order: Right(+X), Left(-X), Top(+Y), Bottom(-Y), Front(+Z), Back(-Z)
 * @note All textures should have identical resolution for optimal rendering quality
 * @note Recommended texture format: square power-of-2 dimensions (512x512, 1024x1024, 2048x2048)
 * @note Supports HDR textures for advanced lighting techniques
 * @note Resource ID generated from first texture filename for consistency
 *
 * @code
 * // Standard cubemap with 6 separate textures
 * std::array<std::string, 6> skybox_faces = {
 *     "textures/right.jpg",   // Right (+X)
 *     "textures/left.jpg",    // Left (-X)
 *     "textures/top.jpg",     // Top (+Y)
 *     "textures/bottom.jpg",  // Bottom (-Y)
 *     "textures/front.jpg",   // Front (+Z)
 *     "textures/back.jpg"     // Back (-Z)
 * };
 * auto skybox = CreateSkyBox(core, skybox_faces);
 *
 * // Large outdoor environment skybox
 * std::array<std::string, 6> mountain_skybox = {
 *     "textures/mountain_rt.png", "textures/mountain_lf.png",
 *     "textures/mountain_up.png", "textures/mountain_dn.png",
 *     "textures/mountain_ft.png", "textures/mountain_bk.png"
 * };
 * auto outdoor_skybox = CreateSkyBox(core, mountain_skybox,
 *                                   glm::vec3(0.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
 *                                   glm::vec3(500.0f));
 * @endcode
 */
ES::Engine::Entity CreateSkyBox(ES::Engine::Core &core, const std::array<std::string, 6> &texture_paths,
                                const glm::vec3 &world_position = glm::vec3{0.0f},
                                const glm::quat &world_rotation = glm::quat{1.0f, 0.0f, 0.0f, 0.0f},
                                const glm::vec3 &skybox_dimensions = glm::vec3{1.0f});

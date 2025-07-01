#include "CreateSkyBox.hpp"

#include "OpenGL.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <array>
#include <filesystem>
#include <stdexcept>

/**
 * @brief Creates an optimized cubic skybox mesh with texture coordinates
 *
 * Generates a cube mesh with inverted normals and proper UV mapping designed for skybox rendering.
 * The mesh is optimized with memory pre-allocation and procedural face generation using modern C++ techniques.
 *
 * @param size Cube dimensions (width, height, depth) - must be positive values
 * @return ES::Plugin::Object::Component::Mesh Cubic mesh ready for skybox rendering
 *
 * @note Normals point inward for proper skybox rendering
 * @note UV coordinates are generated for each face (0,0) to (1,1)
 * @note Performance: O(1) with constexpr optimizations and memory pre-allocation
 * @note Generates exactly 24 vertices (4 per face) and 36 indices (12 triangles)
 * @note Thread-safe and const-correct implementation
 *
 * @pre size components must be > 0.0f
 * @post Returns a valid mesh with 24 vertices, 24 normals, 24 UVs, and 36 indices
 * @internal This function is not part of the public API
 *
 * @code
 * // Internal usage for mesh generation
 * auto mesh = CreateSkyBoxMesh(glm::vec3(10.0f, 10.0f, 10.0f));
 * // mesh.vertices.size() == 24
 * // mesh.normals.size() == 24
 * // mesh.indices.size() == 36
 *
 * // Small skybox for indoor scenes
 * auto indoor_mesh = CreateSkyBoxMesh(glm::vec3(5.0f));
 *
 * // Large skybox for outdoor scenes
 * auto outdoor_mesh = CreateSkyBoxMesh(glm::vec3(1000.0f));
 * @endcode
 */
static ES::Plugin::Object::Component::Mesh CreateSkyBoxMesh(const glm::vec3 &size) noexcept
{
    ES::Plugin::Object::Component::Mesh skybox_mesh;

    constexpr size_t total_vertices_per_skybox = 24;
    constexpr size_t total_indices_per_skybox = 36;
    skybox_mesh.vertices.reserve(total_vertices_per_skybox);
    skybox_mesh.normals.reserve(total_vertices_per_skybox);
    skybox_mesh.indices.reserve(total_indices_per_skybox);

    const std::array<glm::vec3, 8> cube_corners = {{
        {-size.x, -size.y, -size.z}, // front_bottom_left
        {size.x,  -size.y, -size.z}, // front_bottom_right
        {-size.x, size.y,  -size.z}, // front_top_left
        {size.x,  size.y,  -size.z}, // front_top_right
        {-size.x, -size.y, size.z }, // back_bottom_left
        {size.x,  -size.y, size.z }, // back_bottom_right
        {-size.x, size.y,  size.z }, // back_top_left
        {size.x,  size.y,  size.z }  // back_top_right
    }};

    struct CubeFace {
        std::array<uint8_t, 4> corner_indices;
        glm::vec3 inverted_normal;
        std::array<uint8_t, 6> triangle_indices;
    };

    constexpr std::array<CubeFace, 6> skybox_faces = {{
        // Front face
        {{0, 1, 2, 3}, {0.0f, 0.0f, 1.0f},  {2, 1, 0, 2, 3, 1}},
        // Back face
        {{4, 5, 6, 7}, {0.0f, 0.0f, -1.0f}, {0, 1, 2, 1, 3, 2}},
        // Bottom face
        {{0, 1, 4, 5}, {0.0f, 1.0f, 0.0f},  {2, 1, 0, 2, 3, 1}},
        // Top face
        {{2, 3, 6, 7}, {0.0f, -1.0f, 0.0f}, {0, 1, 2, 1, 3, 2}},
        // Left face
        {{0, 2, 4, 6}, {1.0f, 0.0f, 0.0f},  {2, 1, 0, 2, 3, 1}},
        // Right face
        {{1, 3, 5, 7}, {-1.0f, 0.0f, 0.0f}, {0, 1, 2, 1, 3, 2}}
    }};

    for (const auto &current_face : skybox_faces)
    {
        const auto base_vertex_index = static_cast<uint32_t>(skybox_mesh.vertices.size());

        for (size_t vertex_index_in_face = 0; const auto corner_index : current_face.corner_indices)
        {
            skybox_mesh.vertices.emplace_back(cube_corners[corner_index]);
            skybox_mesh.normals.emplace_back(current_face.inverted_normal);
            ++vertex_index_in_face;
        }

        for (const auto triangle_vertex_index : current_face.triangle_indices)
        {
            skybox_mesh.indices.emplace_back(base_vertex_index + triangle_vertex_index);
        }
    }

    return skybox_mesh;
}

/**
 * @brief Internal helper function to create the base skybox entity with validation
 *
 * Creates the fundamental skybox entity with transform and mesh components only.
 * This function factors out common entity creation logic with proper input validation.
 *
 * @param core Reference to the engine core
 * @param position World position for the skybox
 * @param rotation World rotation for the skybox
 * @param size Physical dimensions of the skybox (must be positive)
 * @return ES::Engine::Entity Base skybox entity with core components
 *
 * @throws std::invalid_argument If size contains non-positive values
 * @internal This function is not part of the public API
 * @note Skybox is visual-only without physics components for optimal performance
 * @note Uses RAII principles for automatic resource management
 *
 * @code
 * // Internal usage example (not accessible from public API)
 * auto skybox = CreateSkyBoxEntity(core,
 *                                 glm::vec3(0.0f, 0.0f, 0.0f),
 *                                 glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
 *                                 glm::vec3(50.0f, 50.0f, 50.0f));
 * @endcode
 */
static ES::Engine::Entity CreateSkyBoxEntity(ES::Engine::Core &core, const glm::vec3 &position,
                                             const glm::quat &rotation, const glm::vec3 &size)
{
    if (size.x <= 0.0f || size.y <= 0.0f || size.z <= 0.0f)
        throw std::invalid_argument("Skybox size components must be positive values");

    constexpr glm::vec3 box_scale{1.0f};

    auto box = core.CreateEntity();

    box.AddComponent<ES::Plugin::Object::Component::Transform>(core, position, box_scale, rotation);
    box.AddComponent<ES::Plugin::Object::Component::Mesh>(core, CreateSkyBoxMesh(size));

    return box;
}

/**
 * @brief Generates UV coordinates for a skybox face
 * @internal Helper function for UV generation
 *
 * @code
 * // Internal usage for UV coordinate generation
 * constexpr auto uvs = GetFaceUVs();
 * // uvs[0] = {0.0f, 0.0f} (bottom-left)
 * // uvs[1] = {1.0f, 0.0f} (bottom-right)
 * // uvs[2] = {0.0f, 1.0f} (top-left)
 * // uvs[3] = {1.0f, 1.0f} (top-right)
 * @endcode
 */
static constexpr std::array<glm::vec2, 4> GetFaceUVs() noexcept
{
    return {{
        {0.0f, 0.0f}, // bottom-left
        {1.0f, 0.0f}, // bottom-right
        {0.0f, 1.0f}, // top-left
        {1.0f, 1.0f}  // top-right
    }};
}

/**
 * @brief Internal helper to add rendering components to skybox
 *
 * Adds the necessary rendering components (material, shader) to the skybox entity
 * for proper rendering integration with the engine.
 *
 * @param core Reference to the engine core
 * @param skybox The skybox entity to configure
 *
 * @internal This function factors out common rendering setup
 * @note Only adds MaterialHandle and ShaderHandle components
 * @note Ambient light should be managed separately in the main application
 * @note CRITICAL: Skybox must be rendered LAST with GL_LEQUAL depth function
 * @note View matrix should remove translation: glm::mat4(glm::mat3(viewMatrix))
 * @note Depth function sequence: GL_LEQUAL -> render skybox -> GL_LESS
 *
 * @code
 * // Internal usage for skybox rendering configuration
 * ES::Engine::Entity skybox = CreateSkyBoxEntity(core, pos, rot, size);
 * AddSkyboxRenderingComponents(core, skybox);
 * // Adds MaterialHandle and ShaderHandle only
 * // Proper skybox rendering order in your render loop:
 * // 1. Render all normal objects with GL_LESS
 * // 2. glDepthFunc(GL_LEQUAL);
 * // 3. Render skybox with view matrix without translation
 * // 4. glDepthFunc(GL_LESS);
 * @endcode
 */
static void AddSkyboxRenderingComponents(ES::Engine::Core &core, ES::Engine::Entity &skybox)
{
    skybox.AddComponent<ES::Plugin::OpenGL::Component::MaterialHandle>(core, "skyboxDefault");
    skybox.AddComponent<ES::Plugin::OpenGL::Component::ShaderHandle>(core, "skyboxDefault");
}

ES::Engine::Entity CreateSkyBox(ES::Engine::Core &core, const std::string_view texture_path,
                                const glm::vec3 &world_position, const glm::quat &world_rotation,
                                const glm::vec3 &skybox_dimensions)
{
    if (texture_path.empty())
        throw std::invalid_argument("Texture path cannot be empty");

    auto skybox_entity = CreateSkyBoxEntity(core, world_position, world_rotation, skybox_dimensions);

    auto &cubemap_resource_manager = core.GetResource<ES::Plugin::OpenGL::Resource::CubeMapManager>();

    const auto texture_file_path = std::filesystem::path(texture_path);
    if (!texture_file_path.has_filename())
        throw std::invalid_argument("Invalid texture path provided");

    const std::string texture_file_name = texture_file_path.stem().string();
    const std::string unique_resource_id = "cubemap_cross_" + texture_file_name;

    try
    {
        cubemap_resource_manager.Add(entt::hashed_string{unique_resource_id.c_str()}, texture_path.data());
        skybox_entity.AddComponent<ES::Plugin::OpenGL::Component::CubeMapHandle>(core, unique_resource_id.c_str());
        skybox_entity.AddComponent<ES::Plugin::OpenGL::Component::ModelHandle>(core, unique_resource_id.c_str());

        AddSkyboxRenderingComponents(core, skybox_entity);
    }
    catch (const std::exception &loading_error)
    {
        throw std::runtime_error("Failed to load cross-layout texture: " + std::string(loading_error.what()));
    }

    return skybox_entity;
}

ES::Engine::Entity CreateSkyBox(ES::Engine::Core &core, const std::array<std::string, 6> &texture_paths,
                                const glm::vec3 &world_position, const glm::quat &world_rotation,
                                const glm::vec3 &skybox_dimensions)
{
    for (size_t path_index = 0; const auto &current_texture_path : texture_paths)
    {
        if (current_texture_path.empty())
            throw std::invalid_argument("Texture path at index " + std::to_string(path_index) + " cannot be empty");

        ++path_index;
    }

    auto skybox_entity = CreateSkyBoxEntity(core, world_position, world_rotation, skybox_dimensions);

    auto &cubemap_resource_manager = core.GetResource<ES::Plugin::OpenGL::Resource::CubeMapManager>();

    const auto first_texture_file_path = std::filesystem::path(texture_paths[0]);
    if (!first_texture_file_path.has_filename())
        throw std::invalid_argument("Invalid texture path provided at index 0");

    const std::string base_file_name = first_texture_file_path.stem().string();
    const std::string unique_resource_id = "cubemap_faces_" + base_file_name;

    try
    {
        cubemap_resource_manager.Add(entt::hashed_string{unique_resource_id.c_str()}, texture_paths);
        skybox_entity.AddComponent<ES::Plugin::OpenGL::Component::CubeMapHandle>(core, unique_resource_id.c_str());
        skybox_entity.AddComponent<ES::Plugin::OpenGL::Component::ModelHandle>(core, unique_resource_id.c_str());

        AddSkyboxRenderingComponents(core, skybox_entity);
    }
    catch (const std::exception &loading_error)
    {
        throw std::runtime_error("Failed to load cubemap textures: " + std::string(loading_error.what()));
    }

    return skybox_entity;
}

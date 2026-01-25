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
#include "component/SoftBody.hpp"
#include "component/SoftBodyAttachment.hpp"

#include <glm/glm.hpp>
#include "Logger.hpp"
#include "spdlog/fmt/fmt.h"
#include <unordered_map>
#include <cmath>

// Configuration: Set toDéveloppement de solution informatique true to enable deformable bodywork
static constexpr bool USE_SOFTBODY_BODYWORK = true;
// Stiffness of the bodywork (0 = very deformable, 1 = nearly rigid)
static constexpr float BODYWORK_STIFFNESS = 0.92f;
// Target vertex count for simplified SoftBody mesh
static constexpr size_t TARGET_SOFTBODY_VERTICES = 2000;

/**
 * @brief Hash function for grid cell coordinates
 */
struct GridCellHash {
    size_t operator()(const glm::ivec3 &v) const {
        return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1) ^ (std::hash<int>()(v.z) << 2);
    }
};

/**
 * @brief Simplify a mesh using vertex clustering (grid-based decimation)
 *
 * This reduces vertex count by clustering nearby vertices into grid cells
 * and replacing each cluster with a single representative vertex.
 *
 * @param mesh Input mesh to simplify
 * @param targetVertexCount Approximate target number of vertices
 * @return Simplified mesh suitable for SoftBody physics
 */
Object::Component::Mesh SimplifyMeshForPhysics(const Object::Component::Mesh &mesh, size_t targetVertexCount)
{
    const auto &vertices = mesh.GetVertices();
    const auto &indices = mesh.GetIndices();

    if (vertices.size() <= targetVertexCount)
    {
        Log::Info(fmt::format("Mesh already has {} vertices, no simplification needed", vertices.size()));
        return mesh;
    }

    // Calculate bounding box
    glm::vec3 minBounds(std::numeric_limits<float>::max());
    glm::vec3 maxBounds(std::numeric_limits<float>::lowest());

    for (const auto &v : vertices)
    {
        minBounds = glm::min(minBounds, v);
        maxBounds = glm::max(maxBounds, v);
    }

    glm::vec3 size = maxBounds - minBounds;

    // Calculate grid cell size to achieve target vertex count
    // Assuming uniform distribution: targetVertexCount ≈ gridSize^3
    float volume = size.x * size.y * size.z;
    float cellVolume = volume / static_cast<float>(targetVertexCount);
    float cellSize = std::cbrt(cellVolume);

    // Ensure minimum cell size
    cellSize = std::max(cellSize, 0.01f);

    Log::Info(fmt::format("Simplifying mesh: {} -> ~{} vertices (cell size: {:.3f})",
                          vertices.size(), targetVertexCount, cellSize));

    // Map from grid cell to accumulated vertex data
    struct CellData {
        glm::vec3 positionSum{0.0f};
        glm::vec3 normalSum{0.0f};
        glm::vec2 texCoordSum{0.0f};
        uint32_t count = 0;
        uint32_t newIndex = 0;
    };

    std::unordered_map<glm::ivec3, CellData, GridCellHash> cellMap;

    // Map old vertex index to grid cell
    std::vector<glm::ivec3> vertexToCell(vertices.size());

    const auto &normals = mesh.GetNormals();
    const auto &texCoords = mesh.GetTexCoords();

    // First pass: accumulate vertices into cells
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        glm::vec3 relPos = vertices[i] - minBounds;
        glm::ivec3 cell(
            static_cast<int>(relPos.x / cellSize),
            static_cast<int>(relPos.y / cellSize),
            static_cast<int>(relPos.z / cellSize)
        );

        vertexToCell[i] = cell;

        auto &data = cellMap[cell];
        data.positionSum += vertices[i];
        if (i < normals.size())
            data.normalSum += normals[i];
        if (i < texCoords.size())
            data.texCoordSum += texCoords[i];
        data.count++;
    }

    // Second pass: create simplified vertices
    std::vector<glm::vec3> newVertices;
    std::vector<glm::vec3> newNormals;
    std::vector<glm::vec2> newTexCoords;

    newVertices.reserve(cellMap.size());
    newNormals.reserve(cellMap.size());
    newTexCoords.reserve(cellMap.size());

    uint32_t newIdx = 0;
    for (auto &[cell, data] : cellMap)
    {
        float invCount = 1.0f / static_cast<float>(data.count);
        newVertices.push_back(data.positionSum * invCount);
        newNormals.push_back(glm::normalize(data.normalSum * invCount + glm::vec3(0.0001f)));
        newTexCoords.push_back(data.texCoordSum * invCount);
        data.newIndex = newIdx++;
    }

    // Third pass: remap indices and filter degenerate triangles
    std::vector<uint32_t> newIndices;
    newIndices.reserve(indices.size());

    for (size_t i = 0; i + 2 < indices.size(); i += 3)
    {
        uint32_t i0 = indices[i];
        uint32_t i1 = indices[i + 1];
        uint32_t i2 = indices[i + 2];

        if (i0 >= vertices.size() || i1 >= vertices.size() || i2 >= vertices.size())
            continue;

        uint32_t newI0 = cellMap[vertexToCell[i0]].newIndex;
        uint32_t newI1 = cellMap[vertexToCell[i1]].newIndex;
        uint32_t newI2 = cellMap[vertexToCell[i2]].newIndex;

        // Skip degenerate triangles (all vertices collapsed to same cell)
        if (newI0 != newI1 && newI1 != newI2 && newI0 != newI2)
        {
            newIndices.push_back(newI0);
            newIndices.push_back(newI1);
            newIndices.push_back(newI2);
        }
    }

    Log::Info(fmt::format("Mesh simplified: {} vertices, {} triangles",
                          newVertices.size(), newIndices.size() / 3));

    // Build result mesh
    Object::Component::Mesh result;
    result.ReserveVertices(newVertices.size());
    result.SetVertices(newVertices);
    result.ReserveNormals(newNormals.size());
    result.SetNormals(newNormals);
    result.ReserveTexCoords(newTexCoords.size());
    result.SetTexCoords(newTexCoords);
    result.ReserveIndices(newIndices.size());
    result.SetIndices(newIndices);

    return result;
}

/**
 * @brief Invert mesh on the X axis to go from right-handed to left-handed coordinate system
 *
 * @param mesh  Mesh to invert
 * @return Inverted mesh
 */
Object::Component::Mesh InvertMeshX(const Object::Component::Mesh &mesh)
{
    Object::Component::Mesh invertedMesh = mesh;
    auto &vertices = invertedMesh.GetVertices();
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        invertedMesh.SetVertexAt(i, glm::vec3(-vertices[i].x, vertices[i].y, vertices[i].z));
    }
    return invertedMesh;
}

/**
 * @brief Combine multiple meshes into a single mesh
 *
 * @param meshes Vector of meshes to combine
 * @return Combined mesh
 */
Object::Component::Mesh CombineMeshes(const std::vector<Object::Component::Mesh> &meshes)
{
    Object::Component::Mesh combined;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;
    std::vector<uint32_t> indices;

    uint32_t indexOffset = 0;

    for (const auto &mesh : meshes)
    {
        const auto &v = mesh.GetVertices();
        const auto &n = mesh.GetNormals();
        const auto &t = mesh.GetTexCoords();
        const auto &i = mesh.GetIndices();

        // Append vertices, normals, texCoords
        vertices.insert(vertices.end(), v.begin(), v.end());
        normals.insert(normals.end(), n.begin(), n.end());
        texCoords.insert(texCoords.end(), t.begin(), t.end());

        // Append indices with offset
        for (uint32_t idx : i)
        {
            indices.push_back(idx + indexOffset);
        }

        indexOffset += static_cast<uint32_t>(v.size());
    }

    combined.ReserveVertices(vertices.size());
        combined.SetVertices(vertices);
    combined.ReserveNormals(normals.size());
        combined.SetNormals(normals);
    combined.ReserveTexCoords(texCoords.size());
        combined.SetTexCoords(texCoords);
    combined.ReserveIndices(indices.size());
        combined.SetIndices(indices);

    return combined;
}

/**
 * @brief Find vertices at the bottom of a mesh (for anchoring to chassis)
 *
 * @param mesh The mesh to analyze
 * @param bottomThreshold Y coordinate threshold (vertices below this are anchors)
 * @return Vector of vertex indices at the bottom
 */
std::vector<uint32_t> FindBottomVertices(const Object::Component::Mesh &mesh, float bottomThreshold)
{
    std::vector<uint32_t> anchors;
    const auto &vertices = mesh.GetVertices();

    for (size_t i = 0; i < vertices.size(); ++i)
    {
        if (vertices[i].y <= bottomThreshold)
        {
            anchors.push_back(static_cast<uint32_t>(i));
        }
    }

    return anchors;
}

/**
 * @brief Invert mesh UVs (U and/or V) in-place.
 *
 * @param mesh Mesh to modify
 * @param invertU If true, set u -> 1 - u
 * @param invertV If true, set v -> 1 - v
 */
void InvertMeshUVs(Object::Component::Mesh &mesh, bool invertU = true, bool invertV = false)
{
    const auto &texCoords = mesh.GetTexCoords();
    for (size_t i = 0; i < texCoords.size(); ++i)
    {
        glm::vec2 uv = texCoords[i];
        if (invertU)
            uv.x = 1.0f - uv.x;
        if (invertV)
            uv.y = 1.0f - uv.y;
        mesh.SetTexCoordAt(i, uv);
    }
}

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
    floorPhysics.AddComponent<Object::Component::Transform>(glm::vec3(0.0f, 0.0f, 0.0f));
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
                                                           .position = glm::vec3(posX, 0.0f, posZ),
                                                           .rotation = glm::vec3(0.0f, 0.0f, 0.0f)});

            Object::Component::Material tileMaterial;
            tileMaterial.diffuse = color;
            tileMaterial.ambient = color * 0.3f;
            tileMaterial.specular = glm::vec3(0.1f);
            tileMaterial.shininess = 16.0f;
            // Use engine default texture (1x1) so material color is used without external textures
            // tileMaterial.diffuseTexName = Graphic::Utils::DEFAULT_TEXTURE_NAME;
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

    Physics::Builder::VehicleBuilder<4> builder;
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
           .SetWheelPositions(frontLeftWheelPos, frontRightWheelPos, rearLeftWheelPos, rearRightWheelPos)
           .SetChassisMass(chassisMass)
           .SetEngine(engineSettings);

    //=========================================================================
    // SoftBody Bodywork Configuration (Optional)
    //=========================================================================
    if constexpr (USE_SOFTBODY_BODYWORK)
    {
        if (!otherShapes.empty())
        {
            // Combine all bodywork shapes into a single mesh for SoftBody
            std::vector<Object::Component::Mesh> bodyworkMeshes;
            for (const auto &shape : otherShapes)
            {
                bodyworkMeshes.push_back(shape.GetMesh());
            }

            Object::Component::Mesh combinedBodywork = CombineMeshes(bodyworkMeshes);

            size_t vertexCount = combinedBodywork.GetVertices().size();
            Log::Info(fmt::format("Original bodywork mesh: {} vertices", vertexCount));

            // Automatically simplify the mesh if too complex for SoftBody physics
            if (vertexCount > TARGET_SOFTBODY_VERTICES)
            {
                Log::Info(fmt::format(
                    "Simplifying mesh from {} vertices to ~{} for SoftBody physics...",
                    vertexCount, TARGET_SOFTBODY_VERTICES));

                combinedBodywork = SimplifyMeshForPhysics(combinedBodywork, TARGET_SOFTBODY_VERTICES);
            }

            // Find anchor vertices at the bottom of the bodywork (AFTER simplification)
            // These connect to the chassis and don't deform
            // Threshold: vertices with Y < -0.4 (near chassis mount points)
            std::vector<uint32_t> anchorVertices = FindBottomVertices(combinedBodywork, -0.4f);

            Log::Info(fmt::format("SoftBody bodywork: {} vertices, {} anchors",
                                  combinedBodywork.GetVertices().size(), anchorVertices.size()));

            builder.EnableSoftBodyBodywork(true)
                   .SetBodyworkMesh(combinedBodywork)
                   .SetBodyworkAnchors(anchorVertices)
                   .SetBodyworkStiffness(BODYWORK_STIFFNESS);
        }
    }

    auto vehicleEntity = builder.Build(core);

    vehicleEntity.AddComponent<Object::Component::Material>(chassisMaterial);
    vehicleEntity.AddComponent<PlayerVehicle>();

    vehicleEntity.AddComponent<Relationship::Component::Relationship>();

    //=========================================================================
    // Create child entities for visual shapes (always needed for rendering)
    // In SoftBody mode: these are visual only, SoftBody handles deformation physics
    // In RigidBody mode: these follow the parent rigidly
    //=========================================================================
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

    Log::Info(fmt::format("Vehicle created with {} child shapes (SoftBody: {})",
                          otherShapes.size(), USE_SOFTBODY_BODYWORK ? "enabled" : "disabled"));

    return vehicleEntity;
}
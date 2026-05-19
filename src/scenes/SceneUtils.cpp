#include "scenes/SceneUtils.hpp"

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
    const auto &normals = invertedMesh.GetNormals();
    for (size_t i = 0; i < normals.size(); ++i)
    {
        invertedMesh.SetNormalAt(i, glm::vec3(-normals[i].x, normals[i].y, normals[i].z));
    }
    auto indices = invertedMesh.GetIndices();
    for (size_t i = 0; i + 2 < indices.size(); i += 3)
    {
        std::swap(indices[i + 1], indices[i + 2]);
    }
    invertedMesh.SetIndices(indices);
    return invertedMesh;
}

/**
 * @brief Invert mesh UVs (U and/or V) in-place.
 *
 * @param mesh Mesh to modify
 * @param invertU If true, set u -> 1 - u
 * @param invertV If true, set v -> 1 - v
 */
void InvertMeshUVs(Object::Component::Mesh &mesh, bool invertU, bool invertV)
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
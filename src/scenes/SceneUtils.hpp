#pragma once

#include "Object.hpp"
#include <glm/glm.hpp>

/**
 * @brief Invert mesh on the X axis to go from right-handed to left-handed coordinate system
 *
 * @param mesh  Mesh to invert
 * @return Inverted mesh
 */
Object::Component::Mesh InvertMeshX(const Object::Component::Mesh &mesh);

/**
 * @brief Invert mesh UVs (U and/or V) in-place.
 *
 * @param mesh Mesh to modify
 * @param invertU If true, set u -> 1 - u
 * @param invertV If true, set v -> 1 - v
 */
void InvertMeshUVs(Object::Component::Mesh &mesh, bool invertU = true, bool invertV = false);

/**
 * @brief Adjust a mesh position by a given offset
 *
 * @param mesh  Mesh to adjust
 * @param offset Offset to apply
 */
void AdjustMeshPosition(Object::Component::Mesh &mesh, const glm::vec3 &offset);
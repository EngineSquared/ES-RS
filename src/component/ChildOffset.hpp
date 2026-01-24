#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

/**
 * @brief Component storing the local offset from a parent entity.
 *
 * Used in conjunction with the Relationship plugin to compute world-space
 * transforms for child entities that should follow their parent.
 */
struct ChildOffset {
    /// Position offset in parent's local space
    glm::vec3 positionOffset = glm::vec3(0.0f);

    /// Rotation offset relative to parent's rotation
    glm::quat rotationOffset = glm::quat(1, 0, 0, 0);
};

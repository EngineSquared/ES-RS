#pragma once

#include "core/Core.hpp"

/**
 * @brief System that updates child entity transforms to follow their parent.
 *
 * For entities with ChildOffset, Transform, and Relationship components,
 * this system computes the world-space transform based on the parent's
 * transform and the child's local offset.
 */
void ChildFollowParentSystem(Engine::Core &core);

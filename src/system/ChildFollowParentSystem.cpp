#include "ChildFollowParentSystem.hpp"

#include "component/ChildOffset.hpp"
#include "component/Transform.hpp"
#include "Relationship.hpp"

void ChildFollowParentSystem(Engine::Core &core)
{
    auto &registry = core.GetRegistry();

    // Iterate over all entities that have ChildOffset, Transform, and Relationship
    auto view = registry.view<ChildOffset, Object::Component::Transform, Relationship::Component::Relationship>();

    for (auto entity : view)
    {
        auto &childOffset = view.get<ChildOffset>(entity);
        auto &childTransform = view.get<Object::Component::Transform>(entity);
        auto &relationship = view.get<Relationship::Component::Relationship>(entity);

        // Skip if no parent
        if (!relationship.parent.has_value())
        {
            continue;
        }

        auto parentEntity = relationship.parent.value();

        // Get parent's transform
        if (!parentEntity.HasComponents<Object::Component::Transform>())
        {
            continue;
        }

        auto &parentTransform = parentEntity.GetComponents<Object::Component::Transform>();

        // Compute child's world position: parentPos + parentRot * childOffset.positionOffset
        glm::vec3 worldPosition = parentTransform.GetPosition() +
                                  parentTransform.GetRotation() * childOffset.positionOffset;

        // Compute child's world rotation: parentRot * childOffset.rotationOffset
        glm::quat worldRotation = parentTransform.GetRotation() * childOffset.rotationOffset;

        // Update child's transform
        childTransform.SetPosition(worldPosition);
        childTransform.SetRotation(worldRotation);
    }
}

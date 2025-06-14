#include "CreateCylinder.hpp"

#include "JoltPhysics.hpp"
#include "OpenGL.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <Jolt/Physics/Collision/Shape/CylinderShape.h>

constexpr float M_PI = 3.14159265358979323846f;

ES::Plugin::Object::Component::Mesh CreateCylinderMesh(
	const glm::vec3 &size,
	int segments,
	const glm::vec3 &up)
{
	ES::Plugin::Object::Component::Mesh mesh;

	float radius = size.x;
	float height = size.y;

	// Normalize up vector and compute a transform to align Y-axis with "up"
	glm::vec3 up_normalized = glm::normalize(up);
	glm::vec3 default_up(0.0f, 1.0f, 0.0f);

	// Calculate rotation matrix to align default Y-axis with desired up vector
    glm::mat3 rotation_matrix(1.0f);
    if (glm::length(up_normalized - default_up) > 1e-6f) {
        glm::vec3 axis = glm::cross(default_up, up_normalized);
        if (glm::length(axis) > 1e-6f) {
            axis = glm::normalize(axis);
            float angle = glm::acos(glm::clamp(glm::dot(default_up, up_normalized), -1.0f, 1.0f));
            rotation_matrix = glm::mat3(glm::rotate(glm::mat4(1.0f), angle, axis));
        } else if (glm::dot(default_up, up_normalized) < 0) {
            // 180 degree rotation case
            rotation_matrix = glm::mat3(-1.0f, 0.0f, 0.0f,
                                       0.0f, -1.0f, 0.0f,
                                       0.0f, 0.0f, 1.0f);
        }
    }

    float half_height = height * 0.5f;

    // Generate side vertices first (two rings)
    uint32_t side_vertex_start = 0;
    for (int i = 0; i < segments; ++i) {
        float angle = 2.0f * M_PI * i / segments;
        float cos_angle = std::cos(angle);
        float sin_angle = std::sin(angle);

        // Calculate side normal in local space first, then transform
        glm::vec3 local_normal(cos_angle, 0.0f, sin_angle);
        glm::vec3 side_normal = glm::normalize(rotation_matrix * local_normal);

        // Top ring vertex
        glm::vec3 top_local(radius * cos_angle, half_height, radius * sin_angle);
        glm::vec3 top_pos = rotation_matrix * top_local;
        mesh.vertices.push_back(top_pos);
        mesh.normals.push_back(side_normal);
        mesh.texCoords.push_back(glm::vec2((float)i / segments, 1.0f));

        // Bottom ring vertex
        glm::vec3 bottom_local(radius * cos_angle, -half_height, radius * sin_angle);
        glm::vec3 bottom_pos = rotation_matrix * bottom_local;
        mesh.vertices.push_back(bottom_pos);
        mesh.normals.push_back(side_normal);
        mesh.texCoords.push_back(glm::vec2((float)i / segments, 0.0f));
    }

    // Generate cap vertices
    uint32_t top_cap_start = segments * 2;
    uint32_t bottom_cap_start = top_cap_start + segments + 1;

    // Top cap center
    glm::vec3 top_center_pos = rotation_matrix * glm::vec3(0.0f, half_height, 0.0f);
    glm::vec3 top_normal = glm::normalize(rotation_matrix * glm::vec3(0.0f, 1.0f, 0.0f));
    mesh.vertices.push_back(top_center_pos);
    mesh.normals.push_back(top_normal);
    mesh.texCoords.push_back(glm::vec2(0.5f, 0.5f));

    // Top cap edge vertices
    for (int i = 0; i < segments; ++i) {
        float angle = 2.0f * M_PI * i / segments;
        float cos_angle = std::cos(angle);
        float sin_angle = std::sin(angle);

        glm::vec3 top_local(radius * cos_angle, half_height, radius * sin_angle);
        glm::vec3 top_pos = rotation_matrix * top_local;
        mesh.vertices.push_back(top_pos);
        mesh.normals.push_back(top_normal);
        mesh.texCoords.push_back(glm::vec2(0.5f + 0.5f * cos_angle, 0.5f + 0.5f * sin_angle));
    }

    // Bottom cap center
    glm::vec3 bottom_center_pos = rotation_matrix * glm::vec3(0.0f, -half_height, 0.0f);
    glm::vec3 bottom_normal = glm::normalize(rotation_matrix * glm::vec3(0.0f, -1.0f, 0.0f));
    mesh.vertices.push_back(bottom_center_pos);
    mesh.normals.push_back(bottom_normal);
    mesh.texCoords.push_back(glm::vec2(0.5f, 0.5f));

    // Bottom cap edge vertices
    for (int i = 0; i < segments; ++i) {
        float angle = 2.0f * M_PI * i / segments;
        float cos_angle = std::cos(angle);
        float sin_angle = std::sin(angle);

        glm::vec3 bottom_local(radius * cos_angle, -half_height, radius * sin_angle);
        glm::vec3 bottom_pos = rotation_matrix * bottom_local;
        mesh.vertices.push_back(bottom_pos);
        mesh.normals.push_back(bottom_normal);
        mesh.texCoords.push_back(glm::vec2(0.5f + 0.5f * cos_angle, 0.5f - 0.5f * sin_angle));
    }

    // Generate indices for side faces
    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;

        uint32_t top_current = i * 2;
        uint32_t bottom_current = i * 2 + 1;
        uint32_t top_next = next * 2;
        uint32_t bottom_next = next * 2 + 1;

        // First triangle (counter-clockwise when viewed from outside)
        mesh.indices.push_back(top_current);
        mesh.indices.push_back(top_next);
        mesh.indices.push_back(bottom_current);

        // Second triangle
        mesh.indices.push_back(top_next);
        mesh.indices.push_back(bottom_next);
        mesh.indices.push_back(bottom_current);
    }

    // Generate indices for top cap
    uint32_t top_center_idx = top_cap_start;
    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;

        uint32_t current_edge = top_cap_start + 1 + i;
        uint32_t next_edge = top_cap_start + 1 + next;

        // Triangle facing upward (counter-clockwise from above)
        mesh.indices.push_back(top_center_idx);
        mesh.indices.push_back(next_edge);
        mesh.indices.push_back(current_edge);
    }

    // Generate indices for bottom cap
    uint32_t bottom_center_idx = bottom_cap_start;
    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;

        uint32_t current_edge = bottom_cap_start + 1 + i;
        uint32_t next_edge = bottom_cap_start + 1 + next;

        // Triangle facing downward (counter-clockwise when viewed from outside)
        mesh.indices.push_back(bottom_center_idx);
        mesh.indices.push_back(current_edge);
        mesh.indices.push_back(next_edge);
    }

	return mesh;
}


ES::Engine::Entity CreateCylinder(
	ES::Engine::Core &core,
	const glm::vec3 &position,
	const glm::quat &rotation,
	const glm::vec3 &size)
{
	using namespace JPH;

	glm::vec3 cylinder_scale = glm::vec3(1.0f);

	// Jolt's CylinderShape uses height from center to top, so divide by 2
	float radius = size.x;
	float half_height = size.y / 2.0f;

	auto cylinder_shape_settings = std::make_shared<JPH::CylinderShapeSettings>(half_height, radius);

	ES::Engine::Entity cylinder = core.CreateEntity();
	cylinder.AddComponent<ES::Plugin::Object::Component::Transform>(core, position, cylinder_scale, rotation);
	cylinder.AddComponent<ES::Plugin::Physics::Component::RigidBody3D>(
		core,
		cylinder_shape_settings,
		JPH::EMotionType::Dynamic,
		ES::Plugin::Physics::Utils::Layers::MOVING);

	auto mesh = CreateCylinderMesh(size);
	cylinder.AddComponent<ES::Plugin::Object::Component::Mesh>(core, mesh);

	return cylinder;
}

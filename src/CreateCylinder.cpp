#include "CreateCylinder.hpp"

#include "JoltPhysics.hpp"
#include "OpenGL.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <Jolt/Physics/Collision/Shape/CylinderShape.h>

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

	glm::mat4 align = glm::mat4(1.0f);
	if (!glm::all(glm::equal(up_normalized, default_up))) {
		glm::vec3 axis = glm::cross(default_up, up_normalized);
		float angle = acos(glm::dot(default_up, up_normalized));
		if (glm::length(axis) > 0.0001f)
			align = glm::rotate(glm::mat4(1.0f), angle, glm::normalize(axis));
	}

	// Top and bottom center points before transformation
	glm::vec3 top_center(0.0f, height, 0.0f);
	glm::vec3 bottom_center(0.0f, -height, 0.0f);

	// Generate side vertices
	for (int i = 0; i < segments; ++i) {
		float angle = 2.0f * glm::pi<float>() * i / segments;
		float x = radius * cos(angle);
		float z = radius * sin(angle);

		glm::vec3 top(x, height, z);
		glm::vec3 bottom(x, -height, z);
		glm::vec3 normal = glm::normalize(glm::vec3(x, 0.0f, z));

		top = glm::vec3(align * glm::vec4(top, 1.0f));
		bottom = glm::vec3(align * glm::vec4(bottom, 1.0f));
		normal = glm::normalize(glm::vec3(align * glm::vec4(normal, 0.0f)));

		mesh.vertices.push_back(top);
		mesh.normals.push_back(normal);
		mesh.vertices.push_back(bottom);
		mesh.normals.push_back(normal);
	}

	// Side indices
	for (int i = 0; i < segments; ++i) {
		int top1 = i * 2;
		int bottom1 = top1 + 1;
		int top2 = (i * 2 + 2) % (segments * 2);
		int bottom2 = (i * 2 + 3) % (segments * 2);

		mesh.indices.push_back(top1);
		mesh.indices.push_back(bottom1);
		mesh.indices.push_back(top2);
		mesh.indices.push_back(top2);
		mesh.indices.push_back(bottom1);
		mesh.indices.push_back(bottom2);
	}

	// Add top cap
	int top_center_index = mesh.vertices.size();
	top_center = glm::vec3(align * glm::vec4(top_center, 1.0f));
	mesh.vertices.push_back(top_center);
	mesh.normals.push_back(up_normalized);

	std::vector<int> top_ring_indices;
	for (int i = 0; i < segments; ++i) {
		float angle = 2.0f * glm::pi<float>() * i / segments;
		float x = radius * cos(angle);
		float z = radius * sin(angle);
		glm::vec3 pos = glm::vec3(x, height, z);
		pos = glm::vec3(align * glm::vec4(pos, 1.0f));

		mesh.vertices.push_back(pos);
		mesh.normals.push_back(up_normalized);
		top_ring_indices.push_back(mesh.vertices.size() - 1);
	}

	for (int i = 0; i < segments; ++i) {
		int next = (i + 1) % segments;
		mesh.indices.push_back(top_center_index);
		mesh.indices.push_back(top_ring_indices[i]);
		mesh.indices.push_back(top_ring_indices[next]);
	}

	// Add bottom cap
	int bottom_center_index = mesh.vertices.size();
	bottom_center = glm::vec3(align * glm::vec4(bottom_center, 1.0f));
	mesh.vertices.push_back(bottom_center);
	mesh.normals.push_back(-up_normalized);

	std::vector<int> bottom_ring_indices;
	for (int i = 0; i < segments; ++i) {
		float angle = 2.0f * glm::pi<float>() * i / segments;
		float x = radius * cos(angle);
		float z = radius * sin(angle);
		glm::vec3 pos = glm::vec3(x, -height, z);
		pos = glm::vec3(align * glm::vec4(pos, 1.0f));

		mesh.vertices.push_back(pos);
		mesh.normals.push_back(-up_normalized);
		bottom_ring_indices.push_back(mesh.vertices.size() - 1);
	}

	for (int i = 0; i < segments; ++i) {
		int next = (i + 1) % segments;
		mesh.indices.push_back(bottom_center_index);
		mesh.indices.push_back(bottom_ring_indices[next]);
		mesh.indices.push_back(bottom_ring_indices[i]);
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
	float half_height = size.y;

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

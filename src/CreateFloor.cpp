#include "CreateFloor.hpp"

#include "CreateBox.hpp"
#include "JoltPhysics.hpp"
#include "Object.hpp"
#include "OpenGL.hpp"

ES::Engine::Entity CreateFloor(ES::Engine::Core &core)
{
	using namespace JPH;

	glm::vec3 floor_position(0.0f, 0.0f, 0.0f);
	glm::vec3 floor_size(20.0f, 1.0f, 20.0f);

	ES::Engine::Entity floor = CreateBox(
		core,
		floor_position,
		glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
		floor_size
	);

	floor.AddComponent<ES::Plugin::OpenGL::Component::ShaderHandle>(core, "no_light");
    floor.AddComponent<ES::Plugin::OpenGL::Component::MaterialHandle>(core, "floor");
    floor.AddComponent<ES::Plugin::OpenGL::Component::ModelHandle>(core, "floor");

	return floor;
}
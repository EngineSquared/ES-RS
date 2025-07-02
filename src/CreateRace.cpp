#include "CreateRace.hpp"

#include "CreateBox.hpp"
#include "LoadCourse.hpp"
#include "JoltPhysics.hpp"
#include "Object.hpp"
#include "OpenGL.hpp"

void CreateRace(ES::Engine::Core &core)
{
	using namespace JPH;

	glm::vec3 level_position(0.0f, 0.0f, 0.0f);
	glm::vec3 level_size(1.0f, 1.0f, 1.0f);

	LoadCourseTextures(core);
	LoadCourseModels(core, level_position, glm::quat(1.0f, 0.0f, 0.0f, 0.0f), level_size);
}
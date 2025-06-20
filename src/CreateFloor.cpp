#include "CreateFloor.hpp"

#include "CreateBox.hpp"
#include "CreateMap.hpp"
#include "JoltPhysics.hpp"
#include "Object.hpp"
#include "OpenGL.hpp"

ES::Engine::Entity CreateFloor(ES::Engine::Core &core)
{
	using namespace JPH;

	glm::vec3 level_position(0.0f, 0.0f, 0.0f);
	glm::vec3 level_size(1.0f, 1.0f, 1.0f);

	ES::Engine::Entity level = CreateMap(core,
		level_position,
		glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
		level_size);
	
	LoadCourseTextures(core);

	level.AddComponent<ES::Plugin::OpenGL::Component::ShaderHandle>(core, "noTextureLightShadow");
    level.AddComponent<ES::Plugin::OpenGL::Component::MaterialHandle>(core, "level");
    level.AddComponent<ES::Plugin::OpenGL::Component::ModelHandle>(core, "level");
	level.AddComponent<ES::Plugin::OpenGL::Component::TextureHandle>(core, "tex_ROADA");

	return level;
}
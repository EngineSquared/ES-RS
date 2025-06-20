#pragma once

#include "LoadNoLightShader.hpp"

#include "OpenGL.hpp"
#include "Camera.hpp" // TODO: remove when camera is in OpenGL

void LoadNoLightShader(ES::Engine::Core &core)
{
	// This "using" allow to use "_hs" compile time hashing for strings
	using namespace entt;
    using namespace ES::Plugin;
	const std::string vertexShader = "asset/shader/no_light/no_light.vs";
	const std::string fragmentShader = "asset/shader/no_light/no_light.fs";
	auto &shaderManager = core.GetResource<OpenGL::Resource::ShaderManager>();
    OpenGL::Utils::ShaderProgram &sp = shaderManager.Add("no_light"_hs);
    sp.Create();
    sp.InitFromFiles(vertexShader, fragmentShader);
	sp.AddUniform("MVP");
    sp.AddUniform("ModelMatrix");
    sp.AddUniform("NormalMatrix");

    sp.AddUniform("Material.Ka");
    sp.AddUniform("Material.Kd");
    sp.AddUniform("Material.Ks");
    sp.AddUniform("Material.Shiness");

    sp.AddUniform("CamPos");

	sp.Use();
    glUniform3fv(sp.GetUniform("CamPos"), 1, glm::value_ptr(core.GetResource<OpenGL::Resource::Camera>().viewer.getViewPoint()));
	sp.Disable();
}

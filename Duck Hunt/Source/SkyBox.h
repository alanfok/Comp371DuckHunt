#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Renderer.h"

class SkyBox
{
public:
	SkyBox();
	~SkyBox();

	void Draw(const glm::mat4& view, const glm::mat4& projection);

private:
	unsigned int cubemapTexture;

	GLuint VAO;
	GLuint VBO;

	
	GLuint loadCubemap(std::vector<std::string> faces);
};


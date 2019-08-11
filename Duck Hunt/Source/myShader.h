#pragma once
#include <glm/glm.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstdlib>


#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <fstream>
#include <sstream>

class myShader
{
public:
	myShader(const GLchar *vertexPath, const GLchar *fragmentPath);
	~myShader();

	GLuint Program;

	void Use();

	void setVec3(const char* ch, const float& x, const float& y, const float& z);

	void setVec3(const char* ch, const glm::vec3& pos);

	void setInt(const char* ch, const int& i);

	void setMat4(const char* ch, const glm::mat4& mat);
};


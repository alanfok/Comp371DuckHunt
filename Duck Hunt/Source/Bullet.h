#pragma once
#include <iostream>
#include <list>

#define GLEW_STATIC 1   // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>    // Include GLEW - OpenGL Extension Wrangler

#include <GLFW/glfw3.h> // cross-platform interface for creating a graphical context,
// initializing OpenGL and binding inputs

#include <glm/glm.hpp>  // GLM is an optimized math library with syntax to similar to OpenGL Shading Language
#include <glm/gtc/matrix_transform.hpp> // include this to create transformation matrices
#include <glm/common.hpp>
#include "Renderer.h"
#include "SphereModel.h"

class Renderer;

using namespace glm;
using namespace std;

class Bullet :public SphereModel
{
public:
	Bullet(vec3 position, vec3 velocity, glm::vec3 size = glm::vec3(1.0f, 1.0f, 1.0f));
	virtual void Update(float dt);
	void Draw();
	~Bullet();
private:
	GLuint mWorldMatrixLocation;
	vec3 mPosition;
	vec3 mVelocity;
};


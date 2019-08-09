

#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <list>
#include <glm/glm.hpp>

class Lighting
{
public:
	Lighting();
	Lighting(glm::vec4 sun, glm::vec3 sunColor, glm::vec3 billboardColor, glm::vec3 gunColor, float amb, glm::vec3 ambColor);
	~Lighting();

	void Update();	//Update the position of the lighting from the gun's tip, since it moves when the camera moves

	glm::vec4 GetSunlightVector();
	glm::vec3 GetSunlightColor();
	glm::vec3 GetBillboardLightColor();
	glm::vec4 GetGunLightingVector();
	glm::vec3 GetGunLightingColor();
	float GetAmbientLightingFloat();
	glm::vec3 GetAmbientLightingColor();
private:
	glm::vec4 sunlight;
	glm::vec3 sunlightColor;

	glm::vec3 billboardLightColor;

	glm::vec4 gunLight;
	glm::vec3 gunLightColor;

	float ambient;
	glm::vec3 ambientColor;
};
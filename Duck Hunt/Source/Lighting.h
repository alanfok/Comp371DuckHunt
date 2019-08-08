

#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <list>
#include <glm/glm.hpp>

class Lighting
{
public:
	Lighting();
	~Lighting();

	void Update();	//Update the position of the lighting from the gun's tip, since it moves when the camera moves

	glm::vec4 GetSunLighting();
	glm::vec4 GetBillboardLighting();
	glm::vec4 GetGunLighting();
private:
	glm::vec4 sunlight;
	glm::vec4 billboardLight;
	glm::vec4 gunLight;
};
#include "Lighting.h"

Lighting::Lighting()
{

}

Lighting::~Lighting()
{
	sunlight = glm::vec4(5.0f, -10.0f, -5.0f, 0.0f);
	billboardLight = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
	gunLight = glm::vec4(0.0f); //Doesn't matter, since it gets updated
}

void Lighting::Update()
{
	gunLight = glm::vec4(glm::vec3(0.0f), 1.0f); //FIXIT change the 'vec3(0.0f)' part to move with the gun in front of the camera!
}

glm::vec4 Lighting::GetSunLighting()
{
	return sunlight;
}

glm::vec4 Lighting::GetBillboardLighting()
{
	return billboardLight;
}

glm::vec4 Lighting::GetGunLighting()
{
	return gunLight;
}
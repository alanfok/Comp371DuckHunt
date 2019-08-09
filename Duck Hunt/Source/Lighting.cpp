#include "Lighting.h"
#include "World.h"

Lighting::Lighting()
{
	sunlight = glm::vec4(5.0f, -10.0f, -5.0f, 0.0f);
	sunlightColor = glm::vec3(1.0f, 1.0f, 1.0f);

	backlight = glm::vec4(0.0f, 0.0f, 5.0f, 0.0f);

	billboardLightColor = glm::vec3(1.0f, 1.0f, 1.0f);

	gunLight = glm::vec4(0.0f); //Doesn't matter, since it gets updated
	gunLightColor = glm::vec3(1.0f, 1.0f, 0.0f);

	ambient = 0.3f;
	ambientColor = glm::vec3(1.0f, 1.0f, 1.0f);
}

Lighting::Lighting(glm::vec4 sun, glm::vec3 sunColor, glm::vec4 back, glm::vec3 billboardColor, glm::vec3 gunColor, float amb, glm::vec3 ambColor)
{
	sunlight = sun;
	sunlightColor = sunColor;

	backlight = back;

	billboardLightColor = billboardColor;

	gunLight = glm::vec4(0.0f); //Doesn't matter, since it gets updated
	gunLightColor = gunColor;

	ambient = amb;
	ambientColor = ambColor;
}


Lighting::~Lighting()
{

}

glm::vec4 Lighting::GetSunlightVector()
{
	return sunlight;
}

glm::vec3 Lighting::GetSunlightColor()
{
	return sunlightColor;
}

glm::vec4 Lighting::GetBacklightVector()
{
	return backlight;
}

glm::vec3 Lighting::GetBillboardLightColor()
{
	return billboardLightColor;
}

glm::vec4 Lighting::GetGunLightingVector()
{
	return gunLight;
}

glm::vec3 Lighting::GetGunLightingColor()
{
	return gunLightColor;
}

float Lighting::GetAmbientLightingFloat()
{
	return ambient;
}

glm::vec3 Lighting::GetAmbientLightingColor()
{
	return ambientColor;
}

void Lighting::Update()
{
	glm::mat4 viewMatrix = World::GetInstance()->GetInverseWorldMatrix();
	vec3 camLookAt = normalize(-viewMatrix[2]);
	vec3 cameraPosition = viewMatrix[3];

	if (timer > 0)
	{
		timer -= 1;
	}

	if (World::GetInstance()->IsShooting())
	{
		timer = 3;
	}

	if (timer > 0)
	{
		gunLight = glm::vec4(camLookAt + cameraPosition, 1.0f);
	}
	else
	{
		vec3 camLookAtExtreme = camLookAt * -1000.0f;

		gunLight = glm::vec4(cameraPosition + camLookAtExtreme, 1.0f);
	}
}
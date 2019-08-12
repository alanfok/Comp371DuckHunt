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

	//distanceFogColor = glm::vec3(0.294f, 0.78f, 0.98f); //Original duck hunt background
	distanceFogColor = glm::vec3(0.663f, 0.749f, 0.792f); //Winter sky
	distanceFogDetails = glm::vec3(16.0f, 64.0f, 0.9f);

	lightView = glm::lookAt(-vec3(sunlight),		// Camera position
							vec3(sunlight),			// Look towards this point
							vec3(5.0f, 5.0f, -10.0f)// Up vector
	);
	lightProjection = glm::ortho<float>(-10, 10, -10, 10, -10, 20);
}

Lighting::Lighting(glm::vec4 sun, glm::vec3 sunColor, glm::vec4 back, glm::vec3 billboardColor, glm::vec3 gunColor, float amb, glm::vec3 ambColor, glm::vec3 fog, glm::vec3 fogDetails)
{
	sunlight = sun;
	sunlightColor = sunColor;

	backlight = back;

	billboardLightColor = billboardColor;

	gunLight = glm::vec4(0.0f); //Doesn't matter, since it gets updated
	gunLightColor = gunColor;

	ambient = amb;
	ambientColor = ambColor;

	distanceFogColor = fog;
	distanceFogDetails = fogDetails;
	

	lightView = glm::lookAt(-vec3(sunlight),		// Camera position
							vec3(sunlight),			// Look towards this point
							vec3(5.0f, 5.0f, -10.0f)// Up vector
	);
	lightProjection = glm::ortho<float>(-10, 10, -10, 10, -10, 20);
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

glm::vec3 Lighting::GetDistanceFogColor()
{
	return distanceFogColor;
}

glm::vec3 Lighting::GetDistanceFogDetails()
{
	return distanceFogDetails;
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
		gunLight = glm::vec4(camLookAt * 5.0f + cameraPosition, 1.0f);
	}
	else
	{
		vec3 camLookAtExtreme = camLookAt * -1000.0f;

		gunLight = glm::vec4(cameraPosition + camLookAtExtreme, 1.0f);
	}
}

glm::mat4 Lighting::GetLightLookAt()
{
	/*return glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f),
							glm::vec3(0.0f, 0.0f, 0.0f),
							glm::vec3(0.0f, 1.0f, 0.0f));*/

	return  lightView; 
}

glm::mat4 Lighting::GetLightProjection()
{
	return lightProjection;
}
//
// COMP 371 Assignment Framework
//
// Created by Nicolas Bergeron on 8/7/14.
// Updated by Gary Chang on 14/1/15
//
// Copyright (c) 2014-2019 Concordia University. All rights reserved.
//

#include "World.h"
#include "Renderer.h"
#include "ParsingHelper.h"

#include "StaticCamera.h"
#include "FirstPersonCamera.h"

#include "CubeModel.h"
#include "SphereModel.h"
#include "Animation.h"
#include "Billboard.h"
#include <GLFW/glfw3.h>
#include "EventManager.h"
#include "TextureLoader.h"

#include "ParticleDescriptor.h"
#include "ParticleEmitter.h"
#include "ParticleSystem.h"


using namespace std;
using namespace glm;

World* World::instance;


World::World()
{
    instance = this;

	// Setup Camera
	mCamera.push_back(new FirstPersonCamera(vec3(3.0f, 5.0f, 20.0f)));
	mCamera.push_back(new StaticCamera(vec3(3.0f, 30.0f, 5.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)));
	mCamera.push_back(new StaticCamera(vec3(0.5f,  0.5f, 5.0f), vec3(0.0f, 0.5f, 0.0f), vec3(0.0f, 1.0f, 0.0f)));
	mCurrentCamera = 0;

    
#if defined(PLATFORM_OSX)
//    int billboardTextureID = TextureLoader::LoadTexture("Textures/BillboardTest.bmp");
    int billboardTextureID = TextureLoader::LoadTexture("Textures/Particle.png");
#else
//    int billboardTextureID = TextureLoader::LoadTexture("../Assets/Textures/BillboardTest.bmp");
    int billboardTextureID = TextureLoader::LoadTexture("../Assets/Textures/Particle.png");
	int featherBillboardTextureID = TextureLoader::LoadTexture("../Assets/Textures/Feather.png");
	int flakeBillboardTextureID = TextureLoader::LoadTexture("../Assets/Textures/flake.png");
	int snowBillboardTextureID = TextureLoader::LoadTexture("../Assets/Textures/snow.png");
#endif
    assert(billboardTextureID != 0);

    mpBillboardList = new BillboardList(2048, billboardTextureID);
	mpFeatherBillboardList = new BillboardList(1024, featherBillboardTextureID);
	mpFlakeBillboardList = new BillboardList(1024, flakeBillboardTextureID);
	mpSnowBillboardList = new BillboardList(1024, snowBillboardTextureID);
    // TODO - You can un-comment out these 2 temporary billboards and particle system
    // That can help you debug billboards, you can set the billboard texture to billboardTest.png
    /*    Billboard *b = new Billboard();
     b->size  = glm::vec2(2.0, 2.0);
     b->position = glm::vec3(0.0, 3.0, 0.0);
     b->color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
     
     Billboard *b2 = new Billboard();
     b2->size  = glm::vec2(2.0, 2.0);
     b2->position = glm::vec3(0.0, 3.0, 1.0);
     b2->color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

     mpBillboardList->AddBillboard(b);
     mpBillboardList->AddBillboard(b2);
     */    // TMP

	mpWorldLighting = new Lighting();
}

World::~World()
{
	// Models
	for (vector<Model*>::iterator it = mModel.begin(); it < mModel.end(); ++it)
	{
		delete *it;
	}

	mModel.clear();

	for (vector<Animation*>::iterator it = mAnimation.begin(); it < mAnimation.end(); ++it)
	{
		delete *it;
	}

	mAnimation.clear();

	for (vector<AnimationKey*>::iterator it = mAnimationKey.begin(); it < mAnimationKey.end(); ++it)
	{
		delete *it;
	}

	mAnimationKey.clear();

	// Camera
	for (vector<Camera*>::iterator it = mCamera.begin(); it < mCamera.end(); ++it)
	{
		delete *it;
	}
	mCamera.clear();
    
    for (vector<ParticleSystem*>::iterator it = mParticleSystemList.begin(); it < mParticleSystemList.end(); ++it)
    {
        delete *it;
    }
    mParticleSystemList.clear();
    
    for (vector<ParticleDescriptor*>::iterator it = mParticleDescriptorList.begin(); it < mParticleDescriptorList.end(); ++it)
    {
        delete *it;
    }
    mParticleDescriptorList.clear();

    
	delete mpBillboardList;
	delete mpFeatherBillboardList;
	delete mpFlakeBillboardList;
	delete mpSnowBillboardList;

	delete mpWorldLighting;
}

World* World::GetInstance()
{
    return instance;
}

void World::Update(float dt)
{
	// User Inputs
	// 0 1 2 to change the Camera
	if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_1 ) == GLFW_PRESS)
	{
		mCurrentCamera = 0;
	}
	else if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_2 ) == GLFW_PRESS)
	{
		if (mCamera.size() > 1)
		{
			mCurrentCamera = 1;
		}
	}
	else if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_3 ) == GLFW_PRESS)
	{
		if (mCamera.size() > 2)
		{
			mCurrentCamera = 2;
		}
	}

	// Spacebar to change the shader
	if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_0 ) == GLFW_PRESS)
	{
		Renderer::SetShader(SHADER_SOLID_COLOR);
	}
	else if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_9 ) == GLFW_PRESS)
	{
		Renderer::SetShader(SHADER_BLUE);
	}

	mInverseViewMatrix = glm::inverse(GetCurrentCamera()->GetViewMatrix());

	// shooting function
	if (lastMouseState == false && glfwGetMouseButton(EventManager::GetWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
		const float projectileSpeed = 50.0f;
		//mat4 viewMatrix = glm::inverse(GetCurrentCamera()->GetViewMatrix());
		vec3 camLookAt= -mInverseViewMatrix[2];
		vec3 cameraPosition = mInverseViewMatrix[3];
		std::cout << camLookAt.x << " " << camLookAt.y << " " << camLookAt.z << "\n";
		Bullet *bt = new Bullet(cameraPosition, projectileSpeed *  camLookAt);
		bulletList.push_back(bt);
        //printf("I like trains");
        cout << "clicked" << endl;
        lastMouseState = true;
    }

    if (glfwGetMouseButton(EventManager::GetWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    {
        lastMouseState = false;
    }

    // Update animation and keys
    for (vector<Animation*>::iterator it = mAnimation.begin(); it < mAnimation.end(); ++it)
    {
        (*it)->Update(dt);
    }
    
    for (vector<AnimationKey*>::iterator it = mAnimationKey.begin(); it < mAnimationKey.end(); ++it)
    {
        (*it)->Update(dt);
    }

	//bullet
	for (list<Bullet*>::iterator it = bulletList.begin(); it != bulletList.end(); ++it)
	{
		(*it)->Update(dt);
	}

	// Update current Camera
	mCamera[mCurrentCamera]->Update(dt);

	// Update models
	for (vector<Model*>::iterator it = mModel.begin(); it < mModel.end(); ++it)
	{
		(*it)->Update(dt);
	}


    
    // Update billboards
    
    for (vector<ParticleSystem*>::iterator it = mParticleSystemList.begin(); it != mParticleSystemList.end(); ++it)
    {
        (*it)->Update(dt);
    }
    
    mpBillboardList->Update(dt);
	mpFeatherBillboardList->Update(dt);
	mpFlakeBillboardList->Update(dt);
	mpSnowBillboardList->Update(dt);

	mpWorldLighting->Update();
}

void World::Draw()
{
	Renderer::BeginFrame();
	
	// Set shader to use
	Renderer::SetShader(SHADER_PHONG);
	glUseProgram(Renderer::GetShaderProgramID());

	// This looks for the MVP Uniform variable in the Vertex Program
	GLuint VPMatrixLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "ViewProjectionTransform");
	GLuint VMatrixLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "ViewTransform");
	GLuint PMatrixLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "ProjectonTransform");
	GLuint WLPositionLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "WorldLightPosition");
	GLuint LColorLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightColor");
	GLuint LAttenuationLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightAttenuation");

	// Light Coefficients
	const vec3 lightColor = GetLightColor();//GetGunLightColor();//GetLightColor();
	const vec3 lightAttenuation = GetLightAttenuation();
	const vec4 lightPosition = GetLightPosition();//GetGunLightPosition();//GetLightPosition(); // If w = 1.0f, we have a point light
	//const vec4 lightPosition = vec4(5.0f, -5.0f, 5.0f, 0.0f); // If w = 0.0f, we have a directional light


	glUniform4f(WLPositionLocation, lightPosition.x, lightPosition.y, lightPosition.z, lightPosition.w);
	glUniform3f(LColorLocation, lightColor.r, lightColor.g, lightColor.b);
	glUniform3f(LAttenuationLocation, lightAttenuation.x, lightAttenuation.y, lightAttenuation.z);

	// Send the view projection constants to the shader
	mat4 VP = mCamera[mCurrentCamera]->GetViewProjectionMatrix();
	glUniformMatrix4fv(VPMatrixLocation, 1, GL_FALSE, &VP[0][0]);

	//mat4 VMatrix = mCamera[mCurrentCamera]->GetViewMatrix();
	//mat4 PMatrix = mCamera[mCurrentCamera]->GetProjectionMatrix();
	glUniformMatrix4fv(VMatrixLocation, 1, GL_FALSE, &mCamera[mCurrentCamera]->GetViewMatrix()[0][0]);
	glUniformMatrix4fv(PMatrixLocation, 1, GL_FALSE, &mCamera[mCurrentCamera]->GetProjectionMatrix()[0][0]);



	// Draw models
	for (vector<Model*>::iterator it = mModel.begin(); it < mModel.end(); ++it)
	{
		(*it)->Draw();
	}

	//Drwa bullets
	for (list<Bullet*>::iterator it = bulletList.begin(); it != bulletList.end(); ++it)
	{
		(*it)->Draw();
	}
	// Draw Path Lines
	


	// Set Shader for path lines
	unsigned int prevShader = Renderer::GetCurrentShader();
	Renderer::SetShader(SHADER_PATH_LINES);
	glUseProgram(Renderer::GetShaderProgramID());

	// Send the view projection constants to the shader
	VPMatrixLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "ViewProjectionTransform");
	glUniformMatrix4fv(VPMatrixLocation, 1, GL_FALSE, &VP[0][0]);

	for (vector<Animation*>::iterator it = mAnimation.begin(); it < mAnimation.end(); ++it)
	{
		mat4 VP = mCamera[mCurrentCamera]->GetViewProjectionMatrix();
		glUniformMatrix4fv(VPMatrixLocation, 1, GL_FALSE, &VP[0][0]);

		(*it)->Draw();
	}

	for (vector<AnimationKey*>::iterator it = mAnimationKey.begin(); it < mAnimationKey.end(); ++it)
	{
		mat4 VP = mCamera[mCurrentCamera]->GetViewProjectionMatrix();
		glUniformMatrix4fv(VPMatrixLocation, 1, GL_FALSE, &VP[0][0]);

		(*it)->Draw();
	}

    Renderer::CheckForErrors();
    
    // Draw Billboards
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    mpBillboardList->Draw();
	mpFeatherBillboardList->Draw();
	mpFlakeBillboardList->Draw();
	mpSnowBillboardList->Draw();
    glDisable(GL_BLEND);


	// Restore previous shader
	Renderer::SetShader((ShaderType) prevShader);

	Renderer::EndFrame();
}

void World::LoadScene(const char * scene_path)
{
	// Using case-insensitive strings and streams for easier parsing
	ci_ifstream input;
	input.open(scene_path, ios::in);

	// Invalid file
	if(input.fail() )
	{	 
		fprintf(stderr, "Error loading file: %s\n", scene_path);
		getchar();
		exit(-1);
	}

	ci_string item;
	while( std::getline( input, item, '[' ) )   
	{
        ci_istringstream iss( item );

		ci_string result;
		if( std::getline( iss, result, ']') )
		{
			if( result == "cube" )
			{
				// Box attributes
				CubeModel* cube = new CubeModel();
				cube->Load(iss);
				mModel.push_back(cube);
			}
            else if( result == "sphere" )
            {
                SphereModel* sphere = new SphereModel();
                sphere->Load(iss);
                mModel.push_back(sphere);
            }
			else if ( result == "animationkey" )
			{
				AnimationKey* key = new AnimationKey();
				key->Load(iss);
				mAnimationKey.push_back(key);
			}
			else if (result == "animation")
			{
				Animation* anim = new Animation();
				anim->Load(iss);
				mAnimation.push_back(anim);
			}
            else if (result == "particledescriptor")
            {
                ParticleDescriptor* psd = new ParticleDescriptor();
                psd->Load(iss);
                AddParticleDescriptor(psd);
            }
			else if (result == "lighting")
			{
				delete mpWorldLighting;
				mpWorldLighting = new Lighting(); //FIXIT make it load anything
				//mpWorldLighting->Load(iss);
			}
			else if ( result.empty() == false && result[0] == '#')
			{
				// this is a comment line
			}
			else
			{
				fprintf(stderr, "Error loading scene file... !");
				getchar();
				exit(-1);
			}
	    }
	}
	input.close();

	// Set Animation vertex buffers
	for (vector<Animation*>::iterator it = mAnimation.begin(); it < mAnimation.end(); ++it)
	{
		// Draw model
		(*it)->CreateVertexBuffer();
	}
}

Animation* World::FindAnimation(ci_string animName)
{
    for(std::vector<Animation*>::iterator it = mAnimation.begin(); it < mAnimation.end(); ++it)
    {
        if((*it)->GetName() == animName)
        {
            return *it;
        }
    }
    return nullptr;
}

AnimationKey* World::FindAnimationKey(ci_string keyName)
{
    for(std::vector<AnimationKey*>::iterator it = mAnimationKey.begin(); it < mAnimationKey.end(); ++it)
    {
        if((*it)->GetName() == keyName)
        {
            return *it;
        }
    }
    return nullptr;
}

const Camera* World::GetCurrentCamera() const
{
     return mCamera[mCurrentCamera];
}

void World::AddBillboard(Billboard* b, ci_string particleTexture)
{
	if (particleTexture == "Feather")
	{
		mpFeatherBillboardList->AddBillboard(b);
	}
	else if (particleTexture == "Snow")
	{
		mpSnowBillboardList->AddBillboard(b);
	}
	else if (particleTexture == "Flake")
	{
		mpFlakeBillboardList->AddBillboard(b);
	}
	else
	{ //Gunsmoke goes in here
		mpBillboardList->AddBillboard(b);
	}
	/*
	switch (particleTexture) {
	case "Feather": mpFeatherBillboardList->AddBillboard(b);
	case "Smoke": mpBillboardList->AddBillboard(b);
	case "Snow": mpSnowBillboardList->AddBillboard(b);
	}
	
	if (!feather)
	{
		mpBillboardList->AddBillboard(b);
	}
	else
	{
		mpFeatherBillboardList->AddBillboard(b);
	}*/
}

void World::RemoveBillboard(Billboard* b, ci_string particleTexture)
{
	if (particleTexture == "Feather")
	{
		mpFeatherBillboardList->RemoveBillboard(b);
	}
	else if (particleTexture == "Snow")
	{
		mpSnowBillboardList->RemoveBillboard(b);
	}
	else if (particleTexture == "Flake")
	{
		mpFlakeBillboardList->RemoveBillboard(b);
	}
	else
	{ //Gunsmoke goes in here
		mpBillboardList->RemoveBillboard(b);
	}
	/*
	if (!feather)
	{
		mpBillboardList->RemoveBillboard(b);
	}
	else
	{
		mpFeatherBillboardList->RemoveBillboard(b);
	}*/
}

void World::AddParticleSystem(ParticleSystem* particleSystem)
{
    mParticleSystemList.push_back(particleSystem);
}

void World::RemoveParticleSystem(ParticleSystem* particleSystem)
{
    vector<ParticleSystem*>::iterator it = std::find(mParticleSystemList.begin(), mParticleSystemList.end(), particleSystem);
    mParticleSystemList.erase(it);
}

void World::AddParticleDescriptor(ParticleDescriptor* particleDescriptor)
{
    mParticleDescriptorList.push_back(particleDescriptor);
}

vec3 World::GetLightColor() {
	return mpWorldLighting->GetSunlightColor();
}

vec3 World::GetBillboardLightColor() {
	return mpWorldLighting->GetBillboardLightColor();
}

vec4 World::GetBillboardLightPosition() {
	mat4 viewMatrix = GetCurrentCamera()->GetViewMatrix();
	vec3 camLookAt(viewMatrix[0][2], viewMatrix[1][2], viewMatrix[2][2]);

	return vec4(-camLookAt, 0.0f);
}

vec3 World::GetLightAttenuation() {
	return vec3(0.05f, 0.02, 0.002f);
}

vec4 World::GetLightPosition() {
	//return vec4(0.0f, 10.0f, 20.0f, 1.0f);
	return mpWorldLighting->GetSunlightVector();
}

float World::GetMinimumAmbient()
{
	return mpWorldLighting->GetAmbientLightingFloat();
}

vec4 World::GetGunLightPosition()
{
	return mpWorldLighting->GetGunLightingVector();
}

vec3 World::GetGunLightColor()
{
	return mpWorldLighting->GetGunLightingColor();
}

ParticleDescriptor* World::FindParticleDescriptor(ci_string name)
{
    for(std::vector<ParticleDescriptor*>::iterator it = mParticleDescriptorList.begin(); it < mParticleDescriptorList.end(); ++it)
    {
        if((*it)->GetName() == name)
        {
            return *it;
        }
    }
    return nullptr;
}

glm::mat4 World::GetInverseWorldMatrix()
{
	return mInverseViewMatrix;
}

bool World::IsShooting()
{
	return lastMouseState;
}
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
#include "BoneAnimation.h"
#include "Billboard.h"
#include <GLFW/glfw3.h>
#include "EventManager.h"
#include "TextureLoader.h"
#include "ObjModel.h"
#include "AnimatedObjModel.h"

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
	mSkybox = new SkyBox();


    
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
	glClearColor(GetDistanceFogColor().x, GetDistanceFogColor().y, GetDistanceFogColor().z, 1.0f);
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

	//Turn on or off collision sphere drawing
	if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_C) == GLFW_PRESS)
	{
		if (!collisionKeyHeld)
		{
			showCollisionShere = !showCollisionShere;
		}
		collisionKeyHeld = true;
	}
	else if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_C) == GLFW_RELEASE)
	{
		collisionKeyHeld = false;
	}

	//Turn on or off viewing the skybox
	if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_B) == GLFW_PRESS)
	{
		if (!skyboxKeyHeld)
		{
			showSkybox = !showSkybox;
		}
		skyboxKeyHeld = true;
	}
	else if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_B) == GLFW_RELEASE)
	{
		skyboxKeyHeld = false;
	}

	//Turn on or off making the fog nearer
	if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_F) == GLFW_PRESS)
	{
		if (!fogKeyHeld)
		{
			fogNear = !fogNear;
		}
		fogKeyHeld = true;
	}
	else if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_F) == GLFW_RELEASE)
	{
		fogKeyHeld = false;
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


	if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_7) == GLFW_PRESS)
	{
		AnimatedObjModel* duck = (AnimatedObjModel*)FindModel("duck");
		BoneAnimation* duckFlight = FindBoneAnimation("DuckFlight");
		duck->attachBoneAnimation(duckFlight);
	}
	else if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_8) == GLFW_PRESS)
	{
		AnimatedObjModel* duck = (AnimatedObjModel*)FindModel("duck");
		BoneAnimation* duckFall = FindBoneAnimation("DuckFall");
		duck->attachBoneAnimation(duckFall);
	}

	mInverseViewMatrix = glm::inverse(GetCurrentCamera()->GetViewMatrix());

	// shooting function
	clicked = false;
	if (lastMouseState == false && glfwGetMouseButton(EventManager::GetWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
		const float projectileSpeed = 75.0f;
		//mat4 viewMatrix = glm::inverse(GetCurrentCamera()->GetViewMatrix());
		vec3 camLookAt= -mInverseViewMatrix[2];
		vec3 cameraPosition = mInverseViewMatrix[3];
		//std::cout << camLookAt.x << " " << camLookAt.y << " " << camLookAt.z << "\n";
		//vec3 cameraPosition1 = cameraPosition * vec3(1.0f, 0.78f, 1.0f);
		vec3 cameraPosition1 = cameraPosition * vec3(1.0f, 0.9f, 1.0f) + camLookAt * 5.0f;
		Bullet *bt = new Bullet(cameraPosition1, projectileSpeed * camLookAt);
		bulletList.push_back(bt);
        //printf("I like trains");
        //cout << "clicked" << endl;
        lastMouseState = true;
		clicked = true;
		smokeShotTimer = 70;
		smokeShotDelay = 2;
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

	for (vector<BoneAnimation*>::iterator it = mBoneAnimation.begin(); it < mBoneAnimation.end(); ++it)
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
		if ((*it)->GetName() == "\"GunCube\"")
		{
			if (smokeShotTimer > 0 && smokeShotDelay == 0)
			{
				vec3 camLookAt = -mInverseViewMatrix[2];
				vec3 cameraPosition = mInverseViewMatrix[3];
				vec3 cameraPosition1 = cameraPosition * vec3(1.0f, 0.9f, 1.0f) + camLookAt * 5.0f;
				(*it)->SetPosition(cameraPosition1);
			}
			else
			{
				(*it)->SetPosition(vec3(1000.0f, 1000.0f, 1000.0f));
			}
		}
		else if ((*it)->GetName() == "\"DuckCube\"")
		{
			//Do nothing here
		}
		else
		{
			(*it)->Update(dt);
		}
	}
	if (smokeShotTimer > 0 && smokeShotDelay == 0)
	{
		smokeShotTimer -= 1;
	}
	else if (smokeShotDelay > 0)
	{
		smokeShotDelay -= 1;
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

	//Collision

	if (collisionTimer == 0)
	{
		duckFeatherPosition = vec3(1000.0f, 1000.0f, 1000.0f);
	}
	Bullet* clearBullet = NULL;
	for (vector<Animation*>::iterator it_d = mAnimation.begin(); it_d < mAnimation.end(); ++it_d)
	{
		for (list<Bullet*>::iterator it_b = bulletList.begin(); it_b != bulletList.end(); ++it_b)
		{
			float pyth = sqrt(pow((*it_b)->GetPosition().x - (*it_d)->GetPosition().x, 2)
				+ pow((*it_b)->GetPosition().y - (*it_d)->GetPosition().y, 2)
				+ pow((*it_b)->GetPosition().z - (*it_d)->GetPosition().z, 2));
			float bulletRadius = sqrt(pow((*it_b)->GetSize().x, 2) + pow((*it_b)->GetSize().y, 2) + pow((*it_b)->GetSize().z, 2));
			float duckRadius = sqrt(pow(1.0, 2) + pow(1.0, 2) + pow(1.0, 2));

			//cout << "Bullet: " << (*it_b)->GetPosition().x << ", " << (*it_b)->GetPosition().y << ", " << (*it_b)->GetPosition().z << endl;
			//cout << "Duck: " << (*it_d)->GetPosition().x << ", " << (*it_d)->GetPosition().y << ", " << (*it_d)->GetPosition().z << endl;
			//cout << pyth << endl;

			if (pyth < bulletRadius + duckRadius)
			{
				if (showCollisionShere)
				{
					printf("I like turtles\n");
				}
				duckFeatherPosition = vec3((*it_d)->GetPosition());
				collisionTimer = 10;
				clearBullet = (*it_b);
			}
		}
	}
	if (clearBullet != nullptr)
	{
		//printf("I also like turtles\n");
		bulletList.remove(clearBullet);
	}

	// Update models
	for (vector<Model*>::iterator it = mModel.begin(); it < mModel.end(); ++it)
	{
		if ((*it)->GetName() == "\"DuckCube\"")
		{
			(*it)->SetPosition(duckFeatherPosition);
		}
		else
		{
			//Do nothing here
		}
	}
	if (collisionTimer > 0)
	{
		collisionTimer -= 1;
	}
}

void World::Draw()
{
	Renderer::BeginFrame();

	//Shadows work starts here
	/*(Renderer::SetShader(SHADER_PHONG);
	glUseProgram(Renderer::GetShaderProgramID());

	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	// Depth texture. Slower than a depth buffer, but you can sample it later in your shader
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	GLuint depthTexture;
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	Renderer::CheckForErrors();

	GLuint VMatrixLocation_Light = glGetUniformLocation(Renderer::GetShaderProgramID(), "ViewTransform_Light");
	GLuint PMatrixLocation_Light = glGetUniformLocation(Renderer::GetShaderProgramID(), "ProjectonTransform_Light");
	glUniformMatrix4fv(VMatrixLocation_Light, 1, GL_FALSE, &mpWorldLighting->GetLightLookAt()[0][0]);
	glUniformMatrix4fv(PMatrixLocation_Light, 1, GL_FALSE, &mpWorldLighting->GetLightProjection()[0][0]);

	Renderer::CheckForErrors();*/

	//Shadows work ends here
	//Renderer::BeginFrame();
	
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
	GLuint BLPositionLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "BackLightPosition");
	GLuint GLPositionLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "GunLightPosition");
	GLuint GLColorLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "gunLightColor");
	GLuint DistanceFogColor = glGetUniformLocation(Renderer::GetShaderProgramID(), "distanceFogColor");
	GLuint DistanceFogDetails = glGetUniformLocation(Renderer::GetShaderProgramID(), "distanceFogDetails");

	// Light Coefficients
	const vec3 lightColor = GetLightColor();//GetGunLightColor();//GetLightColor();
	const vec3 lightAttenuation = GetLightAttenuation();
	const float minAmbient = GetMinimumAmbient();
	const vec4 lightPosition = GetLightPosition();//GetGunLightPosition();//GetLightPosition(); // If w = 1.0f, we have a point light
	//const vec4 lightPosition = vec4(5.0f, -5.0f, 5.0f, 0.0f); // If w = 0.0f, we have a directional light
	const vec4 backlightPosition = GetBackLightPosition();
	const vec3 gunLightColor = GetGunLightColor();
	const vec4 gunLightPosition = GetGunLightPosition();
	const vec3 distanceFogColor = GetDistanceFogColor();
	const vec3 distanceFogDetails = GetDistanceFogDetails();

	glUniform4f(WLPositionLocation, lightPosition.x, lightPosition.y, lightPosition.z, lightPosition.w);
	glUniform4f(LColorLocation, lightColor.r, lightColor.g, lightColor.b, minAmbient);
	glUniform3f(LAttenuationLocation, lightAttenuation.x, lightAttenuation.y, lightAttenuation.z);
	glUniform4f(BLPositionLocation, backlightPosition.x, backlightPosition.y, backlightPosition.z, backlightPosition.w);
	glUniform4f(GLPositionLocation, gunLightPosition.x, gunLightPosition.y, gunLightPosition.z, gunLightPosition.w);
	glUniform3f(GLColorLocation, gunLightColor.r, gunLightColor.g, gunLightColor.b);
	glUniform3f(DistanceFogColor, distanceFogColor.x, distanceFogColor.y, distanceFogColor.z);
	glUniform3f(DistanceFogDetails, distanceFogDetails.x, distanceFogDetails.y, distanceFogDetails.z);

	// Send the view projection constants to the shader
	mat4 VP = mCamera[mCurrentCamera]->GetViewProjectionMatrix();
	mat4 VMatrix = mCamera[mCurrentCamera]->GetViewMatrix();
	mat4 PMatrix = mCamera[mCurrentCamera]->GetProjectionMatrix();
	glUniformMatrix4fv(VPMatrixLocation, 1, GL_FALSE, &VP[0][0]);

	glUniformMatrix4fv(VMatrixLocation, 1, GL_FALSE, &mCamera[mCurrentCamera]->GetViewMatrix()[0][0]);
	glUniformMatrix4fv(PMatrixLocation, 1, GL_FALSE, &mCamera[mCurrentCamera]->GetProjectionMatrix()[0][0]);



	// Draw models
	for (vector<Model*>::iterator it = mModel.begin(); it < mModel.end(); ++it)
	{
		if (!showCollisionShere && (*it)->GetName() == "\"Sphere\"")
		{
			//printf("Life is like a box of chocolates\n");
		}
		else if (showSkybox && (*it)->GetName() == "ground")
		{

		}
		else if (!showCollisionShere && (*it)->GetName() == "\"GunCube\"")
		{

		}
		else if (!showCollisionShere && (*it)->GetName() == "\"DuckCube\"")
		{

		}
		else
		{
			(*it)->Draw();
		}
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


	// Draw Skybox at the end
	Renderer::SetShader(SKYBOX_SHADER);
	glUseProgram(Renderer::GetShaderProgramID());
	//Draw Skybox
	mSkybox->Draw(VMatrix, PMatrix);

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
			else if (result.size() > 6 && result.substr(0, 6) == "Object")
			{
				std::string name = std::string(result.substr(7).c_str());
				ObjModel* obj = new ObjModel("../Assets/Models/" + name + ".obj");
				obj->Load(iss);
				mModel.push_back(obj);
			}
			else if (result.size() > 14 && result.substr(0, 14) == "AnimatedObject")
			{
				std::string name = std::string(result.substr(15).c_str());
				AnimatedObjModel* obj = new AnimatedObjModel("../Assets/Models/" + name + ".obj");
				obj->Load(iss);
				mModel.push_back(obj);
			}
			else if (result == "BoneAnimation")
			{
				BoneAnimation* anim = new BoneAnimation();
				anim->Load(iss);
				mBoneAnimation.push_back(anim);
			}
			else if (result == "lighting")
			{
				//mpWorldLighting->Load(iss); //Make it not hard-coded!
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

vec4 World::GetBackLightPosition()
{
	return mpWorldLighting->GetBacklightVector();
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

vec3 World::GetDistanceFogColor()
{
	return mpWorldLighting->GetDistanceFogColor();
}

vec3 World::GetDistanceFogDetails()
{
	if (!fogNear)
	{
		return mpWorldLighting->GetDistanceFogDetails(); //Normal distance fog
	}
	else
	{
		return vec3(4.0f, 16.0f, 0.9f); //Hardcoded closer fog to show it off
	}
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
	return clicked;
}

BoneAnimation* World::FindBoneAnimation(ci_string animName)
{
	for (std::vector<BoneAnimation*>::iterator it = mBoneAnimation.begin(); it < mBoneAnimation.end(); ++it)
	{
		if ((*it)->GetName() == animName)
		{
			return *it;
		}
	}
	return nullptr;
}

Model* World::FindModel(ci_string name)
{
	for (std::vector<Model*>::iterator it = mModel.begin(); it < mModel.end(); ++it)
	{
		if ((*it)->GetName() == name)
		{
			return *it;
		}
	}
	return nullptr;
}
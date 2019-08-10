//
// COMP 371 Assignment Framework
//
// Created by Nicolas Bergeron on 15/7/15.
//         with help from Jordan Rutty
//
// Copyright (c) 2014-2019 Concordia University. All rights reserved.
//

#include "ParticleSystem.h"
#include "ParticleDescriptor.h"
#include "ParticleEmitter.h"
#include "EventManager.h"
#include "World.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/common.hpp>

using namespace glm;


ParticleSystem::ParticleSystem(ParticleEmitter* emitter, ParticleDescriptor* descriptor)
: mpDescriptor(descriptor), mpEmitter(emitter), timeSinceLastParticleEmitted(0.0f)
{
    assert(mpDescriptor != nullptr);
    assert(mpEmitter != nullptr);
    
    // Pre-allocate the maximum number of particles at a give time, according to
    // lifetime and emission rate
    int maxParticles = static_cast<int>(mpDescriptor->emissionRate * (mpDescriptor->totalLifetime + mpDescriptor->totalLifetimeRandomness)) + 1;
    
    mInactiveParticles.resize(maxParticles);
    for (std::list<Particle*>::iterator it = mInactiveParticles.begin(); it != mInactiveParticles.end(); ++it)
    {
        *it = new Particle();
    }
}

ParticleSystem::~ParticleSystem()
{
	for (std::list<Particle*>::iterator it = mParticleList.begin(); it != mParticleList.end(); ++it)
	{

			World::GetInstance()->RemoveBillboard(&(*it)->billboard, mpDescriptor->name);

	}

    for (std::list<Particle*>::iterator it = mInactiveParticles.begin(); it != mInactiveParticles.end(); ++it)
    {
        delete *it;
    }
    
    mParticleList.resize(0);
	mInactiveParticles.resize(0);
}

void ParticleSystem::Update(float dt)
{
    // Emit particle according to the emission rate
    float averageTimeBetweenEmission = 1.0f / mpDescriptor->emissionRate;
//    float randomValue = EventManager::GetRandomFloat(0.0f, averageTimeBetweenEmission);
    timeSinceLastParticleEmitted += dt;
    
    if (timeSinceLastParticleEmitted > averageTimeBetweenEmission && mInactiveParticles.size() > 0)
    {
        timeSinceLastParticleEmitted = 0.0f;
        
        
        // emit particle
        // transfer a particle from the inactive pool to the active pool
        Particle* newParticle = mInactiveParticles.back();
        mParticleList.push_back(newParticle);
        mInactiveParticles.pop_back();
		/*std::cout << mpDescriptor->name[0] << mpDescriptor->name[1] << mpDescriptor->name[2]
			<< mpDescriptor->name[3] << mpDescriptor->name[4] << mpDescriptor->name[5] << mpDescriptor->name[6] << std::endl;*/

			World::GetInstance()->AddBillboard(&newParticle->billboard, mpDescriptor->name);
        
        // Set particle initial parameters
			if (mpDescriptor->name == "Flake" || mpDescriptor->name == "Snow")
			{
				newParticle->billboard.position = vec3(EventManager::GetRandomFloat(-20.0f, 20.0f), 20.0f, EventManager::GetRandomFloat(-20.0f, 10.0f));

			}else newParticle->billboard.position = mpEmitter->GetPosition();
        
        newParticle->billboard.size = mpDescriptor->initialSize + EventManager::GetRandomFloat(-1.0f, 1.0f) * mpDescriptor->initialSizeRandomness;
        newParticle->billboard.color = mpDescriptor->initialColor;
        newParticle->currentTime = 0.0f;
        newParticle->lifeTime = mpDescriptor->totalLifetime + mpDescriptor->totalLifetimeRandomness * EventManager::GetRandomFloat(-1.0f, 1.0f);
        newParticle->velocity = mpDescriptor->velocity;

        newParticle->billboard.angle = mpDescriptor->initialRotationAngle + EventManager::GetRandomFloat(0.0f, mpDescriptor->initialRotationAngleRandomness);
        
        // @TODO 7 - Initial Random Particle Velocity vector
        //
        // Adjust the random velocity according to a random vertical angle variation on a cone
        //
        // Step 1 : You can rotate the velocity vector by a random number between 0 and
        //          mpDescriptor->velocityAngleRandomness.
        // Step 2 : You can rotate the result in step 1 by an random angle from 0 to
        //          360 degrees about the original velocity vector

        // ...

		vec3 tempVector = vec3(0.0f, 0.0f, 1.0f);
		vec3 rotateAxis = cross(newParticle->velocity, vec3(0.0f, 0.0f, 1.0f));
		if (length(rotateAxis) == 0.0f) {
			rotateAxis = cross(newParticle->velocity, vec3(1.0f, 0.0f, 0.0f));
		}
		mat4 firstRotate = rotate(mat4(1.0f), radians(EventManager::GetRandomFloat(0.0f, mpDescriptor->velocityAngleRandomness)), rotateAxis);
		mat4 secondRotate = rotate(mat4(1.0f), radians(EventManager::GetRandomFloat(0.0f, 360.0f)), normalize(newParticle->velocity));
		newParticle->velocity = vec3(secondRotate * firstRotate * vec4(newParticle->velocity, 0.0f));


    }
    
	if (mParticleList.size() > 0) {
		for (std::list<Particle*>::iterator it = mParticleList.begin(); it != mParticleList.end(); it++)
		{
			Particle* p = *it;
			p->currentTime += dt;
			p->billboard.position += p->velocity * dt;

			// @TODO 6 - Update each particle parameters
			//
			// Update the velocity of the particle from the acceleration in the descriptor
			// Update the size of the particle according to its growth
			// Update The color is also updated in 3 phases
			//
			//
			// Phase 1 - Initial: from t = [0, fadeInTime] - Linearly interpolate between initial color and mid color
			// Phase 2 - Mid:     from t = [fadeInTime, lifeTime - fadeOutTime] - color is mid color
			// Phase 3 - End:     from t = [lifeTime - fadeOutTime, lifeTime]

			// Velocity update
			p->velocity += dt * mpDescriptor->acceleration;

			// Size update
			p->billboard.size += dt * mpDescriptor->sizeGrowthVelocity;

			// Calculate color
			if (p->currentTime < mpDescriptor->fadeInTime) // Initial phase
			{
				p->billboard.color = mix(mpDescriptor->initialColor, mpDescriptor->midColor, p->currentTime / mpDescriptor->fadeInTime);
			}
			else if (p->currentTime < p->lifeTime - mpDescriptor->fadeOutTime) // Mid phase
			{
				p->billboard.color = mpDescriptor->midColor;
			}
			else // End phase
			{
				p->billboard.color = mix(mpDescriptor->midColor, mpDescriptor->endColor, 1 - (p->lifeTime - p->currentTime) / mpDescriptor->fadeOutTime);
			}

			// ...
			//p->billboard.color = vec4(1.0f, 1.0f, 1.0f, 1.0f); // wrong... check required implementation above
			// ...

			// Do not touch code below...

			// Particles are destroyed if expired
			// Move from the particle to inactive list
			// Remove the billboard from the world
			if (p->currentTime > p->lifeTime)
			{
				mInactiveParticles.push_back(*it);

				World::GetInstance()->RemoveBillboard(&(p->billboard), mpDescriptor->name);
				mParticleList.remove(*it++);
			}
		}
	}
}

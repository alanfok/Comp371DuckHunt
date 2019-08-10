//
// COMP 371 Assignment Framework
//
// Created by Ghislain Clermont
//

#pragma once

#include "ParsingHelper.h"
#include "Model.h"
#include <vector>

#include <glm/glm.hpp>

class BoneAnimation
{
public:

	BoneAnimation();
	virtual ~BoneAnimation();

	virtual void Update(float dt);
	void calculateAnimation();
	glm::mat4 GetAnimationWorldMatrix(std::vector<glm::vec3> bonePositions, std::vector<float> boneWeights);

	void Load(ci_istringstream& iss);
	ci_string GetName() const;

	void resetAnim();

protected:
	virtual bool ParseLine(const std::vector<ci_string> &token);

private:
	ci_string mName; // The model name is mainly for debugging

	float mCurrentTime;

	std::vector<glm::vec2> translateKeys; // First value is Y position, second value is time. Only supported for parent bone.
	std::vector<std::vector<glm::vec2>> rotateKeys; // First index is boneNumber, second index is keyFrame. First value is Z rotation, second value is time.
	std::vector<int> startFrames; // The start of the animation. The animation loops before.
	std::vector<int> duration; // The duration of the animation. The animation loops after it is done.
	int fps; // Amount of frames per second. Keys are set on a per-frame basis, rather than per-second basis.
	int currentBone; // Bone currently looked at during the parsing.
	int boneCount; // Total number of bones

	// Optimization variables to improve performance of GetAnimationWorldMatrix();
	float calculatedTime;
	std::vector<float> calculatedAngle;
	float calculatedPosition;

	glm::mat4 getBoneMatrix(float bonePosition, float angle) const;
};



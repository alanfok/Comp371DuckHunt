//
// COMP 371 Assignment Framework
//
// Created by Ghislain Clermont
//

#include "BoneAnimation.h"
#include "Renderer.h"
#include "World.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

using namespace glm;
using namespace std;

BoneAnimation::BoneAnimation()
	: mName(""), mCurrentTime(0.0f)
{
}

BoneAnimation::~BoneAnimation()
{
}

void BoneAnimation::Update(float dt)
{
	mCurrentTime += dt;
}


void BoneAnimation::Load(ci_istringstream& iss)
{
	ci_string line;

	// Parse model line by line
	while (std::getline(iss, line))
	{
		// Splitting line into tokens
		ci_istringstream strstr(line);
		istream_iterator<ci_string, char, ci_char_traits> it(strstr);
		istream_iterator<ci_string, char, ci_char_traits> end;
		vector<ci_string> token(it, end);

		if (ParseLine(token) == false)
		{
			fprintf(stderr, "Error loading scene file... token:  %s! (from BoneAnimation)", token[0].c_str());
			getchar();
			exit(-1);
		}
	}
}


bool BoneAnimation::ParseLine(const std::vector<ci_string> &token)
{
	/*for (int i = 0; i < token.size(); i++)
	{
		std::cout << token[i].c_str();
	}
	printf("\n");*/
	if (token.empty())
	{
		return true;
	}
	else if (token[0] == "name")
	{
		assert(token.size() > 2);
		assert(token[1] == "=");

		mName = token[2];
		return true;
	}
	else if (token[0] == "fps")
	{
		assert(token.size() > 2);
		assert(token[1] == "=");

		fps = (int)atoi(token[2].c_str());
		return true;
	}
	else if (token[0] == "boneCount") // The number of bones that are used in the model
	{
		assert(token.size() > 2);
		assert(token[1] == "=");

		boneCount = (int)atoi(token[2].c_str());
		for (int i = 0; i < boneCount; i++) // Initialize vectors.
		{
			rotateKeys.push_back(vector<vec2>());
			startFrames.push_back(99999);
			duration.push_back(0);
			calculatedAngle.push_back(0);
		}
		//std::cout << rotateKeys.size() << "\n";
		return true;
	}
	else if (token[0] == "bone") // Set the animation of the next bone
	{
		assert(token.size() > 2);
		assert(token[1] == "=");

		currentBone = (int)atoi(token[2].c_str());
		return true;
	}
	else if (token[0] == "translateKey") // Only supported for main bone, and on the Y axis.
	{
		assert(token.size() > 6);
		assert(token[1] == "position");
		assert(token[2] == "=");
		assert(token[4] == "time");
		assert(token[5] == "=");

		float position = (float)atof(token[3].c_str());
		float time = (float)atof(token[6].c_str());

		std::cout << "time: " << time << ", startFrames[0]: " << startFrames[0] << "\n";
		if (time < startFrames[0]) startFrames[0] = time;
		translateKeys.push_back(vec2(position, time));
		duration[0] = time - startFrames[0];
		
		return true;
	}
	else if (token[0] == "rotateKey") // Only supported for the Z axis rotation
	{
		assert(token.size() > 6);
		assert(token[1] == "angle");
		assert(token[2] == "=");
		assert(token[4] == "time");
		assert(token[5] == "=");

		float position = (float)atof(token[3].c_str());
		float time = (float)atof(token[6].c_str());

		if (time < startFrames[currentBone]) startFrames[currentBone] = time;
		rotateKeys[currentBone].push_back(vec2(position, time));
		duration[currentBone] = time - startFrames[currentBone];

		return true;
	}
	return false;
}

ci_string BoneAnimation::GetName() const
{
	return mName;
}

void BoneAnimation::resetAnim()
{
	mCurrentTime = 0.0f;
}

glm::mat4 BoneAnimation::GetAnimationWorldMatrix(std::vector<vec3> bonePositions, std::vector<float> boneWeights)
{
	mat4 worldMatrix(1.0f);
	for (int i = boneCount-1; i >= 0; i--) // Reverse order, as children bones must be rotated before parents
	{
		// Avoid unnecessary calculations if there's nothing to calculate;
		if (boneWeights[i] < 0.002f || rotateKeys[i].size() == 0) continue;

		
		float newAngle = calculatedAngle[i] * boneWeights[i];
		//std::cout << fullAngle << "\n";
		

		/*worldMatrix = translate(mat4(1.0f), bonePositions[i]) *
			          rotate(mat4(1.0f), radians(newAngle), vec3(0.0f, 0.0f, 1.0f)) *
			          translate(mat4(1.0f), -bonePositions[i]) * worldMatrix;*/
		worldMatrix = getBoneMatrix(bonePositions[i].x, newAngle) * worldMatrix;
	}

	// Repeat, but for Main's translate

	worldMatrix[3][1] += calculatedPosition;

	return worldMatrix;
}

mat4 BoneAnimation::getBoneMatrix(float bonePosition, float angle) const // Performance optimization, given the limitation chosen for my animations
{
	/*
	pos = p (translation on x axis only) 
	angle = a (rotation around z axis only)

	T(p)R(a)T(-p) =
	[ cos(a) -sin(a)  0  -cos(a)*p + p ]
	[ sin(a)  cos(a)  0   -sin(a)*p    ]
	[   0       0     1      0         ]
	[   0       0     0      1         ]
	*/
	mat4 matrix(1.0f);
	matrix[0][0] = matrix[1][1] = cos(radians(angle));
	matrix[0][1] = sin(radians(angle));
	matrix[1][0] = -matrix[0][1];
	matrix[3][0] = bonePosition * (-matrix[0][0] + 1);
	matrix[3][1] = bonePosition * matrix[1][0];
	return matrix;
}

void BoneAnimation::calculateAnimation() {
	if (mCurrentTime == calculatedTime) return;
	
	for (int i = boneCount - 1; i >= 0; i--) // Reverse order, as children bones must be rotated before parents
	{
		// Avoid unnecessary calculations if there's nothing to calculate;
		if (rotateKeys[i].size() == 0) continue;
		else if (rotateKeys[i].size() == 1) {
			calculatedAngle[i] = rotateKeys[i][0][0];
			continue;
		}

		float loopedTime = mCurrentTime * fps;
		if (duration[i] > 0)
		{
			while (loopedTime < startFrames[i]) loopedTime += duration[i];
			while (loopedTime > startFrames[i] + duration[i]) loopedTime -= duration[i];
		}
		
		int firstKey = 0;
		int secondKey = 0;
		for (uint j = 0; j < rotateKeys[i].size(); j++)
		{
			if (rotateKeys[i][j][1] > loopedTime)
			{
				secondKey = j;
				firstKey = j - 1;
				break;
			}
		}

		float interpolation = (loopedTime - rotateKeys[i][firstKey][1]) / (rotateKeys[i][secondKey][1] - rotateKeys[i][firstKey][1]);
		float fullAngle = (rotateKeys[i][firstKey][0] * (1 - interpolation) + rotateKeys[i][secondKey][0] * interpolation);

		calculatedAngle[i] = fullAngle;
	}

	// Repeat, but for Main's translate
	float loopedTime = mCurrentTime * fps;
	if (duration[0] > 0)
	{
		while (loopedTime < startFrames[0]) loopedTime += duration[0];
		while (loopedTime > startFrames[0] + duration[0]) loopedTime -= duration[0];
	}
	int firstKey = 0;
	int secondKey = 0;
	for (uint i = 0; i < translateKeys.size(); i++)
	{
		if (translateKeys[i][1] > loopedTime)
		{
			secondKey = i;
			firstKey = i - 1;
			break;
		}
	}

	//cout << "firstkey: " << firstKey << ", secondKey: " << secondKey << ", loopedTime: " << loopedTime << ", duration[0]: " << duration[0] << ", startFrames[0]: " << startFrames[0] << "\n";

	float interpolation = (loopedTime - translateKeys[firstKey][1]) / (translateKeys[secondKey][1] - translateKeys[firstKey][1]);
	float newTranslation = (translateKeys[firstKey][0] * (1 - interpolation) + translateKeys[secondKey][0] * interpolation);

	calculatedPosition = newTranslation;

	calculatedTime = mCurrentTime;
}
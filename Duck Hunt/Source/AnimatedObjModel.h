//
// COMP 371 Final Project
//
// Created by Ghislain Clermont
//

#pragma once

#include "ObjModel.h"
#include "BoneAnimation.h"

class AnimatedObjModel : public ObjModel
{
public:
	AnimatedObjModel(std::string fileName);
	virtual ~AnimatedObjModel(void);

	virtual void Update(float dt);
	virtual void Draw();

	void attachBoneAnimation(BoneAnimation* anim);
	void removeBoneAnimation();

protected:
	virtual bool ParseLine(const std::vector<ci_string> &token);
	bool createModel(const char * fileName);


private:
	// The vertex format could be different for different types of models
	std::vector<std::vector<float>> vertexWeights;
	std::vector<glm::vec3> bonePositions;
	BoneAnimation* boneAnimation;

	struct Vertex
	{
		glm::vec3 position;
		glm::vec2 uv;
		glm::vec3 normal;
	};
	std::vector<Vertex> vertexBuffer;
	std::vector<Vertex> animatedVertexBuffer;

	unsigned int mVAO;
	unsigned int mVBO;
	unsigned int numOfVertices;

	bool createWeights(std::string boneName);
	void updateVertex();
	//void sendVertexBuffer();
};



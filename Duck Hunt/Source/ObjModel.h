//
// COMP 371 Final Project
//
// Created by Ghislain Clermont
//

#pragma once

#include "Model.h"

class ObjModel : public Model
{
public:
	ObjModel(std::string fileName);
	virtual ~ObjModel(void);

	virtual void Update(float dt);
	virtual void Draw();

protected:
	virtual bool ParseLine(const std::vector<ci_string> &token);
	bool createModel(const char * fileName);


private:
	// The vertex format could be different for different types of models
	struct Vertex
	{
		glm::vec3 position;
		glm::vec2 uv;
		glm::vec3 normal;
	};
	std::vector<Vertex> vertexBuffer;

	unsigned int mVAO;
	unsigned int mVBO;
	unsigned int numOfVertices;
};



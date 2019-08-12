//
// COMP 371 Assignment Framework
//
// Created by Nicolas Bergeron on 8/7/14.
// Updated by Gary Chang on 14/1/15
//
// Copyright (c) 2014-2019 Concordia University. All rights reserved.
//

#include "AnimatedObjModel.h"
#include "World.h"
#include "Renderer.h"
#include <FreeImageIO.h>
#include <LibJPEG/jpeglib.h>

using namespace glm;

AnimatedObjModel::AnimatedObjModel(std::string fileName) : ObjModel(fileName)
{
	// Set the material coefficients.
	materialCoefficients = vec4(0.2f, 0.8f, 0.3f, 50.0f);

	boneAnimation = nullptr;

	createModel(fileName.c_str());

	animatedVertexBuffer = vertexBuffer;
	
	numOfVertices = vertexBuffer.size();

	glGenVertexArrays(1, &mVAO);
	glBindVertexArray(mVAO);

	glGenBuffers(1, &mVBO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size() * sizeof(Vertex), &vertexBuffer.front(), GL_DYNAMIC_DRAW);

	// 1st attribute buffer : vertex Positions
	glVertexAttribPointer(0,              // attribute. No particular reason for 0, but must match the layout in the shader.
		3,              // size
		GL_FLOAT,       // type
		GL_FALSE,       // normalized?
		sizeof(Vertex), // stride
		(void*)0        // array buffer offset
	);
	glEnableVertexAttribArray(0);

	// 2nd attribute buffer : vertex uv
	glVertexAttribPointer(3,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(void*)sizeof(vec3)    // UV is offseted by one vec3
	);
	glEnableVertexAttribArray(3);


	// 3rd attribute buffer : vertex normal
	glVertexAttribPointer(1,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(void*)(sizeof(vec3) + sizeof(vec2)) // Normal is offsetted by a vec3 and vec2
	);
	glEnableVertexAttribArray(1);
}

AnimatedObjModel::~AnimatedObjModel()
{
	glDeleteBuffers(1, &mVBO);
	glDeleteVertexArrays(1, &mVAO);
}


void AnimatedObjModel::Update(float dt)
{
	Model::Update(dt);
}

void AnimatedObjModel::Draw()
{
	// Draw the Vertex Buffer
	// Note this draws a Sphere
	// The Model View Projection transforms are computed in the Vertex Shader
	if (boneAnimation != nullptr) {
		updateVertex();
		//sendVertexBuffer();
	}

	glBindVertexArray(mVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, animatedVertexBuffer.size() * sizeof(Vertex), &animatedVertexBuffer.front(), GL_DYNAMIC_DRAW);

	GLuint WorldMatrixLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "WorldTransform");
	glUniformMatrix4fv(WorldMatrixLocation, 1, GL_FALSE, &GetWorldMatrix()[0][0]);

	GLuint MaterialLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "materialCoefficients");
	glUniform4f(MaterialLocation, materialCoefficients.x, materialCoefficients.y, materialCoefficients.z, materialCoefficients.w);

	// Draw the triangles !
	glDrawArrays(GL_TRIANGLES, 0, numOfVertices);
}

void AnimatedObjModel::attachBoneAnimation(BoneAnimation* anim)
{
	boneAnimation = anim;
	anim->resetAnim();
}

void AnimatedObjModel::removeBoneAnimation()
{
	boneAnimation = nullptr;
	animatedVertexBuffer = vertexBuffer;
}

bool AnimatedObjModel::ParseLine(const std::vector<ci_string> &token)
{
	if (token.empty())
	{
		return true;
	}
	else if (token.size() > 7 && token[0] == "bone") {
		assert(token[1] == "=");
		assert(token[3] == "position");
		assert(token[4] == "=");

		float x = static_cast<float>(atof(token[5].c_str()));
		float y = static_cast<float>(atof(token[6].c_str()));
		float z = static_cast<float>(atof(token[7].c_str()));
		bonePositions.push_back(vec3(x, y, z));

		std::string name = std::string(token[2].c_str());
		createWeights(name);
		return true;
	}
	else if (token.size() > 2 && token[0] == "boneAnimation")
	{
		assert(token[1] == "=");

		ci_string animName = token[2];
		attachBoneAnimation(World::GetInstance()->FindBoneAnimation(animName));

		return true;
	}
	else if (token.size() > 3 && token[0] == "boneParent")
	{
		assert(token[2] == "+=");

		int parentIndex = (int)atoi(token[1].c_str());
		int childIndex = (int)atoi(token[3].c_str());
		for (int i = 0; i < vertexWeights.size(); i++) {
			//std::cout << "parent: " << vertexWeights[i][parentIndex] << ", child: " << vertexWeights[i][childIndex];
			vertexWeights[i][parentIndex] += vertexWeights[i][childIndex];
			//std::cout << ", new Parent: " << vertexWeights[i][parentIndex] << "\n";
		}
		return true;
	}
	else
	{
		return Model::ParseLine(token);
	}
}

bool AnimatedObjModel::createModel(const char * fileName)
{
	std::vector<int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	FILE * file;
	file = fopen(fileName, "r");
	if (!file) {
		printf("Impossible to open the file ! Are you in the right path ?\n");
		printf(fileName);
		return false;
	}

	while (1) {

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			//uv.y = -uv.y; // This line caused me hours of headache.
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			int vertexIndex[3], uvIndex[3], normalIndex[3];
			bool uv = true;
			bool norm = true;
			char line[128];
			fgets(line, 128, file);
			//vertex, uv, norm
			int matches = sscanf(line, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				//vertex, norm
				matches = sscanf(line, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);
				if (matches != 6) {
					//vertex, uv
					matches = sscanf(line, "%d/%d %d/%d %d/%d\n", &vertexIndex[0], &uvIndex[0], &vertexIndex[1], &uvIndex[1], &vertexIndex[2], &uvIndex[2]);
					if (matches != 6) {
						//vertex
						matches = sscanf(line, "%d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);
						if (matches != 6) {
							printf("File can't be read by our simple parser. 'f' format expected: d/d/d d/d/d d/d/d || d/d d/d d/d || d//d d//d d//d\n");
							printf("Character at %ld", ftell(file));
							return false;
						}
						uv, norm = false;
					}
					else {
						norm = false;
					}
				}
				else {
					uv = false;
				}
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			if (norm) {
				normalIndices.push_back(normalIndex[0]);
				normalIndices.push_back(normalIndex[1]);
				normalIndices.push_back(normalIndex[2]);
			}
			if (uv) {
				uvIndices.push_back(uvIndex[0]);
				uvIndices.push_back(uvIndex[1]);
				uvIndices.push_back(uvIndex[2]);
			}
		}
		else {
			char clear[1000];
			fgets(clear, 1000, file);
		}

	}

	for (unsigned int i = 0; i < vertexIndices.size(); i++) {
		vec3 pos = temp_vertices[vertexIndices[i] - 1];
		vec2 uv = temp_uvs[uvIndices[i] - 1];
		vec3 normal = temp_normals[normalIndices[i] - 1];
		Vertex vertex = { pos, uv, normal };
		vertexBuffer.push_back(vertex);

		//std::cout << i << ": (" << pos.x << "," << pos.y << "," << pos.z << "), (" << uv.x << "," << uv.y << "), (" << normal.x << "," << normal.y << "," << normal.z << ")\n";
	}

	return true;
}

bool AnimatedObjModel::createWeights(std::string boneName) 
{
	std::string file = "../Assets/Textures/" + std::string(mName.c_str()) + "_" + boneName + ".jpg";
	//std::cout << file << "\n";
	FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(file.c_str());
	FIBITMAP* bmp = FreeImage_Load(fif, file.c_str());
	unsigned width = FreeImage_GetWidth(bmp);
	unsigned height = FreeImage_GetHeight(bmp);
	unsigned bpp = FreeImage_GetBPP(bmp);
	RGBQUAD color;

	unsigned int x = 0;
	unsigned int y = 0;

	for (unsigned int i = 0; i < vertexBuffer.size(); i++) {

		x = int(vertexBuffer[i].uv.x * width);
		y = int(vertexBuffer[i].uv.y * height);

		bool success = FreeImage_GetPixelColor(bmp, x, y, &color);
		float weight = float(color.rgbBlue)/255.0f;

		if (vertexWeights.size() <= i) {
			vertexWeights.push_back(std::vector<float>());
		}
		vertexWeights[i].push_back(weight);
		//std::cout << weight << "\n";
	}

	FreeImage_Unload(bmp);

	return true;
}

void AnimatedObjModel::updateVertex()
{
	if (!dead || deathDelay == 0)
	{
		boneAnimation->calculateAnimation(); // Precalculate the angle rotation of each bone of current frame.

		float noWeightMove = 0; // Y-translation for vertices that are not weighted to any bone except the Main one.
		bool noWeightCalculated = false;
		for (int i = 0; i < vertexBuffer.size(); i++) {
			Vertex vertex;
			bool nonWeighted = true;
			for (int j = 1; j < vertexWeights[i].size(); j++) {
				if (vertexWeights[i][j] > 0.002) {
					nonWeighted = false;
					break;
				}
			}
			if (nonWeighted) // Case: It does not have any weight on the wings.
			{
				if (!noWeightCalculated) {
					noWeightMove = boneAnimation->GetAnimationWorldMatrix(bonePositions, vertexWeights[i])[3][1];
					noWeightCalculated = true;
				}
				vertex.position = vec3(vertexBuffer[i].position.x, vertexBuffer[i].position.y + noWeightMove, vertexBuffer[i].position.z);
				vertex.normal = vertexBuffer[i].normal;
			}
			else {
				int identical = 0;
				if (i > 3 && vertexBuffer[i].position == vertexBuffer[i - 3].position) identical = -3;
				else if (i > 2 && vertexBuffer[i].position == vertexBuffer[i - 2].position) identical = -2;
				else if (i > 1 && vertexBuffer[i].position == vertexBuffer[i - 1].position) identical = -1;
				if (identical < 0) // Case: An identical vertex has already been calculated before.
				{
					vertex.position = animatedVertexBuffer[i + identical].position;
					if (vertexBuffer[i].normal == vertexBuffer[i + identical].normal) vertex.normal = animatedVertexBuffer[i + identical].normal;
					else {
						//vertex.normal = vec3(transpose(inverse(boneAnimationMatrix)) * vec4(vertexBuffer[i].normal, 0.0f));
						vertex.normal = animatedVertexBuffer[i+identical].normal;
					}
				}
				else // Case: Brand new vertex. Big chunk of the calculation
				{
					mat4 boneAnimationMatrix = boneAnimation->GetAnimationWorldMatrix(bonePositions, vertexWeights[i]);
					vertex.position = vec3(boneAnimationMatrix * vec4(vertexBuffer[i].position, 1.0f));
					//vertex.normal = vec3(transpose(inverse(boneAnimationMatrix)) * vec4(vertexBuffer[i].normal, 0.0f));
					vertex.normal = vertexBuffer[i].normal;
				}
			
			}
			vertex.uv = vertexBuffer[i].uv;
			animatedVertexBuffer[i] = vertex;
			//std::cout << "before: " << vertexBuffer[i].position.x << "," << vertexBuffer[i].position.y << "," << vertexBuffer[i].position.z << 
			//	        ", after: " << vertex.position.x << "," << vertex.position.y << "," << vertex.position.z << "\n";
		}
	}
}

//void AnimatedObjModel::sendVertexBuffer()
//{
//	glBufferData(GL_ARRAY_BUFFER, animatedVertexBuffer.size() * sizeof(Vertex), &animatedVertexBuffer.front(), GL_STREAM_DRAW);
//
//	// 1st attribute buffer : vertex Positions
//	glVertexAttribPointer(0,              // attribute. No particular reason for 0, but must match the layout in the shader.
//		3,              // size
//		GL_FLOAT,       // type
//		GL_FALSE,       // normalized?
//		sizeof(Vertex), // stride
//		(void*)0        // array buffer offset
//	);
//	glEnableVertexAttribArray(0);
//
//	// 2nd attribute buffer : vertex uv
//	glVertexAttribPointer(1,
//		2,
//		GL_FLOAT,
//		GL_FALSE,
//		sizeof(Vertex),
//		(void*)sizeof(vec3)    // UV is offseted by one vec3
//	);
//	glEnableVertexAttribArray(1);
//
//
//	// 3rd attribute buffer : vertex normal
//	glVertexAttribPointer(2,
//		3,
//		GL_FLOAT,
//		GL_FALSE,
//		sizeof(Vertex),
//		(void*)(sizeof(vec3) + sizeof(vec2)) // Normal is offsetted by a vec3 and vec2
//	);
//	glEnableVertexAttribArray(2);
//}

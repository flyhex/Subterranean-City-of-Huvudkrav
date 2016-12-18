#pragma once
#include "Resource.h"
#include "GraphicsCore.h"
class Skybox
{
private:
	void generateVertices();
public:
	vector<SimpleMeshVertex> vertices;
	Skybox();
	~Skybox();

	void draw();
};


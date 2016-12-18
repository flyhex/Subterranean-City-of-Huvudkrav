#pragma once
#include "Mesh3D.h"
class SimpleMesh3D
	: public Mesh3D
{
private:
	
public:
	SimpleMesh3D(XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale);
	~SimpleMesh3D();
	bool SimpleMesh3D::loadObj(const char *filePath);
	vector<SimpleMeshVertex>	vertices;
};


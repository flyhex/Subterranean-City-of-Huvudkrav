#pragma once
#include "mesh3d.h"
class NormalMesh3D :
	public Mesh3D
{
private:

public:
	NormalMesh3D(XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale);

	vector<MeshVertex> vertices;

	bool loadObj(const char *filePath);

	~NormalMesh3D();
};


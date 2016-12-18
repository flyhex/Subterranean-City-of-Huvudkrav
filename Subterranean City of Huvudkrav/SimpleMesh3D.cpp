#include "SimpleMesh3D.h"


SimpleMesh3D::SimpleMesh3D(XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale)
	: Mesh3D(pos,rot,scale)
{
	vertices		= vector<SimpleMeshVertex>();
}

bool SimpleMesh3D::loadObj(const char *filePath)
{
	if(!objLoader->LoadMesh(&vertices, &iGroups, &materials, filePath, &texturePaths))
		return false;

	int k;
	for(int i = texturePaths.size()-1; i >= 0; i--)
	{
		for(k = texturePaths.size()-1; k >= 0; k--)
		{
			if(texturePaths.at(i) == texturePaths.at(k) && i != k)
			{
				texturePaths.erase(texturePaths.begin() + i);
				i--;
			}
		}
	}
	return true;
}

SimpleMesh3D::~SimpleMesh3D()
{
}

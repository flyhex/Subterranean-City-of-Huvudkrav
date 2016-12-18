#include "NormalMesh3D.h"

// http://www.terathon.com/code/tangent.html
NormalMesh3D::NormalMesh3D(XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale)
	: Mesh3D(pos, rot, scale)
{
}
bool NormalMesh3D::loadObj(const char *filePath)
{
	vector<SimpleMeshVertex> tempVertices = vector<SimpleMeshVertex>();
	if( !objLoader->LoadMesh(&tempVertices, &iGroups, &materials, filePath, &texturePaths) )
		return false;

	vertices = vector<MeshVertex>(tempVertices.size());
	for(unsigned int i = 0; i < vertices.size(); i++)
	{
		vertices.at(i).pos = tempVertices.at(i).pos;
		vertices.at(i).normal = tempVertices.at(i).normal;
		vertices.at(i).texC = tempVertices.at(i).texC;
	}
	objLoader->generateTangents2(&vertices);
	return true;
}

NormalMesh3D::~NormalMesh3D()
{
}

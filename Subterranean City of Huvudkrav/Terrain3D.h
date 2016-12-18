#pragma once
#include "object3d.h"

class Terrain3D :
	public Object3D
{
private:
	int blendMapID;
	int texturesStartID, endID;
	void generateTerrain(const char* heightMap, XMINT2 size);
	XMINT2 size;
	XMINT2 tileSize;
public:
	Terrain3D(XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale,const char* heightMap, 
		int blendMapID, int texturesStartID, int endID, XMINT2 size);
	~Terrain3D();

	XMINT2 getSize()		{ return size; }
	XMINT2 getTileSize()	{ return tileSize; }

	vector<SimpleMeshVertex> vertices;
	vector<DWORD> indices;


};


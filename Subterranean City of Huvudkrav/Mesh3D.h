#pragma once
#include "object3d.h"
#include "ObjLoader.h"
#include "GraphicsCore.h"

class Mesh3D :
	public Object3D
{
protected:
	ObjLoader			*objLoader;

	vector<Material>	materials;
	vector<IndexGroup>	iGroups;
	vector<string>		texturePaths;
	vector<ID3D11ShaderResourceView*>	textures;
public:
	
	Mesh3D(XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale);
	virtual bool loadObj(const char *filePath) = 0;
	void loadTextures();
	void updateCbObjects(CBObjects *CBObjects, int groupIndex);
	void updateShaderTextures(int groupIndex);
	unsigned int getNoGroups() { return iGroups.size(); }
	unsigned int getGroupStart(unsigned int groupIndex)		{ return iGroups.at(groupIndex).iStart; }
	unsigned int getGroupEnd(unsigned int groupIndex)		{ return iGroups.at(groupIndex).iEnd; }
	unsigned int getGroupSize(unsigned int groupIndex)		{ return iGroups.at(groupIndex).iEnd - iGroups.at(groupIndex).iStart; }
	Material	*getGroupMaterial(unsigned int groupIndex)	{ return iGroups.at(groupIndex).material; }
	~Mesh3D();
};


#include "Mesh3D.h"


Mesh3D::Mesh3D(XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale)
	:Object3D(pos,rot,scale)
{
	objLoader		= ObjLoader::getInstance();

	
	iGroups			= vector<IndexGroup>();
	materials		= vector<Material>();
	texturePaths	= vector<string>();
}


void Mesh3D::loadTextures()
{
	for(unsigned int i = 0; i < texturePaths.size();i++)
	{
		textures.push_back(NULL);
		HRESULT hr = D3DX11CreateShaderResourceViewFromFile(GraphicsCore::getInstance()->getDevice(),texturePaths.at(i).c_str(),NULL,NULL,&textures.at(i),NULL);
		if( FAILED( hr ) )
		{
			//MessageBox( NULL, "Error creating shaderresourceview","GraphicsCore Error", S_OK);
			cout << "Error creating shaderresourceview: "<< texturePaths.at(i).c_str() << endl;
		}
	}
}
void Mesh3D::updateCbObjects(CBObjects *cb, int groupIndex)
{
	GraphicsCore *g = GraphicsCore::getInstance();
	cb->illum	= iGroups.at(groupIndex).material->illum;
	cb->ka		= XMLoadFloat3(&iGroups.at(groupIndex).material->ka);
	cb->kd		= XMLoadFloat3(&iGroups.at(groupIndex).material->kd);
	cb->ke		= XMLoadFloat3(&iGroups.at(groupIndex).material->ke);
	cb->ks		= XMLoadFloat3(&iGroups.at(groupIndex).material->ks);
	cb->ni		= iGroups.at(groupIndex).material->ni;
	cb->ns		= iGroups.at(groupIndex).material->ns;
	cb->tf		= XMLoadFloat3(&iGroups.at(groupIndex).material->tf);
	cb->tr		= iGroups.at(groupIndex).material->tr;
	g->immediateContext->UpdateSubresource( g->cbObjects, 0, NULL, cb, 0, 0 );
}

void Mesh3D::updateShaderTextures(int groupIndex)
{
	GraphicsCore *g = GraphicsCore::getInstance();
	if(iGroups.at(groupIndex).material->map_ka >= 0)
		g->immediateContext->PSSetShaderResources(0,1,&textures.at(iGroups.at(groupIndex).material->map_ka));
	if(iGroups.at(groupIndex).material->map_kd >= 0)
		g->immediateContext->PSSetShaderResources(1,1,&textures.at(iGroups.at(groupIndex).material->map_kd));
	if(iGroups.at(groupIndex).material->map_ks >= 0)
		g->immediateContext->PSSetShaderResources(2,1,&textures.at(iGroups.at(groupIndex).material->map_ks));
	if(iGroups.at(groupIndex).material->map_ke >= 0)
		g->immediateContext->PSSetShaderResources(3,1,&textures.at(iGroups.at(groupIndex).material->map_ke));
	if(iGroups.at(groupIndex).material->map_ns >= 0)
		g->immediateContext->PSSetShaderResources(4,1,&textures.at(iGroups.at(groupIndex).material->map_ns));
	if(iGroups.at(groupIndex).material->map_tr >= 0)
		g->immediateContext->PSSetShaderResources(5,1,&textures.at(iGroups.at(groupIndex).material->map_tr));
	if(iGroups.at(groupIndex).material->map_decal >= 0)
		g->immediateContext->PSSetShaderResources(6,1,&textures.at(iGroups.at(groupIndex).material->map_decal));
	if(iGroups.at(groupIndex).material->map_disp >= 0)
		g->immediateContext->PSSetShaderResources(7,1,&textures.at(iGroups.at(groupIndex).material->map_disp));
}

Mesh3D::~Mesh3D()
{
	Object3D::~Object3D();
}

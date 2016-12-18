#include "Terrain3D.h"
#include "GraphicsCore.h"
#include "HeightMap.h"

Terrain3D::Terrain3D(	XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale,const char* heightMap, 
						int blendMapID, int texturesStartID, int endID, XMINT2 size)
	:Object3D(pos,rot,scale)
{
	this->blendMapID =	blendMapID;
	this->texturesStartID =	texturesStartID;
	this->endID =	endID;
	this->size = size;
	tileSize = XMINT2(20,20);
	generateTerrain(heightMap, size);
}

void Terrain3D::generateTerrain(const char* heightMap, XMINT2 size)
{
	HeightMap *hm = new HeightMap();
	hm->loadRaw((int)size.x,(int)size.y, heightMap,4.0f,-200);
	vector<float> heights = hm->getHeights();
	SAFE_DELETE(hm);

	vertices = vector<SimpleMeshVertex>((int)(size.x * size.y));

	float width = (size.x -1) * 2.0f;
	float depth = (size.y -1) * 2.0f;

	CBTerrain cb;
	cb.terrainSize = size;
	cb.terrainTileSize = tileSize;
	GraphicsCore::getInstance()->immediateContext->UpdateSubresource( GraphicsCore::getInstance()->cbTerrain,0,NULL,&cb,0,0);

	int k = 0;
	for( int y = 0; y < (int)size.y; y++ )
	{
		for( int x = 0; x < (int)size.x; x++)
		{
			k = y * (int)size.x + x;
			vertices.at(k).pos = XMFLOAT3((float)x * tileSize.x, heights.at(k), (float)y * tileSize.y);
			vertices.at(k).texC.x = (vertices.at(k).pos.x + (0.5f * width)) / width;
			vertices.at(k).texC.y = (vertices.at(k).pos.z - (0.5f * depth)) / -depth;
		}
	}
	bool perface = false;
	//temporary normals
	vector<XMVECTOR> normals;
	if(!perface)
		normals = vector<XMVECTOR>( ((size.x-1)*2) * (size.y-1) );
	else
		normals = vector<XMVECTOR>( ((size.x) * (size.y)) );
	int index = 0;
	for(int y = 0; y < size.y-1; y++)
	{
		for(int x = 0; x < size.x-1; x++)
		{
			k = y * size.x + x;
			XMVECTOR vec1 = XMLoadFloat3( &vertices.at(k + size.x		).pos) -
							XMLoadFloat3( &vertices.at(k 				).pos );
			XMVECTOR vec2 = XMLoadFloat3( &vertices.at(k + size.x + 1	).pos) -
							XMLoadFloat3( &vertices.at(k				).pos);

			if(!perface)
			{
				normals.at(index++) =  XMVector3Cross(vec1,vec2);
			}
			else
			{
				XMVECTOR normal =  XMVector3Cross(vec1,vec2);

				normals.at(k 				) += normal;
				normals.at(k + size.x		) += normal;
				normals.at(k + size.x + 1	) += normal;
			}
			vec1 =	XMLoadFloat3( &vertices.at(k + 1	).pos) -
					XMLoadFloat3( &vertices.at(k).pos	);

			if(!perface)
			{
				normals.at(index++) =  XMVector3Cross(vec2,vec1);
			}
			else
			{
				XMVECTOR normal =  XMVector3Cross(vec2,vec1);
				normals.at(k 				) += normal;
				normals.at(k + 1			) += normal;
				normals.at(k + size.x + 1	) += normal;
			}
			//XMStoreFloat3(&vertices.at(k).normal, normals.at(index-1));
		}
	}

	for(int y = 0; y < size.y; y++)
	{
		for(int x = 0; x < size.x; x++)
		{
			
			k = y * size.x + x;
			if(perface)
				XMStoreFloat3(&vertices.at(k).normal, XMVector3Normalize( normals.at(k)));
			else
			{
				XMVECTOR normalsum = XMVectorSet(0,0,0,0);
				int count = 0;
				if( x > 0 && y > 0 )
				{
					index = (y-1) * (size.x-1)*2 + ( x*2 - 2);
					normalsum += normals.at( index );		// the 2 faces in the quad
					normalsum += normals.at( index + 1 );    // in the bottom left quad
				}
				if( x > 0 && y < size.y - 1 )
				{
					index = (y) * (size.x-1)*2 + ( x*2 - 2);
					normalsum += 2*normals.at( index + 1 );		// the 1 face in the top left
												// quad that touch this vertex
				}
				if( x < size.x -1 && y < size.y - 1 )
				{
					index = (y) * (size.x-1)*2 + ( x*2 );
					normalsum += normals.at( index );		// the 2 faces in the quad
					normalsum += normals.at( index + 1 );	// in the top right quad
				}
				if( x < size.x -1 && y > 0 )
				{
					index = (y-1) * (size.x-1)*2 + ( x*2 );	//the 1 face in the bottom
					normalsum += 2 * normals.at( index );		//right quad that touch this vertex
				}
				//normalsum /= (float)count;
				XMStoreFloat3(&vertices.at(k).normal, XMVector3Normalize(normalsum)); 
			}
		}
	}
	for(int y = 0; y < size.y-1; y++)
	{
		for( int x = 0; x < size.x-1; x++)
		{
			k = y * size.x + x;
			indices.push_back(k);
			indices.push_back(k + size.x );
			indices.push_back(k + size.x +1);

			indices.push_back(k);
			indices.push_back(k + size.x +1);
			indices.push_back(k+1);
		}
	}


}

Terrain3D::~Terrain3D(void)
{
}

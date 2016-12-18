#pragma once
#include <d3d11.h>
#include <D3DX11.h>
#include <stdlib.h>
#include <crtdbg.h>
#include <xnamath.h>
#include <vector>
#include <string>

using namespace std;
//memory leak detection
#define _CRTDBG_MAP_ALLOC

#define DEFAULTSCRNWIDTH (1280)
#define DEFAULTSCRNHEIGHT (720)
#define NORENDERTARGETS (4)

#define	SAFE_RELEASE(x)	if( x ) { (x)->Release();	(x) = NULL; }
#define SAFE_DELETE(x)	if( x ) { delete(x);		(x) = NULL; }
struct XMINT2
{
	int x;
	int y;
	XMINT2()
	{
		x = 0;
		y = 0;
	}
	XMINT2(int x, int y)
	{
		this->x = x;
		this->y = y;
	}
};
struct SimpleMeshVertex
{
	XMFLOAT3 pos;
	XMFLOAT3 normal;
	XMFLOAT2 texC;

	SimpleMeshVertex(XMFLOAT3 pos, XMFLOAT3 normal, XMFLOAT2 texC)
	{
		this->pos = pos;
		this->normal = normal;
		this->texC = texC;
	}
	SimpleMeshVertex()
	{
		pos = XMFLOAT3(0,0,0);
		normal = XMFLOAT3(0,0,0);
		texC = XMFLOAT2(0,0);
	}
};
struct WaterVertex
{
	XMFLOAT3 pos;
	XMFLOAT2 texC;

	WaterVertex(XMFLOAT3 pos, XMFLOAT2 texC)
	{
		this->pos = pos;
		this->texC = texC;
	}
	WaterVertex()
	{
		this->pos = XMFLOAT3(0,0,0);
		this->texC = XMFLOAT2(0,0);
	}
};
struct PerInstanceData
{
	XMFLOAT3 instPos;
	float instRot;
};
struct MeshVertex
{
	XMFLOAT3 pos;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;
	XMFLOAT3 biTangent;
	XMFLOAT2 texC;

	MeshVertex(XMFLOAT3 pos, XMFLOAT3 normal, XMFLOAT2 texC,XMFLOAT3 tangent,XMFLOAT3 biTangent)
	{
		this->pos = pos;
		this->normal = normal;
		this->tangent = tangent;
		this->biTangent	= biTangent;
		this->texC = texC;
	}
	MeshVertex()
	{
		pos = XMFLOAT3(0,0,0);
		normal = XMFLOAT3(0,0,0);
		texC = XMFLOAT2(0,0);
		tangent = XMFLOAT3(0,0,0);
		biTangent = XMFLOAT3(0,0,0);
	}
};

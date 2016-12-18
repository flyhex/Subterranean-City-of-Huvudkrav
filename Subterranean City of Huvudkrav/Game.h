#pragma once
#include "GraphicsCore.h"
#include "Camera.h"
#include <iostream>
#include "Input.h"
#include "SimpleMesh3D.h"
#include "NormalMesh3D.h"
#include "Terrain3D.h"
#include "Skybox.h"
#include "Water.h"
class Game
{
private:
	CBUserOptions		cbUserOptions;
	CBObjects			cbObjects;

	//the second buffer in this array is for positioning of instanced trees at the moment
	ID3D11Buffer		*staticVBufferSimple[2]; 
	ID3D11Buffer		*staticIBufferSimple;

	ID3D11Buffer		*staticVBuffer;

	Camera				*camera;
	Input				*input;

	NormalMesh3D		*sword;
	SimpleMesh3D		*tree;

	vector<ID3D11ShaderResourceView*> textures;

	Terrain3D			*terrain;
	Skybox				*skybox;
	Water				*water;
	
	int					staticVBufferSimpleCount;
	bool				initBuffers();
	bool				initStaticObj();

	double				timeStep;
	double				gameTime;
	double				timeSinceUpdate;
	int					fps;

	bool				tessellate;

	ID3D11ShaderResourceView *testTexture;
public:
	Game();

	void				draw();
	void				update(double dt);
	~Game();
};


#include "Skybox.h"


Skybox::Skybox()
{
	generateVertices();
}
void Skybox::generateVertices()
{
	//front of the cube
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(5.0f,-5.0f,-5.0f), XMFLOAT3(0.0f,0.0f,-1.0f),XMFLOAT2(0.499f,0.332f)));
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(-5.0f,5.0f,-5.0f), XMFLOAT3(0.0f,0.0f,-1.0f),XMFLOAT2(0.0f,0.0f)));
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(5.0f,5.0f,-5.0f), XMFLOAT3(0.0f,0.0f,-1.0f),XMFLOAT2(0.499f,0.0f)));

	vertices.push_back(SimpleMeshVertex( XMFLOAT3(5.0f,-5.0f,-5.0f), XMFLOAT3(0.0f,0.0f,-1.0f),XMFLOAT2(0.499f,0.332f)));
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(-5.0f,-5.0f,-5.0f), XMFLOAT3(0.0f,0.0f,-1.0f),XMFLOAT2(0.0f,0.332f)));
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(-5.0f,5.0f,-5.0f), XMFLOAT3(0.0f,0.0f,-1.0f),XMFLOAT2(0.0f,0.0f)));

	//right side of the cube
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(5.0f,-5.0f,5.0f), XMFLOAT3(1.0f,0.0f,0.0f),XMFLOAT2(1.0f,0.332f)));
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(5.0f,5.0f,-5.0f), XMFLOAT3(1.0f,0.0f,0.0f),XMFLOAT2(0.501f,0.0f)));
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(5.0f,5.0f,5.0f), XMFLOAT3(1.0f,0.0f,0.0f),XMFLOAT2(1.0f,0.0f)));
	//här
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(5.0f,-5.0f,5.0f), XMFLOAT3(1.0f,0.0f,0.0f),XMFLOAT2(1.0f,0.332f)));
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(5.0f,-5.0f,-5.0f), XMFLOAT3(1.0f,0.0f,0.0f),XMFLOAT2(0.501f,0.332f)));
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(5.0f,5.0f,-5.0f), XMFLOAT3(1.0f,0.0f,0.0f),XMFLOAT2(0.501f,0.0f)));

	//back of the cube
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(-5.0f,-5.0f,5.0f), XMFLOAT3(0.0f,0.0f,1.0f),XMFLOAT2(0.499f,0.666f)));
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(5.0f,5.0f,5.0f), XMFLOAT3(0.0f,0.0f,1.0f),XMFLOAT2(0.0f,0.334f)));
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(-5.0f,5.0f,5.0f), XMFLOAT3(0.0f,0.0f,1.0f),XMFLOAT2(0.499f,0.334f)));

	vertices.push_back(SimpleMeshVertex( XMFLOAT3(-5.0f,-5.0f,5.0f), XMFLOAT3(0.0f,0.0f,1.0f),XMFLOAT2(0.499f,0.666f)));
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(5.0f,-5.0f,5.0f), XMFLOAT3(0.0f,0.0f,1.0f),XMFLOAT2(0.0f,0.666f)));
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(5.0f,5.0f,5.0f), XMFLOAT3(0.0f,0.0f,1.0f),XMFLOAT2(0.0f,0.334f)));

	//left side of the cube
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(-5.0f,-5.0f,-5.0f), XMFLOAT3(-1.0f,0.0f,0.0f),XMFLOAT2(1.0f,0.666f)));
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(-5.0f,5.0f,5.0f), XMFLOAT3(-1.0f,0.0f,0.0f),XMFLOAT2(0.501f,0.334f)));
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(-5.0f,5.0f,-5.0f), XMFLOAT3(-1.0f,0.0f,0.0f),XMFLOAT2(1.0f,0.334f)));

	vertices.push_back(SimpleMeshVertex( XMFLOAT3(-5.0f,-5.0f,-5.0f), XMFLOAT3(-1.0f,0.0f,0.0f),XMFLOAT2(1.0f,0.666f)));
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(-5.0f,-5.0f,5.0f), XMFLOAT3(-1.0f,0.0f,0.0f),XMFLOAT2(0.501f,0.666f)));
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(-5.0f,5.0f,5.0f), XMFLOAT3(-1.0f,0.0f,0.0f),XMFLOAT2(0.501f,0.334f)));

	//top of the cube
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(-5.0f,5.0f,5.0f), XMFLOAT3(0.0f,1.0f,0.0f),XMFLOAT2(0.0f,0.668f)));
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(5.0f,5.0f,-5.0f), XMFLOAT3(0.0f,1.0f,0.0f),XMFLOAT2(0.499f,1.0f)));
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(-5.0f,5.0f,-5.0f), XMFLOAT3(0.0f,-.0f,0.0f),XMFLOAT2(0.0f,1.0f)));

	vertices.push_back(SimpleMeshVertex( XMFLOAT3(-5.0f,5.0f,5.0f), XMFLOAT3(0.0f,1.0f,0.0f),XMFLOAT2(0.0f,0.668f)));
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(5.0f,5.0f,5.0f), XMFLOAT3(0.0f,1.0f,0.0f),XMFLOAT2(0.499f,0.668f)));
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(5.0f,5.0f,-5.0f), XMFLOAT3(0.0f,1.0f,0.0f),XMFLOAT2(0.499f,1.0f)));

	//bottom of the cube
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(-5.0f,-5.0f,-5.0f), XMFLOAT3(0.0f,-1.0f,0.0f),XMFLOAT2(0.501f,0.668f)));
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(5.0f,-5.0f,5.0f), XMFLOAT3(0.0f,-1.0f,0.0f),XMFLOAT2(1.0f,1.0f)));
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(-5.0f,-5.0f,5.0f), XMFLOAT3(0.0f,-1.0f,0.0f),XMFLOAT2(0.501f,1.0f)));

	vertices.push_back(SimpleMeshVertex( XMFLOAT3(-5.0f,-5.0f,-5.0f), XMFLOAT3(0.0f,-1.0f,0.0f),XMFLOAT2(0.501f,0.668f)));
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(5.0f,-5.0f,-5.0f), XMFLOAT3(0.0f,-1.0f,0.0f),XMFLOAT2(1.0f,0.668f)));
	vertices.push_back(SimpleMeshVertex( XMFLOAT3(5.0f,-5.0f,5.0f), XMFLOAT3(0.0f,-1.0f,0.0f),XMFLOAT2(1.0f,1.0f)));
}

Skybox::~Skybox()
{
}

#include "Game.h"


Game::Game()
{
	initStaticObj();
	initBuffers();
	camera = new Camera(500,5,500);
	gameTime = 0;
	timeStep = 0;
	timeSinceUpdate = 0;
	input = Input::getInstance();
	tessellate = false;
}
bool Game::initStaticObj()
{
	GraphicsCore *g = GraphicsCore::getInstance();
	D3DX11CreateShaderResourceViewFromFile(GraphicsCore::getInstance()->getDevice(),"Terrain/Textures/stone.jpg",NULL,NULL,&testTexture,NULL);

	staticVBufferSimpleCount = 36;
	XMINT2 terrainSize = XMINT2(256, 256);

	skybox	= new Skybox();
	sword	= new NormalMesh3D( XMFLOAT3(0,0,0), XMFLOAT3(0,0,0), XMFLOAT3(1000,1000,1000) );
	tree	= new SimpleMesh3D( XMFLOAT3(0,0,0), XMFLOAT3(0,0,0), XMFLOAT3(1,1,1) );
	terrain	= new Terrain3D(XMFLOAT3(0,-0.005f,0),XMFLOAT3(0,0,0),XMFLOAT3(1,1,1),
		"Terrain/Heightmaps/heightmap2.raw",0,1,4,terrainSize);
	water	= new Water();

	tree->loadObj("Models/deadtree/deadtree.obj");
	sword->loadObj("Models/rebellion/rebellion.obj");
	
	//textures
	sword->loadTextures();
	tree->loadTextures();
	textures = vector<ID3D11ShaderResourceView*>(13);
	D3DX11CreateShaderResourceViewFromFile(g->getDevice(),
		"Terrain/Blendmaps/blendmap1.png",NULL,NULL,&textures[0],NULL);
	D3DX11CreateShaderResourceViewFromFile(g->getDevice(),
		"Terrain/Textures/stone.jpg",NULL,NULL,&textures[1],NULL);
	D3DX11CreateShaderResourceViewFromFile(g->getDevice(),
		"Terrain/Textures/sand.jpg",NULL,NULL,&textures[2],NULL);
	D3DX11CreateShaderResourceViewFromFile(g->getDevice(),
		"Terrain/Textures/ground.jpg",NULL,NULL,&textures[3],NULL);
	D3DX11CreateShaderResourceViewFromFile(g->getDevice(),
		"Models/rebellion/t_dante_weapons_rebellion_s.png",NULL,NULL,&textures[4],NULL);
	D3DX11CreateShaderResourceViewFromFile(g->getDevice(),
		"Skybox/skybox3/combined.png",NULL,NULL,&textures[5],NULL);
	D3DX11CreateShaderResourceViewFromFile(g->getDevice(),
		"Water/heightmap.png",NULL,NULL,&textures[6],NULL);
	D3DX11CreateShaderResourceViewFromFile(g->getDevice(),
		"Water/normalmap.jpg",NULL,NULL,&textures[7],NULL);
	D3DX11CreateShaderResourceViewFromFile(g->getDevice(),
		"Water/foam.jpg",NULL,NULL,&textures[8],NULL);
	D3DX11CreateShaderResourceViewFromFile(g->getDevice(),
		"Water/cirno.jpg",NULL,NULL,&textures[9],NULL);
	D3DX11CreateShaderResourceViewFromFile(g->getDevice(),
		"Terrain/rockheight.png",NULL,NULL,&textures[10],NULL);
	D3DX11CreateShaderResourceViewFromFile(g->getDevice(),
		"Terrain/rockheight2.png",NULL,NULL,&textures[11],NULL);
	D3DX11CreateShaderResourceViewFromFile(g->getDevice(),
		"Terrain/rockheight3.png",NULL,NULL,&textures[12],NULL);
	
	return true;
}
bool Game::initBuffers()
{
	staticVBufferSimple[1] = NULL;
	vector<SimpleMeshVertex> simpleMeshVertices = terrain->vertices;
	vector<MeshVertex> meshVertices = sword->vertices;
	//merged.insert( merged.end(),sword->vertices.begin(),sword->vertices.end() );
	simpleMeshVertices.insert( simpleMeshVertices.end(),skybox->vertices.begin(),skybox->vertices.end() );
	simpleMeshVertices.insert( simpleMeshVertices.end(),tree->vertices.begin(),tree->vertices.end() );

	D3D11_BUFFER_DESC bd;
	ZeroMemory( &bd, sizeof(bd) );
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof( SimpleMeshVertex ) * simpleMeshVertices.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory( &initData, sizeof( initData ) );
	initData.pSysMem = &simpleMeshVertices[0];
	if(!GraphicsCore::getInstance()->createVBuffer(&bd, &initData, &staticVBufferSimple[0] ) )
		return false;

	bd.ByteWidth = sizeof( MeshVertex ) * meshVertices.size();

	ZeroMemory( &initData, sizeof( initData ) );
	initData.pSysMem = &meshVertices[0];
	if(!GraphicsCore::getInstance()->createVBuffer(&bd, &initData, &staticVBuffer ) )
		return false;

	vector<PerInstanceData> perInstanceData = vector<PerInstanceData>(5);
	perInstanceData.at(0).instPos = XMFLOAT3(2000,741,1900);
	perInstanceData.at(0).instRot = 3.14;

	perInstanceData.at(1).instPos = XMFLOAT3(1500,-20,3900);
	perInstanceData.at(1).instRot = 2*3.14;

	perInstanceData.at(2).instPos = XMFLOAT3(2670,90,3480);
	perInstanceData.at(2).instRot = 1.34;

	perInstanceData.at(3).instPos = XMFLOAT3(2300,42,3790);
	perInstanceData.at(3).instRot = 0;

	perInstanceData.at(4).instPos = XMFLOAT3(1400,55,3000);
	perInstanceData.at(4).instRot = 0.75;

	bd.ByteWidth = sizeof( PerInstanceData ) * 5;
	initData.pSysMem = &perInstanceData[0];
	if(!GraphicsCore::getInstance()->createVBuffer(&bd, &initData, &staticVBufferSimple[1] ) )
		return false;

	//static index buffer
	bd.ByteWidth = sizeof(DWORD) * terrain->indices.size();
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	ZeroMemory( &initData, sizeof( initData ) );
	initData.pSysMem = &terrain->indices[0];
	if(!GraphicsCore::getInstance()->createVBuffer(&bd, &initData, &staticIBufferSimple ) )
		return false;
	return true;
}
void Game::update(double dt)
{
	GraphicsCore* g = GraphicsCore::getInstance();

	timeStep = dt;
	gameTime += dt;
	timeSinceUpdate += dt;
	if(timeSinceUpdate >= 1)
	{
		timeSinceUpdate--;
		fps = (int)(1.0 / dt + 0.5);
	}
	//cout << fps << "\n";

	//toggle render target view mode
	if(input->getKeyState()[DIK_0] & 0x80)
		cbUserOptions.mrtTextureToRender = 0;
	if(input->getKeyState()[DIK_1] & 0x80)
		cbUserOptions.mrtTextureToRender = 1;
	if(input->getKeyState()[DIK_2] & 0x80)
		cbUserOptions.mrtTextureToRender = 2;
	if(input->getKeyState()[DIK_3] & 0x80)
		cbUserOptions.mrtTextureToRender = 3;
	if(input->getKeyState()[DIK_4] & 0x80)
		cbUserOptions.mrtTextureToRender = 4;
	if(input->getKeyState()[DIK_5] & 0x80)
		cbUserOptions.mrtTextureToRender = 5;

	if(input->getKeyState()[DIK_T] & 0x80 && input->getPrevKeyState()[DIK_T]!=input->getKeyState()[DIK_T] && g->shader5Support) 
	{
		//if(dx11 compatible)
		if(tessellate)
			tessellate = false;
		else
			tessellate = true;
	}
	g->immediateContext->UpdateSubresource(g->cbUserOptions,0,NULL,&cbUserOptions,0,0);

	XMFLOAT3 movement = XMFLOAT3(0,0,0);

	if(input->getKeyState()[DIK_W] & 0x80)
		movement.z++;
	if(input->getKeyState()[DIK_S] & 0x80)
		movement.z--;
	if(input->getKeyState()[DIK_A] & 0x80)
		movement.x++;
	if(input->getKeyState()[DIK_D] & 0x80)
		movement.x--;
	if(input->getKeyState()[DIK_SPACE] & 0x80)
		movement.y++;
	if(input->getKeyState()[DIK_LCONTROL] & 0x80)
		movement.y--;
	if(!(movement.x == 0 && movement.y == 0 && movement.z == 0))
		XMStoreFloat3( &movement, XMVector3Normalize(XMLoadFloat3(&movement) ) * (float)timeStep );
	camera->fly(movement);

	camera->rotate((float)input->getDeltaX()*0.01f,(float)input->getDeltaY()*0.01f,0);

	water->update(dt);
	sword->rotate(0,0.01f,0);
}
void Game::draw()
{
	GraphicsCore* g = GraphicsCore::getInstance();

	g->immediateContext->PSSetSamplers( 0, 1, &g->samplerLinear );
	g->immediateContext->PSSetSamplers( 1, 1, &g->samplerSkybox );


	ID3D11ShaderResourceView *const pSRV[4] = {NULL, NULL, NULL, NULL};

	g->immediateContext->RSSetViewports( 1, &g->viewPortShadow);
	g->immediateContext->PSSetShaderResources(4, 1, pSRV);
	g->immediateContext->OMSetRenderTargets(0,0,g->depthStencilViewShadow);

	g->immediateContext->OMSetDepthStencilState(g->depthStencilStateEnable,0);
	g->immediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	g->immediateContext->RSSetState(g->rasterizerBackface);
	float blendFactor[4];
	blendFactor[0] = 0.0f;
	blendFactor[1] = 0.0f;
	blendFactor[2] = 0.0f;
	blendFactor[3] = 0.0f;
	g->immediateContext->OMSetBlendState( g->blendEnable, blendFactor, 0xffffffff );

	UINT strides[2];
	strides[0] = sizeof(SimpleMeshVertex);
	strides[1] = sizeof(PerInstanceData);
	UINT offsets[2];
	offsets[0] = 0;
	offsets[1] = 0;

	g->immediateContext->IASetVertexBuffers(0,2,staticVBufferSimple,strides,offsets);
	g->immediateContext->IASetIndexBuffer(staticIBufferSimple,DXGI_FORMAT_R32_UINT,0);
	g->immediateContext->IASetInputLayout( g->instShadowLayout );
	
	g->immediateContext->PSSetSamplers( 0, 1, &g->samplerLinear );
	g->immediateContext->PSSetSamplers( 1, 1, &g->samplerSkybox );
	
	g->techInstShadowmap->useTechnique();
	tree->updateCbObjects(&cbObjects,0);
	tree->updateCbWorld();
	tree->updateShaderTextures(0);
	g->immediateContext->DrawInstanced(tree->getGroupEnd(tree->getNoGroups()-1),5, terrain->vertices.size() + skybox->vertices.size(),0);

	//terrain->updateCbWorld();
	//g->immediateContext->DrawIndexed(terrain->indices.size(),0,0);

	CBTessellation tCb;
	tCb.padding = XMLoadFloat3( &XMFLOAT3(0.0f,0.0f,0.0f));
	tCb.tessellationAmount = 3.0f;
	g->immediateContext->UpdateSubresource(g->cbTessellation,0,NULL,&tCb,0,0);

	g->immediateContext->RSSetViewports( 1, &g->viewPort);

	// for reflection
	g->immediateContext->RSSetState(g->rasterizerFrontface);
	g->immediateContext->OMSetRenderTargets(1,&g->reflRenderTarget,g->depthStencilView);
	g->immediateContext->IASetInputLayout( g->simpleInputLayout );
	g->immediateContext->OMSetDepthStencilState(g->depthStencilStateDisable,0);
	g->immediateContext->PSSetShaderResources(0,1,&textures[5]);
	g->techSkyboxRefl->useTechnique();
	
	g->immediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	g->immediateContext->Draw(skybox->vertices.size(),terrain->vertices.size());

	// just draw the terrain not using deferred and only the sun as a light source
	g->immediateContext->RSSetState(g->rasterizerBackface);
	g->immediateContext->IASetInputLayout( g->simpleInputLayout );
	terrain->updateCbWorld();
	g->immediateContext->PSSetShaderResources(9,1,pSRV);
	g->immediateContext->PSSetShaderResources(0,5,&textures[0]);
	
	g->immediateContext->OMSetDepthStencilState(g->depthStencilStateEnable,0);
	
	g->techRefl->useTechnique();
	g->immediateContext->DrawIndexed(terrain->indices.size(),0,0);

	//must be cleared here in order for the reflection to not ruin the depth
	g->immediateContext->ClearDepthStencilView( g->depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );
	//end for reflection

	
	g->immediateContext->PSSetShaderResources(0, 4, pSRV);
	g->immediateContext->OMSetRenderTargets(4,g->mrtRTV,g->depthStencilView);

	
	g->immediateContext->OMSetDepthStencilState(g->depthStencilStateEnable,0);
	g->immediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	g->immediateContext->IASetInputLayout( g->inputLayout );
	g->immediateContext->RSSetState(g->rasterizerBackface);
	
	UINT stride = sizeof( MeshVertex );
    UINT offset = 0;
	g->immediateContext->IASetVertexBuffers(0,1, &staticVBuffer, &stride, &offset);
	g->techModelTan->useTechnique();
	
	g->immediateContext->PSSetShaderResources(4,1,&testTexture);


	
	sword->updateCbWorld();
	for(int i = 0; i < sword->getNoGroups(); i++)
	{
		sword->updateCbObjects(&cbObjects,i);
		sword->updateShaderTextures(i);
		g->immediateContext->Draw(sword->getGroupSize(i),sword->getGroupStart(i));
	}
	stride = sizeof( SimpleMeshVertex );
    offset = 0;

	g->immediateContext->IASetInputLayout( g->simpleInputLayout );
	g->immediateContext->IASetVertexBuffers(0,1, &staticVBufferSimple[0], &stride, &offset);

	
	g->techTerrain->useTechnique();
	g->immediateContext->PSSetShaderResources(0,5,&textures[0]);
	terrain->updateCbWorld();
	if(tessellate)
	{
		g->techTessellation->useTechnique();
		g->immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
		g->immediateContext->DSSetSamplers( 0, 1, &g->samplerLinear );
		g->immediateContext->DSSetShaderResources(0,5,&textures[0]);
		g->immediateContext->DSSetShaderResources(10,3,&textures[10]);
	}
	else
	{
		g->immediateContext->PSSetShaderResources(10,3,&textures[10]);
		g->techTerrain->useTechnique();
	}
	g->immediateContext->DrawIndexed(terrain->indices.size(),0,0);

	g->immediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	g->techInstModel->useTechnique();
	tree->updateCbObjects(&cbObjects,0);
	tree->updateCbWorld();
	tree->updateShaderTextures(0);
	g->immediateContext->IASetVertexBuffers(0,2,staticVBufferSimple,strides,offsets);
	g->immediateContext->IASetInputLayout( g->instanceLayout );
	g->immediateContext->DrawInstanced(tree->getGroupEnd(tree->getNoGroups()-1),5, terrain->vertices.size() + skybox->vertices.size(),0);

	//g->immediateContext->OMSetRenderTargets(1,&g->renderTargetView,g->depthStencilView);
	g->immediateContext->PSSetShaderResources(5,1,pSRV);
	g->immediateContext->OMSetRenderTargets(1,&g->sceneRenderTarget,NULL);
	g->immediateContext->IASetInputLayout( g->simpleInputLayout );
	g->immediateContext->OMSetDepthStencilState(g->depthStencilStateDisable,0);
	g->immediateContext->RSSetState(g->rasterizerFrontface);
	g->immediateContext->PSSetShaderResources(0,1,&textures[5]);
	g->techSkybox->useTechnique();
	
	g->immediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	g->immediateContext->Draw(skybox->vertices.size(),terrain->vertices.size());

	g->immediateContext->RSSetState(g->rasterizerBackface);

	

	//there's currecntly an overlap in index positions here,
	//is because depth is nolonger saved in the rendertagets but
	//depth stencil and so on
	g->immediateContext->PSSetShaderResources(0,4,g->mrtSRV);
	g->immediateContext->PSSetShaderResources(3,1,&g->depthStencilResource);
	g->immediateContext->PSSetShaderResources(4,1, &g->depthStencilResourceShadow);

	//draw the scene to a quad
	g->immediateContext->OMSetRenderTargets(1,&g->sceneRenderTarget,NULL);
	g->immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	g->techQuad->useTechnique();
	g->immediateContext->Draw(1,0); 

	//add water
	g->immediateContext->PSSetShaderResources(6,4,&textures[6]);
	g->immediateContext->PSSetShaderResources(9,1,&g->reflShaderResource);
	g->immediateContext->OMSetRenderTargets(1,&g->waterRenderTarget,NULL);
	g->immediateContext->PSSetShaderResources(5,1,&g->sceneShaderResource);
	g->techWater->useTechnique();
	water->updateCBWater();
	g->immediateContext->Draw(1,0);

	//blur the scene
	g->immediateContext->PSSetShaderResources(6, 1, pSRV);
	g->immediateContext->OMSetRenderTargets(1,&g->blurRenderTarget,NULL);
	g->immediateContext->PSSetShaderResources(5,1,&g->waterShaderResource);
	g->techBlur->useTechnique();
	g->immediateContext->Draw(1,0); 

	//add depth of field
	g->immediateContext->OMSetRenderTargets(1,&g->renderTargetView,NULL);
	g->immediateContext->PSSetShaderResources(6,1,&g->blurShaderResource);
	g->techDoF->useTechnique();
	g->immediateContext->Draw(1,0); 

	
	
	
}
Game::~Game()
{
	SAFE_RELEASE(staticVBufferSimple[0]);
	SAFE_RELEASE(staticVBufferSimple[1]);
	SAFE_DELETE(sword);
	SAFE_DELETE(terrain);
	SAFE_DELETE( skybox );
	SAFE_DELETE(camera);
	SAFE_DELETE(tree);
	SAFE_DELETE(water);
	SAFE_RELEASE(testTexture);

	for(unsigned int i = 0; i < textures.size(); i++) 
		SAFE_RELEASE(textures.at(i));
}

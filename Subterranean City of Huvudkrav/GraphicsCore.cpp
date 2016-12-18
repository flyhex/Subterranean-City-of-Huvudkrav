#include "GraphicsCore.h"
#include <sstream>

GraphicsCore *GraphicsCore::instance = NULL;

GraphicsCore::GraphicsCore()
{
	simpleInputLayout		= NULL;
	instanceLayout			= NULL;
	cbWorld					= NULL;
	cbObjects				= NULL;
	cbCameraMove			= NULL;
	cbOnce					= NULL;
	cbUserOptions			= NULL;
	blendEnable				= NULL;
	depthStencilStateEnable = NULL;
	rasterizerBackface		= NULL;
	rasterizerFrontface		= NULL;
	waterLayout				= NULL;
	shader5Support			= true;

	for(int i= 0; i < NORENDERTARGETS; i++)
	{
		mrtTex[i] = NULL;
		mrtSRV[i] = NULL;
	}
}

bool GraphicsCore::init(HWND *hWnd)
{
	HRESULT hr = S_OK;

	RECT	rc;
	GetClientRect( *hWnd, &rc );
	UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
//#ifdef _DEBUG
//    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
//#endif

	D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
	UINT numDriverTypes = ARRAYSIZE( driverTypes );

    D3D_FEATURE_LEVEL featureLevels[] =
    {
		//D3D_FEATURE_LEVEL_100_0,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

	UINT numFeatureLevels = ARRAYSIZE( featureLevels );

	DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = (*hWnd);
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

	for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
		driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain( NULL, driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
                                            D3D11_SDK_VERSION, &sd, &swapChain, &device, &featureLevel, &immediateContext );
        if( SUCCEEDED( hr ) )
            break;
    }
    if( FAILED( hr ) )
	{
		MessageBox( NULL, "Error creating device and swapchain","GraphicsCore Error", S_OK);
        return false;
	}

	// Create a render target view
    ID3D11Texture2D* backBuffer = NULL;
    hr = swapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&backBuffer );
    if( FAILED( hr ) )
	{
		MessageBox( NULL, "Error creating backbuffer","GraphicsCore Error", S_OK);
        return false;
	}

	hr = device->CreateRenderTargetView( backBuffer, NULL, &renderTargetView );
    backBuffer->Release();
    if( FAILED( hr ) )
	{
		MessageBox( NULL, "Error creating renderTargetView","GraphicsCore Error", S_OK);
        return false;
	}
	//create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
    ZeroMemory( &descDepth, sizeof(descDepth) );
    descDepth.Width					= width;
    descDepth.Height				= height;
    descDepth.MipLevels				= 1;
    descDepth.ArraySize				= 1;
	descDepth.Format				= DXGI_FORMAT_R32_TYPELESS;
    descDepth.SampleDesc.Count		= 1;
    descDepth.SampleDesc.Quality	= 0;
    descDepth.Usage					= D3D11_USAGE_DEFAULT;
	descDepth.BindFlags				= D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE; //also a shader resource
    descDepth.CPUAccessFlags		= 0;
    descDepth.MiscFlags				= 0;
    hr								= device->CreateTexture2D( &descDepth, NULL, &depthStencilTex );
    if( FAILED( hr ) )
	{
		MessageBox( NULL, "Error creating depth stencil texture","GraphicsCore Error", S_OK);
        return false;
	}

	descDepth.Width					= 4*width;
    descDepth.Height				= 4*height;
	//this is going to be used for shadowmapping
	hr								= device->CreateTexture2D( &descDepth,NULL,&depthStencilTexShadow );
	if( FAILED( hr ) )
	{
		MessageBox( NULL, "Error creating depth stencil texture for shadowmap","GraphicsCore Error", S_OK);
        return false;
	}

	// Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory( &descDSV, sizeof(descDSV) );
    descDSV.Format					= DXGI_FORMAT_D32_FLOAT;
    descDSV.ViewDimension			= D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice		= 0;
    hr								= device->CreateDepthStencilView( depthStencilTex, &descDSV, &depthStencilView );
    if( FAILED( hr ) )
	{
		MessageBox( NULL, "Error creating depth stencil view","GraphicsCore Error", S_OK);
        return false;
	}
	//used for shadowmap
	hr								= device->CreateDepthStencilView( depthStencilTexShadow, &descDSV, &depthStencilViewShadow );
    if( FAILED( hr ) )
	{
		MessageBox( NULL, "Error creating depth stencil view for shadowmap","GraphicsCore Error", S_OK);
        return false;
	}

	//create the depth stencil as a shader resource
	D3D11_SHADER_RESOURCE_VIEW_DESC	descSRV;
    descSRV.Format						= DXGI_FORMAT_R32_FLOAT;	// Change to R32_FLOAT from typeless
    descSRV.ViewDimension				= D3D10_SRV_DIMENSION_TEXTURE2D;
    descSRV.Texture2D.MostDetailedMip	= 0;
	descSRV.Texture2D.MipLevels			= 1;
    hr									= device->CreateShaderResourceView( depthStencilTex,
																			&descSRV,
																			&depthStencilResource );
	if( FAILED(hr) )
	{
		MessageBox(NULL, "Error creating depth stencil shader resource!", "GraphicsCore Error", S_OK);
		return false;
	}
	//used for shadowmap
	hr									= device->CreateShaderResourceView( depthStencilTexShadow,
																			&descSRV,
																			&depthStencilResourceShadow );
	if( FAILED(hr) )
	{
		MessageBox(NULL, "Error creating depth stencil shader resource!", "GraphicsCore Error", S_OK);
		return false;
	}
	//immediateContext->OMSetRenderTargets( 1, &renderTargetView, depthStencilView );

	// Setup the viewport
    viewPort.Width		= (FLOAT)width;
    viewPort.Height		= (FLOAT)height;
    viewPort.MinDepth		= 0.0f;
    viewPort.MaxDepth		= 1.0f;
    viewPort.TopLeftX		= 0;
    viewPort.TopLeftY		= 0;

	viewPortShadow.Width		= 4*(FLOAT)width;
    viewPortShadow.Height		= 4*(FLOAT)height;
    viewPortShadow.MinDepth		= 0.0f;
    viewPortShadow.MaxDepth		= 1.0f;
    viewPortShadow.TopLeftX		= 0;
    viewPortShadow.TopLeftY		= 0;
	immediateContext->RSSetViewports( 1, &viewPortShadow);

	techModel			= new ShaderTechnique("Shaders/model.fx","VS_Model","","","Shaders/model.fx","PS_Model");
	techInstModel		= new ShaderTechnique("Shaders/model.fx", "VS_InstancedModel","","","Shaders/model.fx","PS_Model");
	techModelTan		= new ShaderTechnique("Shaders/tangent.fx","VS_Tangent","","","Shaders/tangent.fx","PS_Tangent");
	techTerrain			= new ShaderTechnique("Shaders/terrain.fx", "VS_Terrain","","","Shaders/terrain.fx","PS_Terrain");
	techSkybox			= new ShaderTechnique("Shaders/skybox.fx", "VS_Skybox","","","Shaders/skybox.fx","PS_Skybox");
	techSkyboxRefl		= new ShaderTechnique("Shaders/skybox.fx", "VS_SkyboxRefl","","","Shaders/skybox.fx","PS_Skybox");
	techShadowmap		= new ShaderTechnique("Shaders/shadowmap.fx", "VS_Shadow","","","Shaders/shadowmap.fx","PS_Shadow");
	techInstShadowmap	= new ShaderTechnique("Shaders/shadowmap.fx", "VS_InstShadow","","","Shaders/shadowmap.fx","PS_Shadow");
	//rendering
	techQuad			= new ShaderTechnique("Shaders/lighting.fx", "VS_Quad","Shaders/lighting.fx","GS_Quad","Shaders/lighting.fx","PS_Quad");
	//post effects
	techBlur			= new ShaderTechnique("Shaders/lighting.fx", "VS_Quad","Shaders/lighting.fx","GS_Quad","Shaders/dof.fx","PS_Blur");
	techDoF				= new ShaderTechnique("Shaders/lighting.fx", "VS_Quad","Shaders/lighting.fx","GS_Quad","Shaders/dof.fx","PS_DepthOfField");
	techWater			= new ShaderTechnique("Shaders/lighting.fx", "VS_Quad","Shaders/lighting.fx","GS_Quad","Shaders/water.fx","PS_Water");
	techRefl			= new ShaderTechnique("Shaders/water.fx","VS_Reflection","","","Shaders/water.fx","PS_Reflection");

	if(shader5Support)
		techTessellation	= new ShaderTechnique(	"Shaders/tessellation.fx","VS_Tessellation",
												"Shaders/tessellation.fx","HS_Tessellation",
												"Shaders/tessellation.fx","DS_Tessellation",
												"Shaders/tessellation.fx","PS_Tessellation");
	
	D3D11_INPUT_ELEMENT_DESC simpleLayout[] = 
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,					D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(float) * 3,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, sizeof(float)* 6,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	hr = device->CreateInputLayout(simpleLayout, ARRAYSIZE(simpleLayout), techModel->getInputSignature(),techModel->getInputSignatureSize(), &simpleInputLayout);
	if(FAILED(hr))
	{
		MessageBox( NULL, "Failed to create simpleInputLayout","GraphicsCore Error",MB_OK);
		return false;
	}

	D3D11_INPUT_ELEMENT_DESC simpleLayoutInst[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(float) * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float) * 6, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "PERINST", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "PERINST", 1, DXGI_FORMAT_R32_FLOAT, 1, sizeof(float) * 3, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};
	hr = device->CreateInputLayout(simpleLayoutInst, ARRAYSIZE(simpleLayoutInst), techInstModel->getInputSignature(),techInstModel->getInputSignatureSize(), &instanceLayout);
	if(FAILED(hr))
	{
		MessageBox( NULL, "Failed to create instancedInputLayout","GraphicsCore Error",MB_OK);
		return false;
	}

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,					D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(float) * 3,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(float) * 6,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BITANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(float) * 9,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, sizeof(float)* 12,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	hr = device->CreateInputLayout(layout, ARRAYSIZE(layout), techModelTan->getInputSignature(),techModelTan->getInputSignatureSize(), &inputLayout);
	if(FAILED(hr))
	{
		MessageBox( NULL, "Failed to create InputLayout","GraphicsCore Error",MB_OK);
		return false;
	}

	D3D11_INPUT_ELEMENT_DESC shadowLayoutDesc[] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,0, D3D11_INPUT_PER_VERTEX_DATA,0}
	};
	hr = device->CreateInputLayout(shadowLayoutDesc, ARRAYSIZE(shadowLayoutDesc), techShadowmap->getInputSignature(),techShadowmap->getInputSignatureSize(), &shadowLayout);
	if(FAILED(hr))
	{
		MessageBox( NULL, "Failed to create shadowLayout","GraphicsCore Error",MB_OK);
		return false;
	}
	D3D11_INPUT_ELEMENT_DESC shadowInstLayoutDesc[] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,0, D3D11_INPUT_PER_VERTEX_DATA,0},
		{ "PERINST", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "PERINST", 1, DXGI_FORMAT_R32_FLOAT, 1, sizeof(float) * 3, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};
	hr = device->CreateInputLayout(shadowInstLayoutDesc, ARRAYSIZE(shadowInstLayoutDesc), techInstShadowmap->getInputSignature(),techInstShadowmap->getInputSignatureSize(), &instShadowLayout);
	if(FAILED(hr))
	{
		MessageBox( NULL, "Failed to create instShadowLayout","GraphicsCore Error",MB_OK);
		return false;
	}

	//create constant buffers
	
	if( !createCBuffer(&cbObjects,		sizeof(CBObjects),		0))
		return false;
	if( !createCBuffer(&cbWorld,		sizeof(CBWorld),		1))
		return false;
	if( !createCBuffer(&cbCameraMove,	sizeof(CBCameraMove),	2))
		return false;
	if( !createCBuffer(&cbOnce,			sizeof(CBOnce),			3))
		return false;
	if( !createCBuffer(&cbUserOptions,	sizeof(CBUserOptions),	4))
		return false;
	if( !createCBuffer(&cbTerrain,		sizeof(CBTerrain),		5))
		return false;
	if( !createCBuffer(&cbTessellation,	sizeof(CBTessellation), 6))
		return false;
	if( !createCBuffer(&cbWater,		sizeof(CBWater), 7))
		return false;
	if( !createCBuffer(&cbWaterOnce,	sizeof(CBWaterOnce), 8))
		return false;
	if( !createCBuffer(&cbReflection,	sizeof(CBReflection), 9))
		return false;

	//create samplerstates
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory( &sampDesc, sizeof(sampDesc));
	sampDesc.Filter			= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU		= D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV		= D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW		= D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc	= D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD			= -D3D11_FLOAT32_MAX;
	sampDesc.MaxLOD			= D3D11_FLOAT32_MAX;

	hr = device->CreateSamplerState( &sampDesc, &samplerLinear);
	if(FAILED(hr))
		return false;

	sampDesc.Filter		= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU	= D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV	= D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW	= D3D11_TEXTURE_ADDRESS_CLAMP;

	hr = device->CreateSamplerState( &sampDesc, &samplerSkybox);
	if(FAILED(hr))
		return false;

	//create blendstates
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc,sizeof(blendDesc));
	blendDesc.AlphaToCoverageEnable						= FALSE;
	blendDesc.IndependentBlendEnable					= FALSE;
	blendDesc.RenderTarget[0].BlendEnable				= FALSE;
	blendDesc.RenderTarget[0].SrcBlend					= D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlend					= D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOp					= D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha				= D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha			= D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha				= D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask		= 0x0f;

	hr = device->CreateBlendState(&blendDesc,&blendEnable);
	if(FAILED(hr))
		return false;

	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	hr = device->CreateBlendState(&blendDesc,&blendDisable);
	if(FAILED(hr))
		return false;

	//create depthstencil states
	D3D11_DEPTH_STENCIL_DESC depthDesc;
	depthDesc.DepthEnable					= TRUE;
	depthDesc.DepthWriteMask				= D3D11_DEPTH_WRITE_MASK_ALL;
	depthDesc.DepthFunc						= D3D11_COMPARISON_LESS;
	depthDesc.StencilEnable					= FALSE;
	depthDesc.StencilReadMask				= D3D11_DEFAULT_STENCIL_READ_MASK;
	depthDesc.StencilWriteMask				= D3D11_DEFAULT_STENCIL_WRITE_MASK;
	depthDesc.FrontFace.StencilFunc			= D3D11_COMPARISON_ALWAYS;
	depthDesc.BackFace.StencilFunc			= D3D11_COMPARISON_ALWAYS;
	depthDesc.FrontFace.StencilDepthFailOp	= D3D11_STENCIL_OP_KEEP;
	depthDesc.BackFace.StencilDepthFailOp	= D3D11_STENCIL_OP_KEEP;
	depthDesc.FrontFace.StencilPassOp		= D3D11_STENCIL_OP_KEEP;
	depthDesc.BackFace.StencilPassOp		= D3D11_STENCIL_OP_KEEP;
	depthDesc.FrontFace.StencilFailOp		= D3D11_STENCIL_OP_KEEP;
	depthDesc.BackFace.StencilFailOp		= D3D11_STENCIL_OP_KEEP;

	hr = device->CreateDepthStencilState(&depthDesc, &depthStencilStateEnable);
	if(FAILED(hr))
		return false;

	depthDesc.DepthEnable = FALSE;
	hr = device->CreateDepthStencilState(&depthDesc, &depthStencilStateDisable);

	//create rasterizerstates
	D3D11_RASTERIZER_DESC rasterDesc;
	ZeroMemory( &rasterDesc,sizeof(rasterDesc));
	rasterDesc.FillMode					= D3D11_FILL_SOLID;
	rasterDesc.CullMode					= D3D11_CULL_BACK;
    rasterDesc.FrontCounterClockwise	= false;
    rasterDesc.DepthBias				= 0;
    rasterDesc.DepthBiasClamp			= 0;
    rasterDesc.SlopeScaledDepthBias		= 0;
    rasterDesc.DepthClipEnable			= true;
    rasterDesc.ScissorEnable			= false;
    rasterDesc.MultisampleEnable		= false;
    rasterDesc.AntialiasedLineEnable	= false;

	hr = device->CreateRasterizerState( &rasterDesc, &rasterizerBackface );
	if(FAILED(hr))
		return false;

	rasterDesc.CullMode = D3D11_CULL_FRONT;
	hr = device->CreateRasterizerState( &rasterDesc, &rasterizerFrontface );
	if(FAILED(hr))
		return false;
	if(!initMRT())
		return false;
	return true;
}
bool GraphicsCore::initMRT()
{
	HRESULT hr = S_OK;
	D3D11_TEXTURE2D_DESC dsTex;
	ZeroMemory(&dsTex, sizeof(dsTex));
	dsTex.Width					= DEFAULTSCRNWIDTH;
	dsTex.Height				= DEFAULTSCRNHEIGHT;
	dsTex.MipLevels				= 1;
	dsTex.ArraySize				= 1;
	dsTex.SampleDesc.Count		= 1;
	dsTex.SampleDesc.Quality	= 0;
	dsTex.Format				= DXGI_FORMAT_D32_FLOAT;
	dsTex.Usage					= D3D11_USAGE_DEFAULT;
	dsTex.BindFlags				= D3D10_BIND_DEPTH_STENCIL;
	dsTex.CPUAccessFlags		= 0;

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format				= dsTex.Format;
	dsvDesc.ViewDimension		= D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice	= 0;

	dsTex.Format				= DXGI_FORMAT_R16G16B16A16_UNORM;
	dsTex.ArraySize				= 1;
	dsTex.BindFlags				= D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	for(int i = 0; i < NORENDERTARGETS; i++)
	{
		hr = device->CreateTexture2D( &dsTex, NULL, &mrtTex[i] );
		if(FAILED(hr))
		{
			MessageBox( NULL, "Failed to create multiple render target texture for deferred","GraphicsCore Error",MB_OK);
			return false;
		}
	}
	// ---- for water ------------------------//
	hr = device->CreateTexture2D( &dsTex, NULL, &waterTex );
	if(FAILED(hr))
	{
		MessageBox( NULL, "Failed to create scene render target texture for deferred","GraphicsCore Error",MB_OK);
		return false;
	}
	//reflection should be lower res
	hr = device->CreateTexture2D( &dsTex, NULL, &reflTex );
	if(FAILED(hr))
	{
		MessageBox( NULL, "Failed to create scene render target texture for deferred","GraphicsCore Error",MB_OK);
		return false;
	}
	// ----for post process depth of field----//
	hr = device->CreateTexture2D( &dsTex, NULL, &sceneTex );
	if(FAILED(hr))
	{
		MessageBox( NULL, "Failed to create scene render target texture for deferred","GraphicsCore Error",MB_OK);
		return false;
	}
	hr = device->CreateTexture2D( &dsTex, NULL, &blurTex );
	if(FAILED(hr))
	{
		MessageBox( NULL, "Failed to create blur render target texture for deferred","GraphicsCore Error",MB_OK);
		return false;
	}
	//---------------------------------------//

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format				= dsTex.Format;
	rtvDesc.ViewDimension		= D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice	= 0;

	for(int i = 0; i < NORENDERTARGETS; i++)
	{
		hr = device->CreateRenderTargetView(mrtTex[i],&rtvDesc, &mrtRTV[i]);
		if(FAILED(hr))
		{
			MessageBox( NULL, "Failed to create multiple render target view for deferred","GraphicsCore Error",MB_OK);
			return false;
		}
	}
	// ---- for water ------------------------//
	hr = device->CreateRenderTargetView(waterTex,&rtvDesc, &waterRenderTarget);
	if(FAILED(hr))
	{
		MessageBox( NULL, "Failed to create scene render target view for deferred","GraphicsCore Error",MB_OK);
		return false;
	}
	hr = device->CreateRenderTargetView(reflTex,&rtvDesc, &reflRenderTarget);
	if(FAILED(hr))
	{
		MessageBox( NULL, "Failed to create scene render target view for deferred","GraphicsCore Error",MB_OK);
		return false;
	}
	// ----for post process depth of field----//
	hr = device->CreateRenderTargetView(sceneTex,&rtvDesc, &sceneRenderTarget);
	if(FAILED(hr))
	{
		MessageBox( NULL, "Failed to create scene render target view for deferred","GraphicsCore Error",MB_OK);
		return false;
	}
	hr = device->CreateRenderTargetView(blurTex,&rtvDesc, &blurRenderTarget);
	if(FAILED(hr))
	{
		MessageBox( NULL, "Failed to create blur render target view for deferred","GraphicsCore Error",MB_OK);
		return false;
	}
	//---------------------------------------//

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory( &srvDesc, sizeof(srvDesc));
	srvDesc.Format						= dsTex.Format;
	srvDesc.ViewDimension				= D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels			= 1;
	srvDesc.Texture2D.MostDetailedMip	= 0;

	for(int i = 0; i < NORENDERTARGETS; i++)
	{
		hr = device->CreateShaderResourceView(mrtTex[i], &srvDesc, &mrtSRV[i]);
		if(FAILED(hr))
		{
			MessageBox( NULL, "Failed to create multiple shader resource view for deferred","GraphicsCore Error",MB_OK);
			return false;
		}
	}
	// ---- for water ------------------------//
	hr = device->CreateShaderResourceView(waterTex, &srvDesc, &waterShaderResource);
	if(FAILED(hr))
	{
		MessageBox( NULL, "Failed to create scene shader resource view for deferred","GraphicsCore Error",MB_OK);
		return false;
	}
	hr = device->CreateShaderResourceView(reflTex, &srvDesc, &reflShaderResource);
	if(FAILED(hr))
	{
		MessageBox( NULL, "Failed to create scene shader resource view for deferred","GraphicsCore Error",MB_OK);
		return false;
	}
	// ----for post process depth of field----//
	hr = device->CreateShaderResourceView(sceneTex, &srvDesc, &sceneShaderResource);
	if(FAILED(hr))
	{
		MessageBox( NULL, "Failed to create scene shader resource view for deferred","GraphicsCore Error",MB_OK);
		return false;
	}
	hr = device->CreateShaderResourceView(blurTex, &srvDesc, &blurShaderResource);
	if(FAILED(hr))
	{
		MessageBox( NULL, "Failed to create blur shader resource view for deferred","GraphicsCore Error",MB_OK);
		return false;
	}
	//---------------------------------------//
	immediateContext->PSSetShaderResources(0,4,mrtSRV);
	return true;
}
bool GraphicsCore::createVBuffer( const D3D11_BUFFER_DESC *bd, const D3D11_SUBRESOURCE_DATA *initData, ID3D11Buffer **vBuffer )
{
	HRESULT hr = device->CreateBuffer( bd, initData, vBuffer );
	if( FAILED( hr ) )
	{
		MessageBox(NULL,"Failed to create vertex buffer!", "GraphicsCore Error", S_OK);
		return false;
	}
	return true;
}
HRESULT GraphicsCore::compileShaderFromFile( LPCSTR fileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DX11CompileFromFile( fileName , NULL, NULL, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
    if( FAILED(hr) )
    {
        if( pErrorBlob != NULL )
            OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
        if( pErrorBlob ) pErrorBlob->Release();
        return hr;
    }
    if( pErrorBlob ) pErrorBlob->Release();

    return S_OK;
}

void GraphicsCore::clearRenderTarget(float x, float y, float z, float w)
{
	float clearColor[4] = { x,y,z,w };
	immediateContext->ClearRenderTargetView( renderTargetView, clearColor );
	immediateContext->ClearRenderTargetView( reflRenderTarget, clearColor);
	immediateContext->ClearRenderTargetView( waterRenderTarget, clearColor);
	immediateContext->ClearRenderTargetView( sceneRenderTarget, clearColor);
	immediateContext->ClearRenderTargetView( blurRenderTarget, clearColor);

	immediateContext->ClearDepthStencilView( depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );
	immediateContext->ClearDepthStencilView( depthStencilViewShadow, D3D11_CLEAR_DEPTH,1.0f,0 );
}
void GraphicsCore::clearMultipleRenderTarget(float r, float g, float b)
{
	float clearColor[4] = {r,g,b,1};
	for(int i = 0; i < NORENDERTARGETS; i++)
		immediateContext->ClearRenderTargetView(mrtRTV[i],clearColor);
}
void GraphicsCore::presentSwapChain()
{
	swapChain->Present( 0, 0 );
}

bool GraphicsCore::createCBuffer(ID3D11Buffer **cb, UINT byteWidth, UINT registerIndex)
{
	HRESULT hr;
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage			= D3D11_USAGE_DEFAULT;
	bd.BindFlags		= D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags	= 0;
	bd.ByteWidth		= byteWidth;

	hr = device->CreateBuffer( &bd,NULL,cb);
	if(FAILED(hr))
	{
		stringstream numb;
		numb << registerIndex;
		string message = "Failed to create constant buffer " + numb.str();
		MessageBox( NULL, message.c_str(),"GraphicsCore Error",MB_OK);
		return false;
	}
	immediateContext->VSSetConstantBuffers(registerIndex, 1, cb);
	immediateContext->DSSetConstantBuffers(registerIndex, 1, cb);
	immediateContext->PSSetConstantBuffers(registerIndex, 1, cb);
	immediateContext->HSSetConstantBuffers(registerIndex, 1, cb);
	return true;
}
GraphicsCore::~GraphicsCore()
{
	ShaderTechnique::cleanUp();
	SAFE_RELEASE( renderTargetView );
	SAFE_RELEASE( swapChain );
	SAFE_RELEASE( device );
	SAFE_RELEASE( immediateContext );
	//shaders
	SAFE_DELETE( techModel );
	SAFE_DELETE( techModelTan );
	SAFE_DELETE( techQuad );
	SAFE_DELETE( techSkybox );
	SAFE_DELETE( techSkyboxRefl );
	SAFE_DELETE( techTerrain );
	if(shader5Support)
		SAFE_DELETE( techTessellation );
	SAFE_DELETE( techInstModel );
	SAFE_DELETE( techShadowmap );
	SAFE_DELETE( techInstShadowmap );
	SAFE_DELETE( techWater );
	SAFE_DELETE( techBlur );
	SAFE_DELETE( techDoF );
	SAFE_DELETE( techRefl );

	//layouts
	SAFE_RELEASE( simpleInputLayout );
	SAFE_RELEASE( inputLayout );
	SAFE_RELEASE( instanceLayout );
	SAFE_RELEASE( shadowLayout );
	SAFE_RELEASE( instShadowLayout );
	SAFE_RELEASE( waterLayout );
	//depth stencil
	SAFE_RELEASE( depthStencilTex );
	SAFE_RELEASE( depthStencilResource );
	SAFE_RELEASE( depthStencilView );
	//constant buffers
	SAFE_RELEASE( cbWorld );
	SAFE_RELEASE( cbObjects );
	SAFE_RELEASE( cbCameraMove );
	SAFE_RELEASE( cbOnce );
	SAFE_RELEASE( cbUserOptions );
	SAFE_RELEASE( cbTerrain );
	SAFE_RELEASE( cbTessellation );
	//blendstates
	SAFE_RELEASE( blendEnable );
	SAFE_RELEASE( blendDisable );
	//depthstencilstates
	SAFE_RELEASE( depthStencilStateEnable );
	SAFE_RELEASE( depthStencilStateDisable );
	//rasterizerstates
	SAFE_RELEASE( rasterizerBackface );
	SAFE_RELEASE( rasterizerFrontface );
	//samplestates
	SAFE_RELEASE( samplerLinear );
	SAFE_RELEASE( samplerSkybox );
	for(int i = 0; i < NORENDERTARGETS; i++)
	{
		SAFE_RELEASE(mrtRTV[i]);
		SAFE_RELEASE(mrtTex[i]);
		SAFE_RELEASE(mrtSRV[i]);
	}
	SAFE_RELEASE(sceneTex);
	SAFE_RELEASE(sceneRenderTarget);
	SAFE_RELEASE(sceneShaderResource);

	SAFE_RELEASE(blurTex);
	SAFE_RELEASE(blurRenderTarget);
	SAFE_RELEASE(blurShaderResource);

	SAFE_RELEASE(waterTex);
	SAFE_RELEASE(waterRenderTarget);
	SAFE_RELEASE(waterShaderResource);

	SAFE_RELEASE(reflTex);
	SAFE_RELEASE(reflRenderTarget);
	SAFE_RELEASE(reflShaderResource);
}

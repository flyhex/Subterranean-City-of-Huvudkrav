#pragma once
#include "Resource.h"
#include <D3Dcompiler.h>
#include <iostream>
#include "ShaderTechnique.h"

struct pointLight
{
	XMVECTOR position;
	XMVECTOR color;
	float lightReach;
};

struct spotLight
{
	XMVECTOR position;
	XMVECTOR color;
	float lightReach;
	XMVECTOR direction;
	float cosTheta;
	float cosPhi;
};

struct CBWorld
{
	XMMATRIX world;
};
struct CBObjects
{
	float ns;		//specular intensity (0-1000)
	float ni;		//optical density (index of refraction)
	float tr;		//Transparency a.k.a  d (dissolved)
	int illum;		//Illumination type
	XMVECTOR tf;	//transmission filter (decides what colors get to pass through the material)
	XMVECTOR ka;	//ambient
	XMVECTOR kd;	//diffuse
	XMVECTOR ks;	//specular lightning. black = off.
	XMVECTOR ke;	//emissive. The self-illumination color.
};

struct CBCameraMove
{
	XMMATRIX view;
	XMMATRIX viewInv;
	//XMMATRIX viewRefl;
	XMVECTOR cameraPos;
	XMVECTOR cameraDir;
	
};

struct CBReflection
{
	XMMATRIX viewRefl;
};

struct CBOnce
{
	XMMATRIX projection;
	XMMATRIX projInv;
	XMMATRIX lightView;
};

struct CBUserOptions
{
	int mrtTextureToRender;
	int mipmap; // cause needs to be at least 16 bytes
	int temp1;
	int temp2;
};
struct CBTerrain
{
	XMINT2 terrainTileSize;
	XMINT2 terrainSize;
};
struct CBTessellation
{
	float tessellationAmount;
	XMVECTOR padding;
};

struct CBWater
{
	float waterLevel;
	float timer;
	XMFLOAT2 windDirection;
};

struct CBWaterOnce
{
	float waterFade; //how deep anything need to be to fade out in the water
	float normalScaling; 
	float maxAmplitude; //max wave amplitude
	float shoreTransition; //how soft the water and ground fades
	float refractionStrength; //refraction strength
	float displacementStrength; //multiplier for the height of waves
	float shininess;
	float specularIntensity;
	float transparency;
	float refractionScale;

	XMVECTOR normalModifier; //multiplier for the different normals. first one is for small waves.
	XMVECTOR foamOptions; //depth of which foam starts to fade out, depth of which foam is invisible, height of which foam appears for waves.
	XMVECTOR waterSurfaceColor;
	XMVECTOR waterDepthColor;
	XMVECTOR extinction;
	XMVECTOR scale;
	XMVECTOR temp;
};

// http://www.beyond3d.com/content/articles/19/1
class GraphicsCore
{
private:
	static GraphicsCore			*instance;
	D3D_DRIVER_TYPE				driverType;
	D3D_FEATURE_LEVEL			featureLevel;
	ID3D11Device				*device;
	
	IDXGISwapChain				*swapChain;

	bool initMRT();
	GraphicsCore();
public:
	bool						shader5Support;
	HRESULT						compileShaderFromFile( LPCSTR fileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );

	static GraphicsCore			*getInstance()
	{
		if(!instance)
			instance = new GraphicsCore();
		return instance;
	}
	ID3D11DeviceContext			*immediateContext;
	//shaders

	ShaderTechnique				*techModel;
	ShaderTechnique				*techModelTan;
	ShaderTechnique				*techInstModel;
	ShaderTechnique				*techQuad;
	ShaderTechnique				*techTerrain;
	ShaderTechnique				*techSkybox;
	ShaderTechnique				*techTessellation;
	ShaderTechnique				*techShadowmap;
	ShaderTechnique				*techInstShadowmap;
	ShaderTechnique				*techBlur;
	ShaderTechnique				*techDoF;
	ShaderTechnique				*techWater;
	ShaderTechnique				*techRefl;
	ShaderTechnique				*techSkyboxRefl;

	//layouts
	ID3D11InputLayout			*waterLayout;
	ID3D11InputLayout			*simpleInputLayout;
	ID3D11InputLayout			*instanceLayout;
	ID3D11InputLayout			*inputLayout;
	ID3D11InputLayout			*shadowLayout;
	ID3D11InputLayout			*instShadowLayout;

	//render targets
	ID3D11RenderTargetView		*renderTargetView;

	ID3D11Texture2D*			mrtTex[4];
	ID3D11RenderTargetView*		mrtRTV[4];
	ID3D11ShaderResourceView*	mrtSRV[4];

	ID3D11Texture2D				*sceneTex;
	ID3D11RenderTargetView		*sceneRenderTarget;
	ID3D11ShaderResourceView	*sceneShaderResource;

	ID3D11Texture2D				*waterTex;
	ID3D11RenderTargetView		*waterRenderTarget;
	ID3D11ShaderResourceView	*waterShaderResource;

	ID3D11Texture2D				*reflTex;
	ID3D11RenderTargetView		*reflRenderTarget;
	ID3D11ShaderResourceView	*reflShaderResource;

	ID3D11Texture2D				*blurTex;
	ID3D11RenderTargetView		*blurRenderTarget;
	ID3D11ShaderResourceView	*blurShaderResource;

	//depth stencil
	ID3D11Texture2D				*depthStencilTex;
	ID3D11DepthStencilView		*depthStencilView;
	ID3D11ShaderResourceView	*depthStencilResource;

	ID3D11Texture2D				*depthStencilTexShadow;
	ID3D11DepthStencilView		*depthStencilViewShadow;
	ID3D11ShaderResourceView	*depthStencilResourceShadow;

	//constant buffers
	ID3D11Buffer				*cbWorld;
	ID3D11Buffer				*cbObjects;
	ID3D11Buffer				*cbCameraMove;
	ID3D11Buffer				*cbOnce;
	ID3D11Buffer				*cbUserOptions;
	ID3D11Buffer				*cbTerrain;
	ID3D11Buffer				*cbTessellation;
	ID3D11Buffer				*cbWater;
	ID3D11Buffer				*cbWaterOnce;
	ID3D11Buffer				*cbReflection;

	//viewports
	D3D11_VIEWPORT				viewPort;
	D3D11_VIEWPORT				viewPortShadow;

	//blendstates
	ID3D11BlendState			*blendEnable;
	ID3D11BlendState			*blendDisable;

	//depthstencilstates
	ID3D11DepthStencilState		*depthStencilStateEnable;
	ID3D11DepthStencilState		*depthStencilStateDisable;

	//rasterizerstates
	ID3D11RasterizerState		*rasterizerBackface;
	ID3D11RasterizerState		*rasterizerFrontface;
	//samplestates
	ID3D11SamplerState			*samplerLinear;
	ID3D11SamplerState			*samplerSkybox;

	bool						init(HWND *hWnd);
	bool						createVBuffer( const D3D11_BUFFER_DESC *bd, const D3D11_SUBRESOURCE_DATA *initData, ID3D11Buffer **vBuffer );
	void						clearRenderTarget(float x, float y, float z, float w);
	void						clearMultipleRenderTarget(float r, float g, float b);

	void						presentSwapChain();
	bool						createCBuffer(ID3D11Buffer **cb, UINT byteWidth, UINT registerIndex);

	ID3D11Device				*getDevice() { return device;}
	~GraphicsCore();
};


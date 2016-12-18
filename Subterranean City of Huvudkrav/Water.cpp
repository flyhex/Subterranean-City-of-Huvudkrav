#include "Water.h"
#include "GraphicsCore.h"

Water::Water()
{
	GraphicsCore* g = GraphicsCore::getInstance();

	timer = 0;
	waterLevel				= .0f; //height of sea level
	windDirection			= XMFLOAT2(-0.2, 0.8 );

	waterFade				= .15f; //how deep anything need to be to fade out in the water
	normalScaling			= 1.0f; 
	maxAmplitude			= 5.0f; //max wave amplitude
	shoreTransition			= 0.5f; //how soft the water and ground fades
	refractionStrength		= .0f; //refraction strength
	displacementStrength	= 1.7f; //multiplier for the height of waves
	shininess				= .0f;
	specularIntensity		= .32f;
	transparency			= 4.0f;
	refractionScale			= 0.005f;

	normalModifier			= XMFLOAT4(1.0f, 2.0f, 4.0f, 8.0f ); //multiplier for the different normals. first one is for small waves.
	foamOptions				= XMFLOAT3(0.75f, 1.35f, 0.5f ); //depth of which foam starts to fade out, depth of which foam is invisible, height of which foam appears for waves.
	waterSurfaceColor		= XMFLOAT3(0.0078f,	0.517f, 0.7f );
	waterDepthColor			= XMFLOAT3(0.0039f,	0.0196f, 0.145f );
	extinction				= XMFLOAT3(28.0f, 120.0f, 160.0f );
	scale					= XMFLOAT2(0.0005f, 0.0005f );

	CBWater				cb;
	cb.timer				= timer;
	cb.waterLevel			= waterLevel;
	cb.windDirection		= windDirection;

	g->immediateContext->UpdateSubresource(g->cbWater,0,NULL,&cb,0,0);

	CBWaterOnce	cbOnce;
	cbOnce.waterFade			= waterFade;
	cbOnce.normalScaling		= normalScaling; 
	cbOnce.maxAmplitude			= maxAmplitude;
	cbOnce.shoreTransition		= shoreTransition;
	cbOnce.refractionStrength	= refractionStrength;
	cbOnce.displacementStrength	= displacementStrength;
	cbOnce.shininess			= shininess;
	cbOnce.specularIntensity	= specularIntensity;
	cbOnce.transparency			= transparency;
	cbOnce.refractionScale		= refractionScale;

	cbOnce.normalModifier		= XMLoadFloat4( &normalModifier );
	cbOnce.foamOptions			= XMLoadFloat3( &foamOptions );
	cbOnce.waterSurfaceColor	= XMLoadFloat3( &waterSurfaceColor );
	cbOnce.waterDepthColor		= XMLoadFloat3( &waterDepthColor );
	cbOnce.extinction			= XMLoadFloat3( &extinction );
	cbOnce.scale				= XMLoadFloat2( &scale );
	cbOnce.temp					= XMLoadFloat3( &XMFLOAT3(0,0,0) );

	g->immediateContext->UpdateSubresource(g->cbWaterOnce,0,NULL,&cbOnce,0,0);
}

void Water::update(double dt)
{
	timer += dt*5000;
}
void Water::updateCBWater()
{
	GraphicsCore* g			= GraphicsCore::getInstance();
	CBWater				cb;
	cb.timer				= (float)timer;
	cb.waterLevel			= waterLevel;
	cb.windDirection		= windDirection;

	g->immediateContext->UpdateSubresource(g->cbWater,0,NULL,&cb,0,0);
}
void Water::getReflectionView( XMFLOAT4X4 &out, XMFLOAT4X4 &view)
{

}
Water::~Water()
{
}

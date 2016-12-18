//-----------------------------------------------------------------
//
// Global resources shared between shaders
//
//-----------------------------------------------------------------
#define PI 3.14159265f

SamplerState samLinear	: register( s0 );
SamplerState samSkybox	: register( s1 );

//shared structs
struct pointLight
{
	float3 position;
	float3 color;
	float lightReach;
};
struct spotLight
{
	float3 position;
	float3 color;
	float lightReach;
	float3 direction;
	float cosTheta;
	float cosPhi;
};

cbuffer cbObjs			: register( b0 )
{
	float ns;					//specular intensity (0-1000)
	float ni;					//optical density (index of refraction)
	float tr;					//Transparency a.k.a  d (dissolved)
	int illum;					//Illumination type
	float3 tf;					//transmission filter (decides what colors get to pass through the material)
	float3 ka;					//ambient
	float3 kd;					//diffuse
	float3 ks;					//specular lightning. black = off.
	float3 ke;					//emissive. The self-illumination color.
};

cbuffer cbWorld			: register( b1 )
{
	matrix world;
	matrix worldInv;
};
cbuffer cbCameraMove	: register( b2 )
{
	matrix view;
	matrix viewInv;
	//matrix viewRefl;
	float3 cameraPos;
	float3 cameraDir;
	
};
cbuffer cbOnce			: register( b3 )
{
	matrix projection;
	matrix projInv;
	matrix lightView;
};
cbuffer cbUserOptions	: register( b4 )
{
	int mrtTextureToRender;
	int mipmap; // cause needs to be at least 16 bytes
	int temp1;
	int temp2;
};

cbuffer cbTerrain		: register( b5 )
{
	int2 terrainTileSize;
	int2 terrainSize;
};
cbuffer cbStaticLights	: register( b6 )
{
	int noPointLights;
	int noSpotLights;
	pointLight pointLights[3];
	spotLight spotLights[1];
}
cbuffer cbWater			: register( b7 )
{
	float waterLevel; //height of sea level
	float timer;
	float2 windDirection;
};
cbuffer cbWaterOnce		: register( b8 )
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

	float4 normalModifier; //multiplier for the different normals. first one is for small waves.
	float3 foamOptions; //depth of which foam starts to fade out, depth of which foam is invisible, height of which foam appears for waves.
	float3 waterSurfaceColor;
	float3 waterDepthColor;
	float3 extinction;
	float2 scale;
	float3 temp;
}
cbuffer cbReflection	: register( b9 )
{
	matrix viewRefl;
}
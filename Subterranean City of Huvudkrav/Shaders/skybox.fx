#include "resource.fx" 

Texture2D texSkybox			: register( t0 );

struct VS_Input
{
	float3 pos	: POSITION;
	float3 norm	: NORMAL;
	float2 tex	: TEXCOORD0;
};

struct PS_Input
{
	float4 pos	: SV_POSITION;
	float2 tex	: TEXCOORD0;
};

PS_Input VS_Skybox( VS_Input input )
{
	PS_Input output = (PS_Input)0;
	output.tex = input.tex;
	output.pos = float4(input.pos,1);
	output.pos = float4(mul((float3x3)view, output.pos.xyz ),1);
	output.pos = mul(projection, output.pos );

	return output;
}

PS_Input VS_SkyboxRefl( VS_Input input)
{
	PS_Input output = (PS_Input)0;
	output.tex = input.tex;
	output.pos = float4(input.pos,1);
	output.pos = float4(mul((float3x3)viewRefl, output.pos.xyz ),1);
	output.pos = mul(projection, output.pos );

	return output;
}

float4 PS_Skybox( PS_Input input ) : SV_TARGET0
{
	return texSkybox.Sample(samSkybox, input.tex);
}
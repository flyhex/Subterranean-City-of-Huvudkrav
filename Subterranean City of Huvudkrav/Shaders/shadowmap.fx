#include "resource.fx"

struct VS_Input
{
	float3 pos : POSITION;
};
struct VS_InstInput
{
	float3 pos		: POSITION;
	float3 instPos	: PERINST0;
	float  instRot	: PERINST1;
};
struct PS_Input
{
	float4 pos : SV_POSITION;
};

PS_Input VS_Shadow(VS_Input input)
{
	PS_Input output = (PS_Input)0;
	output.pos = mul(world, float4(input.pos,1.0));
	output.pos = mul(lightView, output.pos);
	output.pos = mul(projection, output.pos);

	return output;
}
PS_Input VS_InstShadow(VS_InstInput input)
{
	PS_Input output = (PS_Input)0;
	float	 fRot = input.instRot;
	matrix mRotation = {	cos(fRot),	0,	sin(fRot),	0,
							0,			1,	0,			0,
							-sin(fRot),	0,	cos(fRot),	0,
							0,			0,	0,			1
						};
	mRotation = transpose(mRotation);
	matrix tempWorld = mul(world,mRotation);
	output.pos = mul(tempWorld, float4(input.pos,1.0f));
	output.pos.xyz += input.instPos;
	output.pos = mul(lightView, output.pos);
	output.pos = mul(projection, output.pos);

	return output;
}
float4 PS_Shadow(PS_Input input) : SV_TARGET0
{
	return float4(1,0,0,0);
}
#include "resource.fx"
#include "deferred.fx"

//texture maps
Texture2D map_ka			: register( t0 );					//ambient map
Texture2D map_kd			: register( t1 );					//diffuse map
Texture2D map_ks			: register( t2 );					//specular map
Texture2D map_ke			: register( t3 );					//emissive map
Texture2D map_ns			: register( t4 );					//specular intensity map
Texture2D map_tr			: register( t5 );					//transparency map
Texture2D map_decal			: register( t6 );					//stencil decal map
Texture2D map_normal		: register( t7 );					//normal map / bump / disp

struct VS_Input
{
	float3 pos	: POSITION;
	float3 norm	: NORMAL;
	float2 tex	: TEXCOORD0;
};

struct VS_InstancedInput
{
	float3 pos		: POSITION;
	float3 norm		: NORMAL;
	float2 tex		: TEXCOORD0;
	float3 instPos	: PERINST0;
	float  instRot	: PERINST1;
};

struct PS_Input
{
	float4 pos		: SV_POSITION;
	float4 posWV	: TEXCOORD0;
	float3 norm		: NORMAL;
	float2 tex		: TEXCOORD1;
};

//todo:
//functions and variables for deferred calculations.
//


// vertex shader for drawing positions, normals and such to the
// different rendertargets
PS_Input VS_Model(VS_Input input)
{
	PS_Input output = (PS_Input)0;
	output.pos = mul(world, float4(input.pos,1.0f));
	output.posWV = mul(view, output.pos);
	output.pos = mul(projection, output.posWV);
	matrix worldView = mul(view, world);
	output.norm = normalize(mul(worldView, float4(input.norm,0)).xyz);
	output.tex = input.tex;
	return output;
}

PS_Input VS_InstancedModel(VS_InstancedInput input)
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
	// add the position of this instance of the model to the vertex
	output.pos.xyz += input.instPos;
	output.posWV = mul(view, output.pos);
	output.pos = mul(projection, output.posWV);
	matrix worldView = mul(view, tempWorld);
	output.norm = normalize(mul(worldView, float4(input.norm,0)).xyz);
	output.tex = input.tex;
	return output;
}

// pixel shader for drawing the stuff needed for deferred to the different 
// render targets
//PS_MRTOutput PS_Model(PS_Input input)
//{
//	PS_MRTOutput output = (PS_MRTOutput)0;
//
//	output.diffuse = map_kd.Sample(samLinear, input.tex);
//	output.normal.xyz = map_normal.Sample( samLinear, input.tex ).xyz;
//	output.normal.xyz = (output.normal.xyz * 2) - 1;
//	output.normal.xyz = mul( (float3x3)world, output.normal.xyz );
//	output.normal.xyz = mul( (float3x3)view, output.normal.xyz );
//	//output.normal.xyz = (output.normal * 2) - 1;
//	output.normal.xyz = normalize( output.normal.xyz );// + input.norm );
//	output.normal.xyz = (output.normal.xyz + 1) * 0.5;
//	
//	if(output.normal.x<0 || output.normal.y<0 || output.normal.z<0)
//		output.normal = float4(1,1,1,1);
//	output.normal.w = input.pos.z;
//	
//
//	return output;
//}
PS_MRTOutput PS_Model(PS_Input input)
{
	PS_MRTOutput output = (PS_MRTOutput)0;

	output.diffuse = map_kd.Sample(samLinear, input.tex);
	/*output.normal.xyz = map_normal.Sample( samLinear, input.tex ).xyz;
	output.normal.xyz = (output.normal.xyz * 2) - 1;
	output.normal.xyz = mul( (float3x3)world, output.normal.xyz );
	output.normal.xyz = mul( (float3x3)view, output.normal.xyz );*/

	//output.normal.xyz = (output.normal * 2) - 1;
	//output.normal.xyz = normalize( output.normal.xyz );// + input.norm );
	output.normal.xyz = input.norm;
	output.normal.xyz = (output.normal.xyz + 1) * 0.5;
	
	if(output.normal.x<0 || output.normal.y<0 || output.normal.z<0)
		output.normal = float4(1,1,1,1);
	output.normal.w = input.pos.z;
	

	return output;
}
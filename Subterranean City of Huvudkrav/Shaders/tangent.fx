#include "resource.fx"
#include "deferred.fx"
//-----------------------------------------------------------------
//
// Tangent space based models
//
//-----------------------------------------------------------------
//

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
	float3 pos		: POSITION;
	float3 norm		: NORMAL;
	float3 tan		: TANGENT;
	float3 bitan	: BITANGENT;
	float2 tex		: TEXCOORD;
};

struct PS_Input
{
	float4 pos		: SV_POSITION;
	float3 norm		: NORMAL;
	float3x3 tanBiNor : MATRIX;
	float2 tex		: TEXCOORD0;
};

PS_Input VS_Tangent(VS_Input input)
{
	PS_Input output = (PS_Input)0;
	output.pos		= mul(world,		float4( input.pos, 1.0f ) );
	output.pos		= mul(view,			output.pos );
	output.pos		= mul(projection,	output.pos );

	output.norm = normalize(mul(world, float4(input.norm,0)).xyz);
	output.norm = normalize(mul(view, float4(output.norm,0)).xyz);
	//output.norm = normalize( output.normal.xyz );
	output.tex		= input.tex;
	if( mrtTextureToRender == 5 )
	{
		output.norm = normalize(mul(world, float4(input.tan,0)).xyz);
		output.norm = normalize(mul(view, float4(output.norm,0)).xyz);
	}
	// world space to tangent space matrix

	output.tanBiNor = float3x3(
    		normalize(mul(world,float4(input.tan,0)).xyz),
    		normalize(mul(world,float4(input.bitan,0)).xyz),
    		normalize(mul(world,float4(input.norm,0)).xyz)
	);
	return output;
}

PS_MRTOutput PS_Tangent(PS_Input input)
{
	PS_MRTOutput output = (PS_MRTOutput)0;

	output.diffuse = map_kd.Sample(samLinear, input.tex);
	//output.normal.xyz = input.norm;
	output.normal.xyz = map_normal.Sample( samLinear, input.tex ).xyz;
	output.normal.xyz = ( output.normal.xyz * 2) - 1;
	output.normal.xyz = normalize(mul( output.normal.xyz, input.tanBiNor ));
	//output.normal.xyz = mul( (float3x3)world, output.normal.xyz );
	output.normal.xyz = mul( (float3x3)view, output.normal.xyz );
	//output.normal.xyz = (output.normal * 2) - 1;
	output.normal.xyz = normalize( output.normal.xyz ); // + input.norm );
	output.normal.xyz = (output.normal.xyz + 1) * 0.5;

	
	if(output.normal.x<0 || output.normal.y<0 || output.normal.z<0)
		output.normal = float4(1,1,1,1);
	output.normal.w = input.pos.z;
	

	return output;
}
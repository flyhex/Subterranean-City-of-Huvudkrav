#include "resource.fx"
#include "deferred.fx"

Texture2D texTerrainBlend	: register( t0 );
Texture2D texTerrain0		: register( t1 );
Texture2D texTerrain1		: register( t2 );
Texture2D texTerrain2		: register( t3 );
Texture2D texTerrain0Spec	: register( t4 );
Texture2D texTerrain1SPec	: register( t5 );
Texture2D texTerrain2Spec	: register( t6 );
Texture2D texNorm1	: register( t10 );
Texture2D texNorm2	: register( t11 );
Texture2D texNorm3	: register( t12 );

struct VS_Input
{
	float3 pos	: POSITION;
	float3 norm	: NORMAL;
	float2 tex	: TEXCOORD0;
};

struct PS_TerrainInput
{
	float4 pos			: SV_POSITION;
	float3 norm			: NORMAL;
	float2 tex			: TEXCOORD0;
	float2 nonTiledTex	: TEXCOORD1;
};

//PS_TerrainInput VS_Terrain(VS_Input input)
//{
//	PS_TerrainInput output = (PS_TerrainInput)0;
//	float2 nonTiledTex;
//	nonTiledTex.x = (input.pos.x + terrainSize.x * terrainTileSize.x) / (terrainSize.x * terrainTileSize.x);
//	nonTiledTex.y = (input.pos.z - terrainSize.y * terrainTileSize.y) / -(terrainSize.y * terrainTileSize.y);
//
//	float4 blendTexCol	= texTerrainBlend.SampleLevel(samLinear, nonTiledTex,0);
//	float invTot		= 1.0f / (blendTexCol.x + blendTexCol.y + blendTexCol.z);
//
//	output.pos = mul(world, float4(input.pos,1.0f));
//	output.pos = mul(view, output.pos);
//	output.pos = mul(projection, output.pos);
//
//	float3 normal1 = normalize(texNorm1.SampleLevel(samLinear, input.tex,0).xyz * 2.0 - 1.0);
//	float3 normal2 = normalize(texNorm2.SampleLevel(samLinear, input.tex,0).xyz * 2.0 - 1.0);
//	float3 normal3 = normalize(texNorm3.SampleLevel(samLinear, input.tex,0).xyz * 2.0 - 1.0);
//
//	normal1 *= blendTexCol.x * invTot;
//	normal2 *= blendTexCol.y * invTot;
//	normal3 *= blendTexCol.z * invTot;
//	float3 totNormal = normalize( normal1 + normal2 + normal3);
//
//	//blue channel is up, we want green channel to be up
//	float3 temp = totNormal;
//	totNormal.g = temp.b;
//	totNormal.b = temp.g;
//
//
//	output.norm = mul((float3x3)world, normalize( input.norm.xyz + totNormal));
//	output.norm = normalize(mul((float3x3)view, output.norm));
//
//	output.tex = input.tex;
//
//	output.nonTiledTex = nonTiledTex;
//
//	return output;
//}
PS_TerrainInput VS_Terrain(VS_Input input)
{
	PS_TerrainInput output = (PS_TerrainInput)0;

	output.pos = mul(world, float4(input.pos,1.0f));
	output.pos = mul(view, output.pos);
	output.pos = mul(projection, output.pos);

	

	output.norm = input.norm;
	//output.norm = mul((float3x3)world, normalize( input.norm.xyz ));
	//output.norm = normalize(mul((float3x3)view, output.norm));

	output.tex = input.tex;

	output.nonTiledTex.x = (input.pos.x + terrainSize.x * terrainTileSize.x) / (terrainSize.x * terrainTileSize.x);
	output.nonTiledTex.y = (input.pos.z - terrainSize.y * terrainTileSize.y) / -(terrainSize.y * terrainTileSize.y);

	return output;
}
PS_MRTOutput PS_Terrain(PS_TerrainInput input)
{
	PS_MRTOutput output = (PS_MRTOutput)0;

	float4 blendTexCol	= texTerrainBlend.Sample(samLinear, input.nonTiledTex);
	float invTot		= 1.0f / (blendTexCol.x + blendTexCol.y + blendTexCol.z);

	float4 terCol0 = texTerrain0.Sample(samLinear, input.tex);
	float4 terCol1 = texTerrain1.Sample(samLinear, input.tex);
	float4 terCol2 = texTerrain2.Sample(samLinear, input.tex);

	terCol0 *= blendTexCol.x * invTot;
	terCol1 *= blendTexCol.y * invTot;
	terCol2 *= blendTexCol.z * invTot;

	output.diffuse = ( terCol0 + terCol1 + terCol2 );

	float3 normal1 = normalize(texNorm1.SampleLevel(samLinear, input.tex,0).xyz * 2.0 - 1.0);
	float3 normal2 = normalize(texNorm2.SampleLevel(samLinear, input.tex,0).xyz * 2.0 - 1.0);
	float3 normal3 = normalize(texNorm3.SampleLevel(samLinear, input.tex,0).xyz * 2.0 - 1.0);

	normal1 *= blendTexCol.x * invTot;
	normal2 *= blendTexCol.y * invTot;
	normal3 *= blendTexCol.z * invTot;
	float3 totNormal = normalize( normal1 + normal2 + normal3);

	//blue channel is up, we want green channel to be up
	float3 temp = totNormal;
	totNormal.g = temp.b;
	totNormal.b = temp.g;

	output.normal = float4(mul((float3x3)world, normalize( input.norm.xyz + totNormal )).xyz,0);
	output.normal = float4(normalize(mul((float3x3)view, output.normal)),0);

	output.normal.xyz = (output.normal + 1) * 0.5;
	if(output.normal.x<0 || output.normal.y<0 || output.normal.z<0)
		output.normal = float4(1,1,1,1);
	output.normal.w = input.pos.z;

	float4 spec0 = float4(0.9,0.9,0.9,0);
	float4 spec1 = float4(0.1,0.1,0.1,0);
	float4 spec2 = float4(0.06470588,0.0513725,0.0376470,0);

	spec0 *= blendTexCol.x * invTot;
	spec1 *= blendTexCol.y * invTot;
	spec2 *= blendTexCol.z * invTot;
	output.specular = ( spec0 + spec1 + spec2); //texTerrain0Spec.Sample( samLinear,input.tex );

	return output;
}
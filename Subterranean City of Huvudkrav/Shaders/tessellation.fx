#include "deferred.fx"
#include "resource.fx"

// http://stackoverflow.com/questions/16417088/dx11-tessellation-lod-with-diameter-incorrect-tessellation-values
Texture2D texBlend	: register( t0 );
Texture2D texMesh1	: register( t1 );
Texture2D texMesh2	: register( t2 );
Texture2D texMesh3	: register( t3 );
Texture2D texNorm1	: register( t10 );
Texture2D texNorm2	: register( t11 );
Texture2D texNorm3	: register( t12 );

struct VS_Input
{
	float3 pos	: POSITION;
	float3 norm	: NORMAL;
	float2 tex	: TEXCOORD0;
};
struct HS_Input
{
	float3 pos	: POSITION;
	float3 norm	: NORMAL;
	float2 tex	: TEXCOORD0;
	float2 nonTiledTex	: TEXCOORD1;
};
struct ConstantOutput
{
	float edges[3]	: SV_TESSFACTOR;
	float inside	: SV_INSIDETESSFACTOR;
};
struct HS_Output
{
	float3 pos	: POSITION;
	float3 norm	: NORMAL;
	float2 tex	: TEXCOORD0;
	float2 nonTiledTex	: TEXCOORD1;
};
struct PS_Input
{
	float4 pos			: SV_POSITION;
	float3 norm			: NORMAL;
	float2 tex			: TEXCOORD0;
	float2 nonTiledTex	: TEXCOORD1;
};
cbuffer cbTessellation	: register( b6 )
{
	float tessellationAmount;
	float3 padding;
};

HS_Input VS_Tessellation(VS_Input input)
{
	HS_Input output = (HS_Input)0;
	output.pos = input.pos;
	output.norm = input.norm;
	output.tex = input.tex;

	output.nonTiledTex.x = (input.pos.x + terrainSize.x * terrainTileSize.x) / (terrainSize.x * terrainTileSize.x);
	output.nonTiledTex.y = (input.pos.z - terrainSize.y * terrainTileSize.y) / -(terrainSize.y * terrainTileSize.y);
	return output;
}

ConstantOutput patchConstantFunction(InputPatch<HS_Input, 3> inputPatch, uint patchID : SV_PRIMITIVEID)
{
	ConstantOutput output = (ConstantOutput)0;
	output.edges[0] = tessellationAmount;
	output.edges[1] = tessellationAmount;
	output.edges[2] = tessellationAmount;
	output.inside = tessellationAmount;
	return output;
	//seems like calculating normals should be done here at least partially (and the rest in domainshader?) and then also the vertices should be moved here?
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("patchConstantFunction")]
HS_Output HS_Tessellation(InputPatch<HS_Input, 3> patch, uint pointID : SV_OUTPUTCONTROLPOINTID, uint patchID : SV_PRIMITIVEID)
{
	HS_Output output = (HS_Output)0;
	output.pos = patch[pointID].pos;
	output.norm = patch[pointID].norm;
	output.tex = patch[pointID].tex;
	output.nonTiledTex = patch[pointID].nonTiledTex;
	return output;
}

[domain("tri")]
PS_Input DS_Tessellation(ConstantOutput input, float3 uvw : SV_DOMAINLOCATION, const OutputPatch<HS_Output, 3> patch)
{
	float3 vertexPos;
	float3 vertexNorm;
	float2 vertexTex;
	float2 vertexNonTiledTex;
	PS_Input output = (PS_Input)0;
	//determine the position of the new vertex
	vertexPos = uvw.x * patch[0].pos + uvw.y * patch[1].pos + uvw.z * patch[2].pos;
	

	vertexNorm = normalize(uvw.x * patch[0].norm + uvw.y * patch[1].norm + uvw.z * patch[2].norm);
	vertexTex = uvw.x * patch[0].tex + uvw.y * patch[1].tex + uvw.z * patch[2].tex;
	vertexNonTiledTex = uvw.x * patch[0].nonTiledTex + uvw.y * patch[1].nonTiledTex + uvw.z * patch[2].nonTiledTex;

	float4 blendTexCol	= texBlend.SampleLevel(samLinear, vertexNonTiledTex,0);
	float invTot		= 1.0f / (blendTexCol.x + blendTexCol.y + blendTexCol.z);
	output.nonTiledTex = vertexNonTiledTex;
	float height1 = texNorm1.SampleLevel(samLinear, vertexTex,0).w * 20;
	float height2 = texNorm2.SampleLevel(samLinear, vertexTex,0).w * 10;
	float height3 = texNorm3.SampleLevel(samLinear, vertexTex,0).w * 15;

	height1 *= blendTexCol.x * invTot;
	height2 *= blendTexCol.y * invTot;
	height3 *= blendTexCol.z * invTot;

	float totHeight = height1 + height2 + height3;
	//heightmap in alpha channel
	//vertexPos += vertexNorm * (texNorm1.SampleLevel(samLinear, vertexTex,0).w ) * 20;
	vertexPos += vertexNorm * totHeight;
	output.pos = mul( world, float4(vertexPos,1));
	output.pos = mul( view, output.pos );
	output.pos = mul( projection, output.pos);

	float3 normal1 = normalize(texNorm1.SampleLevel(samLinear, vertexTex,0).xyz * 2.0 - 1.0);
	float3 normal2 = normalize(texNorm2.SampleLevel(samLinear, vertexTex,0).xyz * 2.0 - 1.0);
	float3 normal3 = normalize(texNorm3.SampleLevel(samLinear, vertexTex,0).xyz * 2.0 - 1.0);
	
	normal1 *= blendTexCol.x * invTot;
	normal2 *= blendTexCol.y * invTot;
	normal3 *= blendTexCol.z * invTot;
	float3 totNormal = normalize( normal1 + normal2 + normal3);
	//blue channel is up, we want green channel to be up
	float3 temp = totNormal;
	totNormal.g = temp.b;
	totNormal.b = temp.g;
	//output.norm = mul((float3x3)world, normalize(normalize(texNorm1.SampleLevel(samLinear, vertexNonTiledTex,0).xyz * 2.0 - 1.0)));
	output.norm = mul((float3x3)world, normalize( vertexNorm + totNormal ));
	output.norm = normalize(mul((float3x3)view, output.norm));

	output.tex = vertexTex;
	return output;
}

PS_MRTOutput PS_Tessellation(PS_Input input)
{
	PS_MRTOutput output = (PS_MRTOutput)0;

	float4 blendTexCol	= texBlend.Sample(samLinear, input.nonTiledTex);
	float invTot		= 1.0f / (blendTexCol.x + blendTexCol.y + blendTexCol.z);

	float4 terCol0 = texMesh1.Sample(samLinear, input.tex);
	float4 terCol1 = texMesh2.Sample(samLinear, input.tex);
	float4 terCol2 = texMesh3.Sample(samLinear, input.tex);

	terCol0 *= blendTexCol.x * invTot;
	terCol1 *= blendTexCol.y * invTot;
	terCol2 *= blendTexCol.z * invTot;

	output.diffuse = ( terCol0 + terCol1 + terCol2 );
	output.normal.xyz = (input.norm + 1) * 0.5;
	if(output.normal.x<0 || output.normal.y<0 || output.normal.z<0)
		output.normal = float4(1,1,1,1);
	output.normal.w = input.pos.z;
	
	//output.specular = float4(0.3,0.9,0.9,0);
	float4 spec0 = float4(0.9,0.9,0.9,0);
	float4 spec1 = float4(0.1,0.1,0.1,0);
	float4 spec2 = float4(0.06470588,0.0513725,0.0376470,0);

	spec0 *= blendTexCol.x * invTot;
	spec1 *= blendTexCol.y * invTot;
	spec2 *= blendTexCol.z * invTot;
	output.specular = ( spec0 + spec1 + spec2);

	return output;

}
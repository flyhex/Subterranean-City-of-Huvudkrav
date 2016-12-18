Texture2D texDiffuse	: register( t0 );
Texture2D texNormal		: register( t1 );
Texture2D texPosition	: register( t2 );
Texture2D texDepth		: register( t3 );
Texture2D texMesh		: register( t4 );

SamplerState samLinear	: register( s0 );

cbuffer cbObjs			: register( b0 )
{
	matrix world;
};
cbuffer cbCameraMove	: register( b1 )
{
	matrix view;
	float3 cameraPos;
};
cbuffer cbOnce			: register( b2 )
{
	matrix projection;
	matrix projInv;
};
cbuffer sbUserOptions	: register( b3 )
{
	int mrtTextureToRender;
	int mipmap; // cause needs to be at least 16 bytes
	int temp1;
	int temp2;
};

struct VS_Input
{
	float3 pos	: POSITION;
	float3 norm	: NORMAL;
	float3 tex	: TEXCOORD0;
};
struct PS_Input
{
	float4 pos	: SV_POSITION;
	float2 tex	: TEXCOORD0;
};
struct PS_MRTInput
{
	float4 pos		: SV_POSITION;
	float4 posWV	: TEXCOORD0;
	float3 norm		: TEXCOORD1;
	float2 tex		: TEXCOORD2;
};
struct PS_MRTOutput
{
	float4 diffuse	: SV_TARGET0;
	float4 normal	: SV_TARGET1;
	float4 pos		: SV_TARGET2;
	float4 depth	: SV_TARGET3;
};

//------------------vertex shader-----------------------//

// vertex shader for drawing the quad which will have the
// finished scene in it
float4 VS_Quad(VS_Input intput) : SV_POSITION
{
	return float4(0.2f,0.2f,0.2f,0.2f);
}
// vertex shader for drawing positions, normals and such to the
// different rendertargets
PS_MRTInput VS_MRT(VS_Input input)
{
	PS_MRTInput output = (PS_MRTInput)0;
	output.pos = mul(float4(input.pos,1.0f), world);
	output.posWV = mul(output.pos, view);
	output.pos = mul(output.posWV, projection);
	output.norm = mul( mul(input.norm, world), view );
	output.tex = input.tex;

	return output;
}

//-------------------geometry shaders------------------//

// geometry shader for expanding a quad onto to screen which will be
// used for drawing the finished scene on
[maxvertexcount(4)]
void GS_Quad( point float4 input[1] : SV_POSITION, inout TriangleStream<PS_Input> triStream )
{
	PS_Input output = (PS_Input)0;

	output.pos = float4( -1, -1, 0, 1 );
	output.tex = float2( 0, 1 );
	triStream.Append(output);

	output.pos = float4( -1, 1, 0, 1 );
	output.tex = float2( 0, 0 );
	triStream.Append(output);

	output.pos = float4( 1, -1, 0, 1);
	output.tex = float2( 1, 1 );
	triStream.Append(output);

	output.pos = float4( 1, 1, 0, 1 );
	output.tex = float2( 1, 0 );
	triStream.Append(output);
}

//-------------------pixel shader----------------------//

float4 PS_Quad(PS_Input input) : SV_TARGET0
{
	float4 outColor = float4(0,0,0,1);
	return outColor;

	float4 diffuse = texDiffuse.Sample( samLinear, input.tex );

	// user has specified to render only the diffuse texture
	if( mrtTextureToRender == 0 )
		return diffuse;

	float4 normal = texNormal.Sample( samLinear, input.tex );

	// user has specified to render only the normal texture
	if( mrtTextureToRender == 1 )
		return normal;
}

// pixel shader for drawing the stuff needed for deferred to the different 
// render targets
PS_MRTOutput PS_MRT(PS_MRTInput input)
{
	PS_MRTOutput output = (PS_MRTOutput)0;

	output.diffuse = texMesh.Sample(samLinear, input.tex);

	output.normal.xyz = input.norm * 0.5 + 0.5;
	output.normal.w = 0.0;

	output.pos = input.posWV * 0.5 + 0.5;

	return output;
}
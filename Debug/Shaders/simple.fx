Texture2D txDiffuse		: register( t0 );

SamplerState samLinear	: register( s0 );

cbuffer cbObjs			: register( b0 )
{
	matrix world;
	//matrix worldViewProj;
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
struct VS_MeshInput
{
	float3 pos	: POSITION;
	float3 norm : NORMAL;
	float2 tex	: TEXCOORD0;
};

struct PS_Input
{
	float4 pos		: SV_POSITION;
	float2 tex		: TEXCOORD0;
	float3 lightInt : LIGHTINT;
};

PS_Input VS_Mesh( VS_MeshInput input )
{
	PS_Input output = (PS_Input)0;

	output.pos = mul( float4(input.pos,1), world );
	output.pos = mul( output.pos, view );
	float4 eyeCoords = output.pos;
	output.pos = mul( output.pos, projection );
	output.tex = input.tex;
	float4 lightPos	= mul(float4(100,1000,300,1), view);
	float3 tnorm	= normalize(mul(mul(input.norm, world),view)).xyz;
	float3 s		= normalize(lightPos - eyeCoords).xyz;
	float3 ka		= float3(0.4f,0.4f,0.4f);
	float3 kd		= float3(0.4f,0.4f,0.4f);
	float3 ks		= float3(0.2f,0.2f,0.2f);
	float power		= 80;

	output.lightInt =	ka +
						kd * max( dot( s, tnorm ), 0.0 ) +
						ks * pow(
								max(
									dot( normalize( -s + 2 * dot( s, tnorm ) * tnorm ),
										normalize( mul( float4( cameraPos, 0.0 ), view ).xyz - eyeCoords.xyz ) ),
									0.0 ),
								power );
	return output;
}
float4 PS_Default( PS_Input input ) : SV_TARGET0
{
	//return txDiffuse.Sample(samLinear, input.tex);
	float alpha = 1.0f;
	return float4(1,1,1,1) * float4(input.lightInt, 1.0);
}
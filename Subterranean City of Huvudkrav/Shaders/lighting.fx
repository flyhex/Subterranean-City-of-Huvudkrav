#include "resource.fx"

Texture2D texDiffuse		: register( t0 );
Texture2D texNormal			: register( t1 );
Texture2D texSpecular		: register( t2 );
Texture2D texDepth			: register( t3 );
Texture2D texShadowMap		: register( t4 );
Texture2D texScene			: register( t5 );


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

//------------------- functions -------------------------//
float4 calculateLight(float3 s, float3 normal, float3 position, float3 cameraPosition )
{
	float4 outColor	=	float4(kd,0) * max(dot(s,normal),0.0);
	outColor +=			float4(ks,0) * pow(max(dot(normalize(-s + 2 *dot(s, normal) * normal),
							normalize( cameraPosition - position)), 0.0),40); 
	return outColor;
}


//------------------vertex shader-----------------------//

// vertex shader for drawing the quad which will have the
// finished scene in it
float4 VS_Quad(VS_Input input) : SV_POSITION
{
	return float4(0.2f,0.2f,0.2f,0.2f);
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
	float4 normal = texNormal.Sample( samLinear, input.tex );
	float depth = normal.w;

	float depthTest = texDepth.Sample( samLinear, input.tex).r;

	/*float r = texShadowMap.Sample( samLinear, input.tex).r;
	if(r == 1)
		discard;
	return float4(r,r,r,0);*/

	depth = depthTest;
	if(depth == 1)
		discard;
	if(mrtTextureToRender == 3)
		return float4(depthTest,depthTest,depthTest,0);
	normal.w = 0;

	float4 position;
	
	float4 outColor = float4(0,0,0,1);

	float4 diffuse = texDiffuse.Sample( samLinear, input.tex );

	
	// user has specified to render only the diffuse texture
	if( mrtTextureToRender == 0 )
		return diffuse;

	// user has specified to render only the normal texture
	if( mrtTextureToRender == 1 )
	{
		return normal;
	}

	normal.xyz = (normal.xyz - 0.5) * 2;

	float4 clipPos = float4(input.tex.x * 2.0 - 1.0,
						(1-input.tex.y) * 2.0 - 1.0,
						depth, 1);
	float4 viewPos = mul(projInv, clipPos);
	position = viewPos / viewPos.w;
	position.w = 1;//viewPos.w;

	float3 ka = float3(0.08,	0.08,	0.08);
	float3 ks = texSpecular.Sample(samLinear,input.tex).xyz;
	if( mrtTextureToRender == 2 )
		return float4(ks,1);
	float3 kd = float3(0.9280,0.9280,0.9280);

	int noLights = 1;
	float4 lightPos[3];
	lightPos[0]= mul(view, float4(200,1000,200,1));
	//lightPos[0]= mul(view, float4(-100,100,100, 1));
	lightPos[2]= mul(view, float4(1560,600,1560,1));
	lightPos[1]= mul(view, float4(3500,300,3500,1));
	//lightPos[3]= mul(view, float4(-2,12,6,1));
	//lightPos[4]= mul(view, float4(2,9,50,1));

	// we do light calculations in view space.
	float3 cameraPosWV = mul(view, float4(cameraPos,1.0)).xyz;
	float3 cameraDirWV = normalize(mul(view, float4(cameraDir,0.0)).xyz);
	for(int i = 0; i < noLights; i++)
	{
		float3 lightDist = lightPos[i].xyz - position.xyz;
		float lightLength = length(lightDist);
		float lightReach = 2000;
		if(lightLength < lightReach)
		{
			float3 s = normalize( lightDist );
			float lightFade = (lightReach - lightLength)/lightReach;
			outColor += (float4(kd,0) * max(dot(s,normal.xyz),0.0)
						+ float4(ks,0) * pow(max(dot(normalize(-s + 2 *dot(s, normal.xyz) * normal.xyz),
							normalize( cameraPosWV - position.xyz)), 0.0),40))
											* lightFade; 
		}
	}
	int noSpotLights	= 1;
	float cosTheta		= cos(PI/32); 
	float cosPhi		= cos(PI/4);  
	for(int i = 0; i < noSpotLights; i++)
	{
		//cameraPosWV is in this case the position of the light
		float3 lightDist = cameraPosWV - position.xyz;
		float lightLength = length(lightDist);
		float lightReach = 4000;
		if(lightLength < lightReach)
		{
			float3 s = normalize( lightDist );
			float attenuation = 0.0;
			//cameraDirWV is in this case the direction of the light
			float cosAlpha = max(dot(-s,cameraDirWV),0.0);
			if( cosAlpha > cosTheta )
				attenuation = 1.0;
			else if( cosAlpha > cosPhi )
				attenuation = pow( (cosAlpha - cosPhi) / (cosTheta - cosPhi ),20.0 ); 
			float lightFade = (lightReach - lightLength)/lightReach;
			outColor += (float4(kd,0) * max(dot(s,normal.xyz),0.0)
						+ float4(ks,0) * pow(max(dot(normalize(-s + 2 *dot(s, normal.xyz) * normal.xyz),
							normalize( cameraPosWV - position.xyz)), 0.0),40))
											* attenuation * lightFade; 
		}
	}

	int noShadowLights = 1;
	float4 shadowLightPos = mul(view, float4(-10,2000,4800,1)); //should be array of pos
	float bias = 0.0000002;
	
	
	for(int i = 0; i < noShadowLights; i++)
	{
		float2 shadowMapTexC;
		float4 posLightWVP = mul(viewInv, position);
		posLightWVP = mul(lightView, posLightWVP); // this pixels position seen from the light
		posLightWVP = mul(projection, posLightWVP);
		posLightWVP /= posLightWVP.w;
		
		shadowMapTexC.x = posLightWVP.x * 0.5 + 0.5;
		shadowMapTexC.y = -posLightWVP.y * 0.5 + 0.5;
		// if the texture coordinates are between 0 and 1, then this pixel
		// is within this lights view 
		//if( saturate(shadowMapTexC.x) == shadowMapTexC.x && saturate(shadowMapTexC.y) == shadowMapTexC.y )
		//{
			float dx = 1.0f/ (4*1280);
			float dy = 1.0f/ (4*720);

			float shadow;
			
			float s0= texShadowMap.Sample( samSkybox, shadowMapTexC).r + bias < posLightWVP.z ? 0.0 : 1.0;
			float s1= texShadowMap.Sample( samSkybox, shadowMapTexC + float2(-dx,0)).r + bias < posLightWVP.z ? 0.0 : 1.0;
			float s2= texShadowMap.Sample( samSkybox, shadowMapTexC + float2(dx,0)).r + bias < posLightWVP.z ? 0.0 : 1.0;
			float s3= texShadowMap.Sample( samSkybox, shadowMapTexC + float2(0,-dy)).r + bias < posLightWVP.z ? 0.0 : 1.0;
			float s4= texShadowMap.Sample( samSkybox, shadowMapTexC + float2(0,dy)).r + bias < posLightWVP.z ? 0.0 : 1.0;
			float s5= texShadowMap.Sample( samSkybox, shadowMapTexC + float2(-dx,-dy)).r + bias < posLightWVP.z ? 0.0 : 1.0;
			float s6= texShadowMap.Sample( samSkybox, shadowMapTexC + float2(dx,-dy)).r + bias < posLightWVP.z ? 0.0 : 1.0;
			float s7= texShadowMap.Sample( samSkybox, shadowMapTexC + float2(-dx,dy)).r + bias < posLightWVP.z ? 0.0 : 1.0;
			float s8= texShadowMap.Sample( samSkybox, shadowMapTexC + float2(dx,dy)).r + bias < posLightWVP.z ? 0.0 : 1.0;
			shadow = (s0 + s1 + s2 +s3 +s4 +s5 +s6 +s7 +s8 )/9 ;
			//float shadowDepth = texShadowMap.Sample( samSkybox, shadowMapTexC ).r;
			//float s0 = shadowDepth + bias < posLightWVP.z ? 0.0 : 1.0;
			//float lightDepth = posLightWVP.z - bias;
			//if(lightDepth < shadowDepth)
			//{
				//outColor += float4(0,1,0,0);
				float3 s = normalize( shadowLightPos.xyz - position.xyz);
				outColor += (float4(kd,0) * max(dot(s,normal.xyz),0.0)
								+ float4(ks,0) * pow(max(dot(normalize(-s + 2 *dot(s, normal.xyz) * normal.xyz),
									normalize( cameraPosWV - position.xyz)), 0.0),40)) * shadow;
			//}
		//}
	
		
	}
	float fogFactor = saturate((3000 - position.z) / (3000 - 1500));
	return saturate(outColor + float4(ka,0)) * diffuse * fogFactor
			+ (1 - fogFactor) * float4(0.38823,0.66274,0.51763,0); // add the ambient light and fog
}

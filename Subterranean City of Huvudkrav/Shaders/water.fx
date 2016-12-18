#include "lighting.fx"
#include "deferred.fx"

//http://mtnphil.wordpress.com/2012/09/23/water-shader-part-3-deferred-rendering/

Texture2D texTerrainBlend	: register( t0 );
Texture2D texTerrain0		: register( t1 );
Texture2D texTerrain1		: register( t2 );
Texture2D texTerrain2		: register( t3 );

Texture2D heightMap			:register( t6 );
Texture2D normalMap			:register( t7 );
Texture2D foamMap			:register( t8 );
Texture2D reflectionMap		:register( t9 );



struct PSrefl_Input
{
	float4 pos			: SV_POSITION;
	float4 posW			: POSW;
	float2 tex			: TEXCOORD;
	float2 nonTiledTex	: TEXCOORD1;
	float3 lightInt		: LIGHTINT;
};

float4x4 mReflection =
	{
		{ .5f, .0f, .0f, .5f },
		{ .0f, .5f, .0f, .5f },
		{ .0f, .0f, .0f, .5f },
		{ .0f, .0f, .0f, 1.0f }
	};

float3x3 computeTangentMatrix(float3 normal, float3 position, float2 texCoord)
{
	float3 dp1	= ddx(position);
	float3 dp2	= ddy(position);
	float2 duv1	= ddx(texCoord);
	float2 duv2	= ddy(texCoord);
	
	float3x3 m			= float3x3( dp1, dp2, cross( dp1, dp2 ) );
	float2x3 inverseM	= float2x3( cross( m[1], m[2] ), cross( m[2], m[0] ) );
	float3 tangent		= mul( float2( duv1.x, duv2.x ), inverseM );
	float3 biTangent	= mul( float2( duv1.y, duv2.y ), inverseM );
	
	return transpose(float3x3(normalize(tangent), normalize(biTangent), normal));
}

float fresnelTerm(float3 normal, float3 eyeDir)
{
		float angle		= 1.0f - saturate( dot( normal, eyeDir ) );
		float fresnel	= angle * angle;
		fresnel			= fresnel * fresnel;
		fresnel			= fresnel * angle;
		return saturate( fresnel * ( 1.0f - saturate( .5f ) ) + .5f - refractionStrength );
}

float4 PS_Water(PS_Input input) : SV_TARGET0
{
	float4 originalColor	= texScene.Sample( samLinear, input.tex );
	if(cameraPos.y < waterLevel) //save work if submerged
		return texScene.Sample( samSkybox, float2( input.tex.x + 2*sin(timer * 0.002f + 3.0f * abs(input.tex.y)) * (refractionScale * min(input.tex.y, 1.0f)), input.tex.y ) ) + float4(0,0,0.3f,1);
	
	float depth				= texDepth.Sample( samLinear, input.tex ).r;
	float4 clipPos = float4(input.tex.x * 2.0 - 1.0,
						(1-input.tex.y) * 2.0 - 1.0,
						depth, 1);
	float4 viewPos = mul(projInv, clipPos);
	float4 worldPos = mul(viewInv, float4( (viewPos / viewPos.w).xyz,1) );

	if(worldPos.y > waterLevel + maxAmplitude)//if the pixel is above the water surface, then discard.
		return originalColor;

	float level = waterLevel;
	//find the position of the point where the pixel meets the surface of the water.
	float3 eyeDir = normalize(worldPos - cameraPos);
	float distanceFromSurface = level - worldPos.y;
	float distanceFromCamera = cameraPos.y - worldPos.y;

	float t = (level - cameraPos.y) / eyeDir.y;
	float3 surfacePos = cameraPos + eyeDir * t;

	float2 texCoord;
	int temp;
	//calculate the position in regards to waves.
	for(int i = 0; i < 10; i++)
	{
		texCoord = (surfacePos.xz + eyeDir.xz * 0.1f) * scale + timer * 0.000005f * windDirection;

		float bias = heightMap.Sample( samLinear, texCoord ).r;

		bias *= 0.1f;
		level += bias * maxAmplitude;
		t = (level - cameraPos.y) / eyeDir.y;
		surfacePos = cameraPos + eyeDir * t;
	}


	depth = length(worldPos - surfacePos);
	float depth2 = surfacePos.y - worldPos.y;
	eyeDir = normalize(cameraPos - surfacePos);
	//calculate normal

	float normal1 = heightMap.Sample( samLinear, (texCoord + float2(-1,0)) / 256).r;
	float normal2 = heightMap.Sample( samLinear, (texCoord + float2(1,0)) / 256).r;
	float normal3 = heightMap.Sample( samLinear, (texCoord + float2(0,-1)) / 256).r;
	float normal4 = heightMap.Sample( samLinear, (texCoord + float2(0,1)) / 256).r;

	float3 surfaceNormal = normalize( float3( (normal1 - normal2) * maxAmplitude,
												normalScaling,
												(normal3 - normal4) * maxAmplitude) );
	//calculate tangent
	texCoord			= (surfacePos.xz * 0.016 + windDirection * timer * 0.00016);
	float3x3 mTangent	= computeTangentMatrix(surfaceNormal, eyeDir, texCoord);
	float3 normal0a		= normalize( mul(mTangent, 2.0f * normalMap.Sample( samLinear, texCoord ) - 1.0f ) );

	texCoord			= (surfacePos.xz * 0.008 + windDirection * timer * 0.00008);
	 mTangent			= computeTangentMatrix(surfaceNormal, eyeDir, texCoord);
	float3 normal1a		= normalize( mul(mTangent, 2.0f * normalMap.Sample( samLinear, texCoord ) - 1.0f ) );

	texCoord			= (surfacePos.xz * 0.004 + windDirection * timer * 0.00004);
	 mTangent			= computeTangentMatrix(surfaceNormal, eyeDir, texCoord);
	float3 normal2a		= normalize( mul(mTangent, 2.0f * normalMap.Sample( samLinear, texCoord ) - 1.0f ) );

	texCoord			= (surfacePos.xz * 0.001 + windDirection * timer * 0.00002);
	 mTangent			= computeTangentMatrix(surfaceNormal, eyeDir, texCoord);
	float3 normal3a		= normalize( mul(mTangent, 2.0f * normalMap.Sample( samLinear, texCoord ) - 1.0f ) );

	float3 normal		= normalize(normal0a * normalModifier.x + normal1a * normalModifier.y + normal2a * normalModifier.z + normal3a * normalModifier.w);

	//calculate refraction
	texCoord	= input.tex;
	texCoord.x	+= sin(timer * 0.002f + 1.0f * abs(worldPos.y)) * (refractionScale * min(depth2, 1.0f));

	float3 refraction	= texScene.Sample( samLinear, texCoord ).rgb;
	float depthRefr		= texDepth.Sample( samLinear, texCoord ).r;

	float4 clipPosRefr	= float4(texCoord.x * 2.0 - 1.0,
						(1-texCoord.y) * 2.0 - 1.0,
						depthRefr, 1);
	float4 viewPosRefr	= mul(projInv, clipPosRefr);
	float4 worldPosRefr	= mul(viewInv, float4( (viewPosRefr / clipPosRefr.w).xyz,1) );

	if( worldPosRefr.y < level)
		refraction = originalColor;

	float4x4 mTextureProj	= mul( mul(view,projection), mReflection);
	float3 waterPos			= surfacePos.xyz - level + waterLevel;
	float4 texCoordProj		= mul(mTextureProj, float4(waterPos,1.0f));

	float4 disPos			= float4( texCoordProj.x + displacementStrength * normal.x,
							texCoordProj.y,
							texCoordProj.z + displacementStrength * normal.z,
							texCoordProj.w );
	texCoordProj = disPos;

	//float3 reflection		= reflectionMap.Sample(samLinear, texCoordProj);
	float3 reflection		= reflectionMap.Sample(samLinear, float2(1-texCoord.x, texCoord.y));

	float fresnel = fresnelTerm(normal, eyeDir);

	float depthN = depth * waterFade;
	float3 waterColor = saturate(length(float3(1.0f,1.0f,1.0f)) / 3.0f );
	refraction = lerp(lerp(refraction, waterSurfaceColor * waterColor, 1-saturate(depthN / transparency)),
							waterDepthColor * waterColor, saturate(depth2 / extinction));
	//refraction = lerp(refraction, waterSurfaceColor * waterColor, 0.5);
	//refraction = lerp(lerp(refraction,waterDepthColor * waterColor, 0.5), waterSurfaceColor * waterColor, saturate(depth2 / extinction*0.00001));

	float foam = 0.0f;

	texCoord			= (surfacePos.xz + eyeDir.xz * 0.1) * 0.005 + timer * 0.00001f * windDirection + sin(timer * 0.001 + worldPos.x) * 0.000005;
	float2 texCoord2	= (surfacePos.xz + eyeDir.xz * 0.1) * 0.005 + timer * 0.00002f * windDirection + sin(timer * 0.001 + worldPos.z) * 0.000005;

	if(depth2 < foamOptions.x)
		foam = ( foamMap.Sample(samLinear, texCoord) + foamMap.Sample(samLinear, texCoord2) ) * 0.5f;
	else if(depth2 < foamOptions.y)
	{
		foam = lerp((foamMap.Sample(samLinear,texCoord) + foamMap.Sample(samLinear,texCoord2)) * 0.5f, 0.0f, (depth2 - foamOptions.x) / (foamOptions.y - foamOptions.x));
	}
	if(maxAmplitude - foamOptions.z > 0.0001f)
	{
		foam += (foamMap.Sample(samLinear, texCoord) + foamMap.Sample(samLinear, texCoord2)) * 0.05f * saturate((level - (waterLevel + foamOptions.z)) / (maxAmplitude - foamOptions.z));
	}

	/*float3 specular = float3(0.0f, 0.0f, 0.0f);
	float3 mirror	= (2.0f * dot(eyeDir, normal) * normal - eyeDir);
	float3 lightDist = mul(view, float4(-10,2000,4800,1)).xyz - float4(( viewPosRefr.xyz/viewPosRefr.w),1).xyz;
	float3 s = normalize( lightDist );
	float dotSpec	= saturate(dot(mirror.xyz, -s) * 0.5f + 0.5f);
	specular = (1.0f - fresnel) * saturate(-s.y) * (pow(dot(dotSpec, 512.0f),40) * (0.7f * 1.8f + 0.2f)) * float3(1,1,1);
	specular += specular * 25 * saturate(0.7f - 0.05f) * float3(1,1,1);

	return float4(lerp(refraction, saturate( lerp(refraction, reflection, fresnel) + max( specular,foam * float3(1,1,1) ) ), saturate( depth * shoreTransition ) ), 1.0f);*/

	float3 tnorm = mul(view, float4(normal,0) ).xyz;
	float3 lightDist = mul(view, float4(-10,2000,4800,1)).xyz - float4(( viewPosRefr.xyz/viewPosRefr.w),1).xyz;
	float3 s = normalize( lightDist );
	float3 cameraPosWV	= mul( view, float4(cameraPos,1.0) ).xyz;
	float3 specular = pow(max(dot(normalize(-s + 2 *dot(s, tnorm ) * tnorm ), normalize( cameraPosWV - float4(( viewPosRefr.xyz/viewPosRefr.w),1).xyz )), 0.0),40);
	//return float4(reflection,1.0f);
	return saturate(lerp(originalColor, lerp(float4(refraction,1.0f), float4(reflection,1.0f),  0.7*fresnel) + float4(specular,1) + foam ,saturate(depth2 * shoreTransition)));

	//return saturate(lerp(originalColor, float4(refraction,1.0f) + float4(specular,1) + foam ,saturate(depth2 * shoreTransition)));
	//return float4( 0.21f, 0.35f, 0.41f, 1.0f );




	/*float3	specular	= (0,0,0);
	int		noLights	= 1;
	float4	lightPos[3];
	lightPos[0]			= mul( view, float4(-10,2000,4800,1) );
	float3 cameraPosWV	= mul( view, float4(cameraPos,1.0) ).xyz;

	for(int i = 0; i < noLights; i++)
	{
		float3 lightDist = lightPos[i].xyz - ( viewPosRefr.xyz / viewPosRefr.w ).xyz;
		float3 s = normalize( lightDist );
		specular += pow(max(dot(normalize(-s + 2 *dot(s, normal.xyz) * normal.xyz),
						normalize( cameraPosWV - ( viewPosRefr.xyz / viewPosRefr.w ).xyz)), 0.0),40);
	}
	return lerp(originalColor, float4(refraction,1.0f) ,saturate(depth2 * shoreTransition) + float4(specular,1));*/
}

PSrefl_Input VS_Reflection(VS_Input input)
{
	PSrefl_Input output = (PSrefl_Input)0;
	output.posW = mul(world,float4(input.pos,1));

	float3 kd = float3(0.9280,0.9280,0.9280);
	float3 ka = float3(0.08,	0.08,	0.08);

	float3 s = normalize(float3(-10,2000,4800)-output.pos.xyz);

	output.lightInt = saturate( ka + (float4(kd,0) * max(dot(s,input.norm),0.0)) );

	output.pos = mul(viewRefl, output.posW);
	output.pos = mul(projection, output.pos);

	output.nonTiledTex.x = (input.pos.x + terrainSize.x * terrainTileSize.x) / (terrainSize.x * terrainTileSize.x);
	output.nonTiledTex.y = (input.pos.z - terrainSize.y * terrainTileSize.y) / -(terrainSize.y * terrainTileSize.y);

	output.tex = input.tex;
	return output;
}

float4 PS_Reflection(PSrefl_Input input) : SV_TARGET0
{
	if(input.posW.y < waterLevel - maxAmplitude)
		discard;

	float4 blendTexCol	= texTerrainBlend.Sample(samLinear, input.nonTiledTex);
	float invTot		= 1.0f / (blendTexCol.x + blendTexCol.y + blendTexCol.z);

	float4 terCol0 = texTerrain0.Sample(samLinear, input.tex);
	float4 terCol1 = texTerrain1.Sample(samLinear, input.tex);
	float4 terCol2 = texTerrain2.Sample(samLinear, input.tex);

	terCol0 *= blendTexCol.x * invTot;
	terCol1 *= blendTexCol.y * invTot;
	terCol2 *= blendTexCol.z * invTot;

	float4 col = ( terCol0 + terCol1 + terCol2 );

	float fogFactor = saturate((3000 - mul(view, input.posW).z) / (3000 - 1500));

	return col * float4(input.lightInt,1) * fogFactor + (1 - fogFactor) * float4(0.38823,0.66274,0.51763,0);
	//float4 normal = texNormal.Sample( samLinear, input.tex );
	//float depth = normal.w;
	//float4 clipPos = float4(input.tex.x * 2.0 - 1.0,
	//					(1-input.tex.y) * 2.0 - 1.0,
	//					depth, 1);
	//float4 viewPos = mul(projInv, clipPos);
	//float4 worldPos = mul(viewInv, float4( (viewPos / viewPos.w).xyz,1) );

	//if(worldPos.y < waterLevel)//if the pixel is above the water surface, then discard.
	//	discard;
	//return PS_Quad(input);
}
//#include "resource.fx"
//#include "deferred.fx"
//
//Texture2D map_normal		: register( t0 );					//normal map / bump / disp
//Texture2D env_reflect		: register( t1 );					//reflection
//Texture2D env_refract		: register( t2 );					//refraction
//
//
//struct VS_Input
//{
//	float3 pos	: POSITION;
//	float3 norm	: NORMAL;
//	float2 tex	: TEXCOORD0;
//};
//
//struct PS_Input
//{
//	float4 pos				: SV_POSITION;
//	float3 norm				: NORMAL;
//	float4 reflectSampPos	: TEXCOORD1;
//	float2 normMapSampPos	: TEXCOORD2;
//	float4 refractSampPos	: TEXCOORD3;
//	float4 pos3D			: TEXCOORD4;
//};
//
//PS_Input VS_Water(VS_Input input)
//{
//	PS_Input output = (PS_Input)0;
//
//	output.pos = mul(world, float4(input.pos,1.0f));
//	output.pos = mul(view, output.pos);
//	output.pos = mul(projection, output.pos);
//
//	output.norm = input.norm;
//
//	return output;
//}
//
//PS_MRTOutput PS_Water(PS_Input input)
//{
//	PS_MRTOutput output = (PS_MRTOutput)0;
//	output.diffuse = float4(0,0.15,0.5,1.0);
//
//	output.normal.xyz = (input.norm + 1) * 0.5;
//	if(output.normal.x<0 || output.normal.y<0 || output.normal.z<0)
//		output.normal = float4(1,1,1,1);
//	output.normal.w = input.pos.z;
//
//	output.specular = float4(0.9,0.9,0.9,1.0);
//
//	return output;
//}
#include "lighting.fx"

Texture2D texBlurredScene	: register( t6 );

float4 PS_Blur(PS_Input input) : SV_TARGET0
{
	float4 outColor = texScene.Sample(samLinear,input.tex);;
	float blurDist = 0.005; //* (texDepth.Sample( samLinear, input.tex ).r);

	float depth = texDepth.Sample( samLinear, input.tex ).r;
	float distance = texDepth.Sample( samLinear, float2(0.5,0.5) ).r;
	int noTimes;
	// ok so this is kinda not good -- the blurFactor is calculated many many times
	// unnecessarily, should maybe write to a rendertarget before the blur?
	// onwards! so all this extra stuff is to counteract the problem mentioned in the 
	// def shader, where for example a treebranch in focus looks blury because the terrain
	// behind it is blurred with that branch. 
	// if a pixel is closer to the camera than the focus distance + some distance then it should be blurred
	// normally. Else only blur it with pixels that are not in focus 
	if(depth < distance - 0.00005)
	{
		outColor +=  texScene.Sample(samSkybox, float2(input.tex.x + blurDist, input.tex.y + blurDist) );
		outColor += texScene.Sample(samSkybox, float2(input.tex.x - blurDist, input.tex.y - blurDist) );
		outColor += texScene.Sample(samSkybox, float2(input.tex.x + blurDist, input.tex.y - blurDist) );
		outColor += texScene.Sample(samSkybox, float2(input.tex.x - blurDist, input.tex.y + blurDist) );
		//outColor /= 5;
		noTimes = 5;
	}
	else
	{
		float range = 0.0007;
		noTimes = 1;

		float2 texC = float2(input.tex.x + blurDist, input.tex.y + blurDist);
		depth			= texDepth.Sample( samLinear, texC ).r;
		float blurFactor = saturate( abs( depth - distance )  / range );
		if(blurFactor > 0.5)
		{
			outColor += texScene.Sample(samLinear,texC);
			noTimes++;
		}
		texC = float2(input.tex.x - blurDist, input.tex.y - blurDist);
		depth			= texDepth.Sample( samLinear, texC ).r;
		blurFactor = saturate( abs( depth - distance )  / range );
		if(blurFactor > 0.5)
		{
			outColor += texScene.Sample(samLinear,texC);
			noTimes++;
		}
		texC = float2(input.tex.x + blurDist, input.tex.y - blurDist);
		depth			= texDepth.Sample( samLinear, texC ).r;
		blurFactor = saturate( abs( depth - distance )  / range );
		if(blurFactor > 0.5)
		{
			outColor += texScene.Sample(samLinear,texC);
			noTimes++;
		}
		texC = float2(input.tex.x - blurDist, input.tex.y + blurDist);
		 depth			= texDepth.Sample( samLinear, texC ).r;
		blurFactor = saturate( abs( depth - distance )  / range );
		if(blurFactor > 0.5)
		{
			outColor += texScene.Sample(samLinear,texC);
			noTimes++;
		}
	}
	outColor/=noTimes;
	return outColor;
}
float4 PS_DepthOfField(PS_Input input) : SV_TARGET0
{
	float depth			= texDepth.Sample( samLinear, input.tex ).r;
	float4 scene		= texScene.Sample(samLinear, input.tex);
	float4 blurredScene	= texBlurredScene.Sample(samLinear, input.tex);

	
	//this means that if something dont have a depth dont blur it
	//so that for example the skybox isn't blurred
	//if(depth == 1)
		//return scene;
	float distance = texDepth.Sample( samLinear, float2(0.5,0.5) ).r ;
	float range = 0.0007;
	float near = 0.1;
	float far = 8000;

	// the reason it still gets blurry on the trees even when blurFactor is 0 on a whole branch is
	// ( i think ) because the blur on the terrain beneath is blurred with the tree and so it only looks 
	// like the tree is blurred, but it is the terrain. prob.
	
	//depth = 1-depth;
	//float sceneZ = ( -near * far ) / ( depth - far );
	float blurFactor = saturate( abs( depth - distance )  / range );
	if(mrtTextureToRender == 3)
		return float4(blurFactor,blurFactor,blurFactor,0);//blurredScene;
	return lerp(scene, blurredScene, blurFactor);
}
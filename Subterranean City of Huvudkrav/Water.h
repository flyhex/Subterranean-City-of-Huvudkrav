#pragma once
#include "Resource.h"
class Water
{
private:
	float waterLevel;
	double timer;
	XMFLOAT2 windDirection;

	float waterFade;
	float normalScaling;
	float maxAmplitude;
	float shoreTransition;
	float refractionStrength;
	XMFLOAT4 normalModifier;
	float displacementStrength;
	XMFLOAT3 foamOptions;

	float shininess;
	float specularIntensity;

	XMFLOAT3 waterSurfaceColor;
	XMFLOAT3 waterDepthColor;
	XMFLOAT3 extinction;

	float transparency;
	XMFLOAT2 scale;
	float refractionScale;
public:
	Water();
	void update(double dt);
	void updateCBWater();
	void getReflectionView( XMFLOAT4X4 &out, XMFLOAT4X4 &view);
	~Water();
};


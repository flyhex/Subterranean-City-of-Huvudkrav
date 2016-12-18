//the output for mutliple render targets (deferred rendering)
struct PS_MRTOutput
{
	float4 diffuse	: SV_TARGET0;
	float4 normal	: SV_TARGET1;
	float4 specular	: SV_TARGET2;
	float4 depth	: SV_TARGET3;
};
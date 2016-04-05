Texture2D colorMap : register(t0);
SamplerState colorSampler : register(s0);

cbuffer cbSurfaceColor : register(b0)
{
	float4 surfaceColor;
}

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 tex: TEXCOORD0;
};

float4 main(PSInput i) : SV_TARGET
{
	float4 mappedColor = colorMap.Sample(colorSampler, i.tex);
	return saturate(mappedColor + surfaceColor);
}
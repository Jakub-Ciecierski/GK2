Texture2D colorMap1 : register(t0);
Texture2D colorMap2 : register(t1);
SamplerState colorSampler : register(s0);

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 tex1: TEXCOORD0;
	float2 tex2: TEXCOORD1;
};

float4 main(PSInput i) : SV_TARGET
{
	float4 c1 = colorMap1.Sample(colorSampler, i.tex1);
	float4 c2 = colorMap2.Sample(colorSampler, i.tex2);
	return c1 * (1 - c2.a) + c2 * c2.a;
}
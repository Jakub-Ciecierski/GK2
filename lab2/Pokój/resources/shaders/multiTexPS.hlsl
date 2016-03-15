Texture2D colorMap : register(t0);
//TODO: Add second texture

SamplerState colorSampler : register(s0);

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 tex: TEXCOORD0;
	//TODO: Add texture coordinate for second texture

};

float4 main(PSInput i) : SV_TARGET
{
	float4 c = colorMap.Sample(colorSampler, i.tex);
	//TODO: Sample second texture and mix the two colors

	return c;
}
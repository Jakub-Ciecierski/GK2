Texture2D colorMap : register(t0);
Texture2D posterMap : register(t1);
//TODO: Add second texture DONE

SamplerState colorSampler : register(s0);

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 tex: TEXCOORD0;
	float2 tex1: TEXCOORD1; //TODO: Add texture coordinate for second texture DONE
};

float4 main(PSInput i) : SV_TARGET
{
	float4 c1 = colorMap.Sample(colorSampler, i.tex);
	float4 c2 = posterMap.Sample(colorSampler, i.tex1);

	//TODO: Sample second texture and mix the two colors DONE
	return c1*(1 - c2.a) + c2*c2.a;
}
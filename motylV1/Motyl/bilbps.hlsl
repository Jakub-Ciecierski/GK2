cbuffer cbSurfaceColor : register(b0) //Pixel Shader constant buffer slot 0
{
	float4 surfaceColor;
}

struct PSBilboardInput
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
};


float4 main(PSBilboardInput i) : SV_TARGET
{
	float d = sqrt(i.tex.x*i.tex.x + i.tex.y*i.tex.y);
	d = clamp(1 - d, 0.0f, 1.0f);
	return surfaceColor*d;
}
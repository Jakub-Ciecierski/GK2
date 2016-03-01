/*Texture2D colorMap : register(t0);
SamplerState colorSampler : register(s0);*/

cbuffer cbWorld : register(b0) //Vertex Shader constant buffer slot 0
{
	matrix worldMatrix;
};

cbuffer cbView : register(b1) //Vertex Shader constant buffer slot 1
{
	matrix viewMatrix;
	matrix invViewMatrix;
};

cbuffer cbProj : register(b2) //Vertex Shader constant buffer slot 2
{
	matrix projMatrix;
};

struct VSBilboardInput
{
	float3 pos : POSITION;
};

struct PSBilboardInput
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
};


PSBilboardInput main(VSBilboardInput i)
{
	PSBilboardInput o = (PSBilboardInput)0;
	o.pos = float4(i.pos, 1.0f);
	float4 zero = float4(0.0f, 0.0f, 0.0f, 1.0f);
	zero = -mul(invViewMatrix, zero);
	o.pos = mul(invViewMatrix, o.pos);
	o.pos += zero;
	o.pos.w = 1.0f;
	o.pos = mul(worldMatrix, o.pos);
	o.pos = mul(viewMatrix, o.pos);
	o.pos = mul(projMatrix, o.pos);
	o.tex = i.pos.xy;
	return o;
}
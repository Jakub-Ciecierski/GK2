cbuffer cbWorld : register(b0)
{
	matrix worldMatrix;
};

cbuffer cbView : register(b1)
{
	matrix viewMatrix;
};

cbuffer cbProj : register(b2)
{
	matrix projMatrix;
};

cbuffer cbTextureTransform : register(b3)
{
	matrix texMatrix;
};

//TODO: Add buffer for second texture transform DONE
cbuffer cbTextureTransform2 : register(b4)
{
	matrix texMatrix2;
};

struct VSInput
{
	float3 pos : POSITION;
	float3 norm : NORMAL0;
};

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 tex: TEXCOORD0;
	float2 tex2: TEXCOORD2; //TODO: Add texture coordinate for second texture DONE
};

PSInput main(VSInput i)
{
	PSInput o = (PSInput)0;
	o.pos = float4(i.pos, 1.0f);

	o.tex = mul(texMatrix, o.pos).xy;
	o.tex2 = mul(texMatrix2, o.pos).xy; //TODO: Calculate second texture coordinate
	
	o.pos = mul(worldMatrix, o.pos);
	o.pos = mul(viewMatrix, o.pos);
	o.pos = mul(projMatrix, o.pos);

	return o;
}
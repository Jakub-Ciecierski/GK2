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

float4 VS_Main(float4 pos : POSITION) : SV_POSITION
{
	return pos;
}

float4 PS_Main(float4 pos : SV_POSITION) : SV_TARGET
{
	return float4(0.0f, 1.0f, 0.0f, 1.0f);
}
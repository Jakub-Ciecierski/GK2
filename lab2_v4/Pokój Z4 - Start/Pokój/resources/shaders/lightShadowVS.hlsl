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

cbuffer cbLightVS : register(b3)
{
	float4 lightPosVS;
};

struct VSInput
{
	float3 pos : POSITION;
	float3 norm : NORMAL0;
};

struct PSInput
{
	float4 pos : SV_POSITION;
	float3 norm : NORMAL;
	float4 worldPos : POSITION;
	float3 viewVec : TEXCOORD0;
	float3 lightVec : TEXCOORD1;
};

PSInput main(VSInput i)
{
	PSInput o = (PSInput)0;
	matrix worldView = mul(viewMatrix, worldMatrix);
	o.worldPos = float4(i.pos, 1.0f);
	o.worldPos = mul(worldMatrix, o.worldPos);


	float4 viewPos = mul(viewMatrix, o.worldPos);
	o.pos = mul(projMatrix, viewPos);

	o.norm = mul(worldView, float4(i.norm, 0.0f)).xyz;
	o.norm = normalize(o.norm);

	o.viewVec = normalize(-viewPos.xyz);
	o.lightVec = normalize((mul(viewMatrix, lightPosVS) - viewPos).xyz);
	return o;
}
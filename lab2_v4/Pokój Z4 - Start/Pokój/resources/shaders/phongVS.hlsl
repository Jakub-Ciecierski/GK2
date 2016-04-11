cbuffer cbWorld : register(b0) //Vertex Shader constant buffer slot 0
{
	matrix worldMatrix;
};

cbuffer cbView : register(b1) //Vertex Shader constant buffer slot 1
{
	matrix viewMatrix;
};

cbuffer cbProj : register(b2) //Vertex Shader constant buffer slot 2
{
	matrix projMatrix;
};

cbuffer cbLights : register(b3)
{
	float4 lightPos;
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
	float3 viewVec : TEXCOORD0;
	float3 lightVec : TEXCOORD1;
};

PSInput main(VSInput i)
{
	PSInput o = (PSInput)0;
	matrix worldView = mul(viewMatrix, worldMatrix);
	float4 viewPos = float4(i.pos, 1.0f);
	viewPos = mul(worldView, viewPos);
	o.pos = mul(projMatrix, viewPos);

	o.norm = mul(worldView, float4(i.norm, 0.0f)).xyz;
	o.norm = normalize(o.norm);

	o.viewVec = normalize(-viewPos.xyz);
	o.lightVec = normalize((mul(viewMatrix, lightPos) - viewPos).xyz);
	return o;
}
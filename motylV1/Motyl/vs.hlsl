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

cbuffer cbLightPos : register(b3) //Vertex Shader constant buffer slot 3
{
	float4 lightPos[3];
}

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
	float3 lightVec0 : TEXCOORD1;
	float3 lightVec1 : TEXCOORD2;
	float3 lightVec2 : TEXCOORD3;
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

	o.lightVec0 = normalize((mul(viewMatrix, lightPos[0]) - viewPos).xyz);
	o.lightVec1 = normalize((mul(viewMatrix, lightPos[1]) - viewPos).xyz);
	o.lightVec2 = normalize((mul(viewMatrix, lightPos[2]) - viewPos).xyz);
	return o;
}
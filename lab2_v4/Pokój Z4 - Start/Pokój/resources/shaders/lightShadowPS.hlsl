Texture2D lightMap : register(t0);
Texture2D shadowMap : register(t1);
SamplerState colorSampler : register(s0);

cbuffer cbSurfaceColor : register(b0)
{
	float4 surfaceColor;
};

cbuffer cbMapTransform : register(b1)
{
	matrix mapMatrix;
};

cbuffer cbLightPS : register(b2)
{
	float4 lightPosPS;
};

struct PSInput
{
	float4 pos : SV_POSITION;
	float3 norm : NORMAL;
	float4 worldPos : POSITION;
	float3 viewVec : TEXCOORD0;
	float3 lightVec : TEXCOORD1;
};

static const float3 ambientColor = float3(0.3f, 0.3f, 0.3f);
static const float3 kd = 0.7, m = 100.0f;
static const float4 defLightColor = float4(0.3f, 0.3f, 0.3f, 0.0f);

float4 main(PSInput i) : SV_TARGET
{
	float4 NDCPos;
	//TODO: calculate texture coordinates DONE
	NDCPos = mul(mapMatrix, i.worldPos);
	NDCPos.x /= NDCPos.w;
	NDCPos.y /= NDCPos.w;
	NDCPos.z /= NDCPos.w;
	
	float3 viewVec = normalize(i.viewVec);
	float3 normal = normalize(i.norm);
	float3 lightVec = normalize(i.lightVec);
	float3 halfVec = normalize(viewVec + lightVec);
	float3 color = surfaceColor.rgb * ambientColor;
	
	//TODO: determine light color based on light map, clipping plane and shadow map. DONE
	float4 sampledLightColor = lightMap.Sample(colorSampler, NDCPos.xy);
	float maxColor = max(sampledLightColor.x, defLightColor.x);
	float4 lightColor = float4(maxColor, maxColor, maxColor, 0);

	if (i.worldPos.y > lightPosPS.y)
		lightColor = defLightColor;

	float shadowSample = shadowMap.Sample(colorSampler, NDCPos.xy);
	if(shadowSample < NDCPos.z)
		lightColor = defLightColor;

	color += lightColor.rgb * surfaceColor.rgb * kd * saturate(dot(normal, lightVec)) +
		lightColor.rgb * lightColor.a * pow(saturate(dot(normal, halfVec)), m);
	return float4(saturate(color), surfaceColor.a);
}
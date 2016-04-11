cbuffer cbSurfaceColor : register(b0)
{
	float4 surfaceColor;
}

struct PSInput
{
	float4 pos : SV_POSITION;
	float3 norm : NORMAL;
	float3 viewVec : TEXCOORD0;
	float3 lightVec : TEXCOORD1;
};

static const float3 ambientColor = float3(0.2f, 0.2f, 0.2f);
static const float3 lightColor = float3(1.0f, 1.0f, 1.0f);
static const float3 kd = 0.5, ks = 0.2f, m = 100.0f;

float4 main(PSInput i) : SV_TARGET
{
	float3 viewVec = normalize(i.viewVec);
	float3 normal = normalize(i.norm);
	float3 lightVec = normalize(i.lightVec);
	float3 halfVec = normalize(viewVec + lightVec);
	float3 color = surfaceColor.rgb * ambientColor;
	color += lightColor * surfaceColor.rgb * kd * saturate(dot(normal, lightVec)) +
		lightColor * ks * pow(saturate(dot(normal, halfVec)), m);
	if (!surfaceColor.a)
		discard;
	return float4(saturate(color), surfaceColor.a);
}
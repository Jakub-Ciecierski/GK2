cbuffer cbSurfaceColor : register(b0) //Pixel Shader constant buffer slot 0
{
	float4 surfaceColor;
}

cbuffer cbLightColors : register(b1) //Pixel Shader constant buffer slot 1
{
	float4 aC;
	float4 surface; //[ka, kd, ks, m]
	float4 lightColor[3];
}

struct PSInput
{
	float4 pos : SV_POSITION;
	float3 norm : NORMAL;
	float3 viewVec : TEXCOORD0;
	float3 lightVec0 : TEXCOORD1;
	float3 lightVec1 : TEXCOORD2;
	float3 lightVec2 : TEXCOORD3;
};

float4 main(PSInput i) : SV_TARGET
{
	float3 viewVec = normalize(i.viewVec);
	float3 normal = normalize(i.norm);
	float3 color = aC.xyz * surface.x;
	float specAlpha = 0.0f;

	float3 lightVec;
	float3 halfVec;
	float nh;
	//Light0
	if (lightColor[0].w)
	{
		lightVec = normalize(i.lightVec0);
		halfVec = normalize(viewVec + lightVec);
		color += lightColor[0].xyz * surfaceColor.xyz * surface.y * clamp(dot(normal, lightVec), 0.0f, 1.0f); //diffuse Color
		nh = dot(normal, halfVec);
		nh = clamp(nh, 0.0f, 1.0f);
		nh = pow(nh, surface.w);
		nh *= surface.z;
		specAlpha += nh;
		color += lightColor[0].xyz * nh; //specular Color
	}
	//Light1
	if (lightColor[1].w)
	{
		lightVec = normalize(i.lightVec1);
		halfVec = normalize(viewVec + lightVec);
		color += lightColor[1].xyz * surfaceColor.xyz * surface.y * clamp(dot(normal, lightVec), 0.0f, 1.0f); //diffuse Color
		nh = dot(normal, halfVec);
		nh = clamp(nh, 0.0f, 1.0f);
		nh = pow(nh, surface.w);
		nh *= surface.z;
		specAlpha += nh;
		color += lightColor[1].xyz * nh; //specular Color
	}
	//Light2
	if (lightColor[2].w)
	{
		lightVec = normalize(i.lightVec2);
		halfVec = normalize(viewVec + lightVec);
		color += lightColor[2].xyz * surfaceColor.xyz * surface.y * clamp(dot(normal, lightVec), 0.0f, 1.0f); //diffuse Color
		nh = dot(normal, halfVec);
		nh = clamp(nh, 0.0f, 1.0f);
		nh = pow(nh, surface.w);
		nh *= surface.z;
		specAlpha += nh;
		color += lightColor[2].xyz * nh; //specular Color
	}
	return float4(color, surfaceColor.w + specAlpha);
}

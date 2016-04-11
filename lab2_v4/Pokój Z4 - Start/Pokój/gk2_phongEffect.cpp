#include "gk2_phongEffect.h"

using namespace std;
using namespace gk2;
using namespace DirectX;

const wstring PhongEffect::ShaderName = L"phong";

PhongEffect::PhongEffect(DeviceHelper& device, shared_ptr<ID3D11InputLayout>& layout,
						 shared_ptr<ID3D11DeviceContext> context /* = nullptr */)
	: EffectBase(context)
{
	Initialize(device, layout, ShaderName);
}

void PhongEffect::SetLightPosBuffer(const shared_ptr<ConstantBuffer<XMFLOAT4>>& lightPos)
{
	if (lightPos != nullptr)
		m_lightPosCB = lightPos;
}

void PhongEffect::SetSurfaceColorBuffer(const shared_ptr<ConstantBuffer<XMFLOAT4>>& surfaceColor)
{
	if (surfaceColor != nullptr)
		m_surfaceColorCB = surfaceColor;
}

void PhongEffect::SetVertexShaderData()
{
	ID3D11Buffer* vsb[4] = { m_worldCB->getBufferObject().get(), m_viewCB->getBufferObject().get(),
							 m_projCB->getBufferObject().get(), m_lightPosCB->getBufferObject().get() };
	m_context->VSSetConstantBuffers(0, 4, vsb);
}

void PhongEffect::SetPixelShaderData()
{
	ID3D11Buffer* psb[1] = { m_surfaceColorCB->getBufferObject().get() };
	m_context->PSSetConstantBuffers(0, 1, psb);
}

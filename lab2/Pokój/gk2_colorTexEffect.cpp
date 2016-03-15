#include "gk2_colorTexEffect.h"

using namespace std;
using namespace gk2;
using namespace DirectX;

const wstring ColorTexEffect::ShaderName = L"colorTex";

ColorTexEffect::ColorTexEffect(DeviceHelper& device, shared_ptr<ID3D11InputLayout>& layout,
						 shared_ptr<ID3D11DeviceContext> context /* = nullptr */)
	: EffectBase(context)
{
	Initialize(device, layout, ShaderName);
}

void ColorTexEffect::SetTextureMtxBuffer(const shared_ptr<gk2::CBMatrix>& textureMtx)
{
	if (textureMtx != nullptr)
		m_textureMtxCB = textureMtx;
}

void ColorTexEffect::SetSamplerState(const shared_ptr<ID3D11SamplerState>& samplerState)
{
	if (samplerState != nullptr)
		m_samplerState = samplerState;
}

void ColorTexEffect::SetTexture(const shared_ptr<ID3D11ShaderResourceView>& texture)
{
	if (texture != nullptr)
		m_texture = texture;
}

void ColorTexEffect::SetSurfaceColorBuffer(const shared_ptr<ConstantBuffer<XMFLOAT4>>& surfaceColor)
{
	if (surfaceColor != nullptr)
		m_surfaceColorCB = surfaceColor;
}

void ColorTexEffect::SetVertexShaderData()
{
	ID3D11Buffer* vsb[4] = { m_worldCB->getBufferObject().get(), m_viewCB->getBufferObject().get(),
							 m_projCB->getBufferObject().get(), m_textureMtxCB->getBufferObject().get() };
	m_context->VSSetConstantBuffers(0, 4, vsb);
}

void ColorTexEffect::SetPixelShaderData()
{
	ID3D11Buffer* psb[1] = { m_surfaceColorCB->getBufferObject().get() };
	m_context->PSSetConstantBuffers(0, 1, psb);
	ID3D11SamplerState* ss[1] = { m_samplerState.get() };
	m_context->PSSetSamplers(0, 1, ss);
	ID3D11ShaderResourceView* srv[1] = { m_texture.get() };
	m_context->PSSetShaderResources(0, 1, srv);
}
#include "gk2_multiTexEffect.h"

using namespace std;
using namespace gk2;


const wstring MultiTexEffect::ShaderName = L"multiTex";

MultiTexEffect::MultiTexEffect(DeviceHelper& device, shared_ptr<ID3D11InputLayout>& layout,
	shared_ptr<ID3D11DeviceContext> context /* = nullptr */)
	: EffectBase(context)
{
	Initialize(device, layout, ShaderName);
}

void MultiTexEffect::Set1stTextureMtxBuffer(const shared_ptr<gk2::CBMatrix>& textureMtx)
{
	if (textureMtx != nullptr)
		m_1stTextureMtxCB = textureMtx;
}

void MultiTexEffect::Set2ndTextureMtxBuffer(const shared_ptr<gk2::CBMatrix>& textureMtx)
{
	if (textureMtx != nullptr)
		m_2ndTextureMtxCB = textureMtx;
}

void MultiTexEffect::SetSamplerState(const shared_ptr<ID3D11SamplerState>& samplerState)
{
	if (samplerState != nullptr)
		m_samplerState = samplerState;
}

void MultiTexEffect::Set1stTexture(const shared_ptr<ID3D11ShaderResourceView>& texture)
{
	if (texture != nullptr)
		m_1stTexture = texture;
}

void MultiTexEffect::Set2ndTexture(const shared_ptr<ID3D11ShaderResourceView>& texture)
{
	if (texture != nullptr)
		m_2ndTexture = texture;
}


void MultiTexEffect::SetVertexShaderData()
{
	ID3D11Buffer* vsb[5] = { m_worldCB->getBufferObject().get(), m_viewCB->getBufferObject().get(),
		m_projCB->getBufferObject().get(), m_1stTextureMtxCB->getBufferObject().get(),
		m_2ndTextureMtxCB->getBufferObject().get() };
	m_context->VSSetConstantBuffers(0, 5, vsb);
}

void MultiTexEffect::SetPixelShaderData()
{
	ID3D11SamplerState* ss[1] = { m_samplerState.get() };
	m_context->PSSetSamplers(0, 1, ss);
	ID3D11ShaderResourceView* srv[2] = { m_1stTexture.get(), m_2ndTexture.get() };
	m_context->PSSetShaderResources(0, 2, srv);
}
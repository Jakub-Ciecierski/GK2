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

void MultiTexEffect::SetTextureMtxBuffer(const shared_ptr<gk2::CBMatrix>& textureMtx)
{
	if (textureMtx != nullptr)
		m_textureMtxCB = textureMtx;
}

void MultiTexEffect::SetTextureMtxBuffer2(const std::shared_ptr<CBMatrix>& textureMtx) {
	if (textureMtx != nullptr)
		m_textureMtxCB2 = textureMtx;
}

void MultiTexEffect::SetSamplerState(const shared_ptr<ID3D11SamplerState>& samplerState)
{
	if (samplerState != nullptr)
		m_samplerState = samplerState;
}

void MultiTexEffect::SetTexture(const shared_ptr<ID3D11ShaderResourceView>& texture)
{
	if (texture != nullptr)
		m_texture = texture;
}

void MultiTexEffect::SetTexture2(const std::shared_ptr<ID3D11ShaderResourceView>& texture) {
	if (texture != nullptr)
		m_texture2 = texture;
}

void MultiTexEffect::SetVertexShaderData()
{
	//TODO: Bind any additional vertex shader buffers
	ID3D11Buffer* vsb[3] = { m_worldCB->getBufferObject().get(), m_viewCB->getBufferObject().get(),
							 m_projCB->getBufferObject().get()
						   };
	m_context->VSSetConstantBuffers(0, 3, vsb);
}

void MultiTexEffect::SetPixelShaderData()
{
	ID3D11SamplerState* ss[1] = { m_samplerState.get() };
	m_context->PSSetSamplers(0, 1, ss);

	//TODO: Bind any additional textures
	ID3D11ShaderResourceView* srv[1] = { m_texture.get() };
	m_context->PSSetShaderResources(0, 1, srv);
}
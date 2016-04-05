#include "gk2_environmentMapper.h"

using namespace std;
using namespace gk2;
using namespace DirectX;

const wstring EnvironmentMapper::ShaderName = L"envMap";
const int EnvironmentMapper::TEXTURE_SIZE = 256;

EnvironmentMapper::EnvironmentMapper(DeviceHelper& device, shared_ptr<ID3D11InputLayout>& layout,
	const shared_ptr<ID3D11DeviceContext>& context, float nearPlane, float farPlane, XMFLOAT3 pos)
	: EffectBase(context)
{
	Initialize(device, layout, ShaderName);
	m_nearPlane = nearPlane;
	m_farPlane = farPlane;
	m_position = XMFLOAT4(pos.x, pos.y, pos.z, 1.0f);
	m_face = static_cast<D3D11_TEXTURECUBE_FACE>(-1);
	InitializeTextures(device);
}

void EnvironmentMapper::InitializeTextures(DeviceHelper& device)
{
	auto texDesc = device.DefaultTexture2DDesc();

	//TODO: setup texture's width, height, mipLevels and bindflags


	//m_envFaceRenderTexture = device.CreateTexture2D(texDesc);
	//m_envFaceRenderTarget = device.CreateRenderTargetView(m_envFaceRenderTexture);
	SIZE s;
	s.cx = s.cy = TEXTURE_SIZE;
	m_envFaceDepthTexture = device.CreateDepthStencilTexture(s);
	m_envFaceDepthView = device.CreateDepthStencilView(m_envFaceDepthTexture);

	//TODO: Create description for empty texture used as environment cube map
	//TODO: setup texture's width, height, mipLevels, bindflags, array size and miscFlags


	//m_envTexture = device.CreateTexture2D(texDesc);
	auto srvDesc = device.DefaultShaderResourceDesc();

	//TODO: Create description of shader resource view for cube map


	//m_envTextureView = device.CreateShaderResourceView(m_envTexture, srvDesc);
}

void EnvironmentMapper::SetupFace(const shared_ptr<ID3D11DeviceContext>& context, D3D11_TEXTURECUBE_FACE face)
{
	if (context != nullptr && context != m_context)
		m_context = context;
	if (m_context == nullptr)
		return;
	//TODO: Setup view and proj matrices


	//TODO: Replace with correct implementation
	m_viewCB->Update(m_context, XMMatrixIdentity());
	//TODO: Replace with correct implementation
	m_projCB->Update(m_context, XMMatrixIdentity());

	D3D11_VIEWPORT viewport;

	//TODO: Setup viewport


	m_context->RSSetViewports(1, &viewport);
	ID3D11RenderTargetView* targets[1] = { m_envFaceRenderTarget.get() };
	m_context->OMSetRenderTargets(1, targets, m_envFaceDepthView.get());
	float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_context->ClearRenderTargetView(m_envFaceRenderTarget.get(), clearColor);
	m_context->ClearDepthStencilView(m_envFaceDepthView.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

}

// ReSharper disable once CppMemberFunctionMayBeConst
void EnvironmentMapper::EndFace()
{
	if (m_face < 0 || m_face > 5)
		return;
	//TODO: copy face texture to the environment cube map

}

void EnvironmentMapper::SetSamplerState(const shared_ptr<ID3D11SamplerState>& samplerState)
{
	if (samplerState != nullptr)
		m_samplerState = samplerState;
}

void EnvironmentMapper::SetCameraPosBuffer(const shared_ptr<ConstantBuffer<XMFLOAT4>>& cameraPos)
{
	if (cameraPos != nullptr)
		m_cameraPosCB = cameraPos;
}

void EnvironmentMapper::SetSurfaceColorBuffer(const shared_ptr<ConstantBuffer<XMFLOAT4>>& surfaceColor)
{
	if (surfaceColor != nullptr)
		m_surfaceColorCB = surfaceColor;
}

void EnvironmentMapper::SetVertexShaderData()
{
	ID3D11Buffer* vsb[4] = { m_worldCB->getBufferObject().get(), m_viewCB->getBufferObject().get(),
							 m_projCB->getBufferObject().get(),  m_cameraPosCB->getBufferObject().get() };
	m_context->VSSetConstantBuffers(0, 4, vsb);
}

void EnvironmentMapper::SetPixelShaderData()
{
	ID3D11Buffer* psb[1] = { m_surfaceColorCB->getBufferObject().get() };
	m_context->PSSetConstantBuffers(0, 1, psb);
	ID3D11SamplerState* ss[1] = { m_samplerState.get() };
	m_context->PSSetSamplers(0, 1, ss);
	ID3D11ShaderResourceView* srv[1] = { m_envTextureView.get() };
	m_context->PSSetShaderResources(0, 1, srv);
}
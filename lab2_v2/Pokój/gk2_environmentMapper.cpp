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

	// 1.1)
	texDesc.Width = TEXTURE_SIZE;
	texDesc.Height = TEXTURE_SIZE;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
	texDesc.MipLevels = 1;
	m_envFaceRenderTexture = device.CreateTexture2D(texDesc);

	m_envFaceRenderTarget = device.CreateRenderTargetView(m_envFaceRenderTexture);
	SIZE s;
	s.cx = s.cy = TEXTURE_SIZE;
	m_envFaceDepthTexture = device.CreateDepthStencilTexture(s);
	m_envFaceDepthView = device.CreateDepthStencilView(m_envFaceDepthTexture);

	//TODO: Create description for empty texture used as environment cube map
	//TODO: setup texture's width, height, mipLevels, bindflags, array size and miscFlags

	// 1.2)
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	texDesc.ArraySize = 6;
	m_envTexture = device.CreateTexture2D(texDesc);

	// 1.3)
	auto srvDesc = device.DefaultShaderResourceDesc();
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = 1;
	srvDesc.TextureCube.MostDetailedMip = 0;
	//TODO: Create description of shader resource view for cube map
	m_envTextureView = device.CreateShaderResourceView(m_envTexture, srvDesc);
}

void EnvironmentMapper::SetupFace(const shared_ptr<ID3D11DeviceContext>& context, D3D11_TEXTURECUBE_FACE face)
{
	if (context != nullptr && context != m_context)
		m_context = context;
	if (m_context == nullptr)
		return;
	//TODO: Setup view and proj matrices
	XMFLOAT4 direction;
	XMFLOAT4 up(0,1,0,0);

	switch (face) {
	case D3D11_TEXTURECUBE_FACE_NEGATIVE_X:
		direction = XMFLOAT4(-1, 0, 0, 0);
		break;
	case D3D11_TEXTURECUBE_FACE_NEGATIVE_Y:
		direction = XMFLOAT4(0, -1, 0, 0);
		up = XMFLOAT4(0, 0, 1, 0);
		break;
	case D3D11_TEXTURECUBE_FACE_NEGATIVE_Z:
		direction = XMFLOAT4(0, 0, -1, 0);
		break;
	case D3D11_TEXTURECUBE_FACE_POSITIVE_X:
		direction = XMFLOAT4(1, 0, 0, 0);
		break;
	case D3D11_TEXTURECUBE_FACE_POSITIVE_Y:
		direction = XMFLOAT4(0, 1, 0, 0);
		up = XMFLOAT4(0, 0, -1, 0);
		break;
	case D3D11_TEXTURECUBE_FACE_POSITIVE_Z:
		direction = XMFLOAT4(0, 0, 1, 0);
		break;
	}
	XMVECTOR position_vec = XMLoadFloat4(&m_position);
	XMVECTOR direction_vec = XMLoadFloat4(&direction);
	XMVECTOR up_vec = XMLoadFloat4(&up);

	
	//TODO: Replace with correct implementation
	m_viewCB->Update(m_context, XMMatrixLookToLH(position_vec, direction_vec, up_vec));
	//TODO: Replace with correct implementation
	m_projCB->Update(m_context, XMMatrixPerspectiveFovLH(XM_PIDIV2, 1, m_nearPlane, m_farPlane));

	//TODO: Setup viewport
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;
	viewport.Height = TEXTURE_SIZE;
	viewport.Width = TEXTURE_SIZE;

	
	m_context->RSSetViewports(1, &viewport);
	ID3D11RenderTargetView* targets[1] = { m_envFaceRenderTarget.get() };
	m_context->OMSetRenderTargets(1, targets, m_envFaceDepthView.get());
	float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_context->ClearRenderTargetView(m_envFaceRenderTarget.get(), clearColor);
	m_context->ClearDepthStencilView(m_envFaceDepthView.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_face = face;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void EnvironmentMapper::EndFace()
{
	if (m_face < 0 || m_face > 5)
		return;
	//TODO: copy face texture to the environment cube map
	
	m_context->CopySubresourceRegion(m_envTexture.get(), m_face, 
		0,0,0,
		m_envFaceRenderTexture.get(), 0, nullptr);
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
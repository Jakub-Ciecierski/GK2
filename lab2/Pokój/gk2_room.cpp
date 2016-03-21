#include "gk2_room.h"
#include "gk2_window.h"
#include "gk2_textureGenerator.h"
#include "gk2_utils.h"
#include <array>

using namespace std;
using namespace gk2;
using namespace DirectX;

const float Room::TABLE_H = 1.0f;
const float Room::TABLE_TOP_H = 0.1f;
const float Room::TABLE_R = 1.5f;
const XMFLOAT4 Room::TABLE_POS = XMFLOAT4(0.5f, -0.96f, 0.5f, 1.0f);
const XMFLOAT4 Room::LIGHT_POS[2] = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f) };
const unsigned int Room::BS_MASK = 0xffffffff;

Room::Room(HINSTANCE hInstance)
	: ApplicationBase(hInstance), m_camera(0.01f, 100.0f)
{

}

Room::~Room()
{

}

void* Room::operator new(size_t size)
{
	return Utils::New16Aligned(size);
}

void Room::operator delete(void* ptr)
{
	Utils::Delete16Aligned(ptr);
}

void Room::InitializeConstantBuffers()
{
	m_projCB.reset(new CBMatrix(m_device));
	m_viewCB.reset(new CBMatrix(m_device));
	m_worldCB.reset(new CBMatrix(m_device));
	m_lightPosCB.reset(new ConstantBuffer<XMFLOAT4, 2>(m_device));
	m_textureCB.reset(new CBMatrix(m_device));
	m_posterTexCB.reset(new CBMatrix(m_device));
	m_surfaceColorCB.reset(new ConstantBuffer<XMFLOAT4>(m_device));
	m_cameraPosCB.reset(new ConstantBuffer<XMFLOAT4>(m_device));
}

void Room::InitializeCamera()
{
	auto s = getMainWindow()->getClientSize();
	auto ar = static_cast<float>(s.cx) / s.cy;
	m_projMtx = XMMatrixPerspectiveFovLH(XM_PIDIV4, ar, 0.01f, 100.0f);
	m_projCB->Update(m_context, m_projMtx);
	m_camera.Zoom(5);
	UpdateCamera();
}

void Room::InitializeTextures()
{
	constexpr int WOOD_TEX_WIDTH = 64;
	constexpr int WOOD_TEX_HEIGHT = 512;
	constexpr int WOOD_TEX_BPP = 4;
	constexpr int WOOD_TEX_STRIDE = WOOD_TEX_WIDTH * WOOD_TEX_BPP;
	constexpr int WOOD_TEX_SIZE = WOOD_TEX_STRIDE * WOOD_TEX_HEIGHT;
	m_wallTexture = m_device.CreateShaderResourceView(L"resources/textures/brick_wall.jpg");
	m_posterTexture = m_device.CreateShaderResourceView(L"resources/textures/lautrec_divan.jpg");
	auto sd = m_device.DefaultSamplerDesc();
	sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	m_samplerWrap = m_device.CreateSamplerState(sd);
	
	//TODO: Initialize back wall sampler state DONE
	sd.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sd.MipLODBias = 1.0f;
	m_samplerBorder = m_device.CreateSamplerState(sd);
	m_perlinTexture = m_device.CreateShaderResourceView(L"resources/textures/perlin.jpg");

	auto texDesc = m_device.DefaultTexture2DDesc();
	texDesc.Width = WOOD_TEX_WIDTH;
	texDesc.Height = WOOD_TEX_HEIGHT;
	texDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	auto woodTexture = m_device.CreateTexture2D(texDesc);
	m_woodTexture = m_device.CreateShaderResourceView(woodTexture);
	array<BYTE, WOOD_TEX_SIZE> data;
	auto d = data.data();
	TextureGenerator txGen(6, 0.35f);
	for (auto i = 0; i < WOOD_TEX_HEIGHT; ++i)
	{
		auto x = i / static_cast<float>(WOOD_TEX_HEIGHT);
		for (auto j = 0; j < WOOD_TEX_WIDTH; ++j)
		{
			auto y = j / static_cast<float>(WOOD_TEX_WIDTH);
			auto c = txGen.Wood(x, y);
			auto ic = static_cast<BYTE>(c * 239);
			*(d++) = ic;
			ic = static_cast<BYTE>(c * 200);
			*(d++) = ic;
			ic = static_cast<BYTE>(c * 139);
			*(d++) = ic;
			*(d++) = 255;
		}
	}
	m_context->UpdateSubresource(woodTexture.get(), 0, nullptr, data.data(), WOOD_TEX_STRIDE, WOOD_TEX_SIZE);
	m_context->GenerateMips(m_woodTexture.get());
}

void Room::CreateScene()
{
	MeshLoader loader(m_device);
	m_walls[0] = loader.GetQuad(4.0f);
	for (auto i = 1; i < 6; ++i)
		m_walls[i] = m_walls[0];
	auto wall = XMMatrixTranslation(0.0f, 0.0f, 2.0f);
	float a = 0;
	for (int i = 0; i < 4; ++i, a += XM_PIDIV2)
		m_walls[i].setWorldMatrix(wall * XMMatrixRotationY(a));
	m_walls[4].setWorldMatrix(wall * XMMatrixRotationX(XM_PIDIV2));
	m_walls[5].setWorldMatrix(wall * XMMatrixRotationX(-XM_PIDIV2));
	m_teapot = loader.LoadMesh(L"resources/meshes/teapot.mesh");
	auto teapotMtx = XMMatrixTranslation(0.0f, -2.3f, 0.f) * XMMatrixScaling(0.1f, 0.1f, 0.1f) *
						 XMMatrixRotationY(-XM_PIDIV2) * XMMatrixTranslation(-1.3f, -0.74f, -0.6f);
	m_teapot.setWorldMatrix(teapotMtx);
	m_debugSphere = loader.GetSphere(8, 16, 0.3f);
	m_debugSphere.setWorldMatrix(XMMatrixRotationY(-XM_PIDIV2) * XMMatrixTranslation(-1.3f, -0.74f, -0.6f));
	m_box = loader.GetBox();
	m_box.setWorldMatrix(XMMatrixTranslation(-1.4f, -1.46f, -0.6f));
	m_lamp = loader.LoadMesh(L"resources/meshes/lamp.mesh");
	UpdateLamp(0.0f);
	m_chairSeat = loader.LoadMesh(L"resources/meshes/chair_seat.mesh");
	m_chairBack = loader.LoadMesh(L"resources/meshes/chair_back.mesh");
	auto chair = XMMatrixRotationY(XM_PI + XM_PI/9 /*20 deg*/) * XMMatrixTranslation(-0.1f, -1.06f, -1.3f);
	m_chairSeat.setWorldMatrix(chair);
	m_chairBack.setWorldMatrix(chair);
	m_monitor = loader.LoadMesh(L"resources/meshes/monitor.mesh");
	m_screen = loader.LoadMesh(L"resources/meshes/screen.mesh");
	auto monitor = XMMatrixRotationY(XM_PIDIV4) *
					   XMMatrixTranslation(TABLE_POS.x, TABLE_POS.y + 0.42f, TABLE_POS.z);
	m_monitor.setWorldMatrix(monitor);
	m_screen.setWorldMatrix(monitor);
	m_tableLegs[0] = loader.GetCylinder(4, 9, 0.1f, TABLE_H - TABLE_TOP_H);
	for (auto i = 1; i < 4; ++i)
		m_tableLegs[i] = m_tableLegs[0];
	a = XM_PIDIV4;
	for (auto i = 0; i < 4; ++i, a += XM_PIDIV2)
		m_tableLegs[i].setWorldMatrix(XMMatrixTranslation(0.0f, 0.0f, TABLE_R - 0.35f) * XMMatrixRotationY(a) *
			XMMatrixTranslation(TABLE_POS.x, TABLE_POS.y - (TABLE_H + TABLE_TOP_H) / 2, TABLE_POS.z));
	m_tableSide = loader.GetCylinder(1, 16, TABLE_R, TABLE_TOP_H);
	m_tableSide.setWorldMatrix(XMMatrixRotationY(XM_PIDIV4/4) *
							   XMMatrixTranslation(TABLE_POS.x, TABLE_POS.y - TABLE_TOP_H / 2, TABLE_POS.z));
	m_tableTop = loader.GetDisc(16, TABLE_R);
	m_tableTop.setWorldMatrix(XMMatrixRotationY(XM_PIDIV4/4) *
							  XMMatrixTranslation(TABLE_POS.x, TABLE_POS.y, TABLE_POS.z));
	m_lightPosCB->Update(m_context, LIGHT_POS);
	m_textureCB->Update(m_context, XMMatrixScaling(0.25f, 0.25f, 1.0f) * XMMatrixTranslation(0.5f, 0.5f, 0.0f));
	
	//TODO: replace following with correct value

	float width = 4.0f;
	XMMATRIX posterMatrix = XMMatrixTranslation(0.2*width, 0, 0) * XMMatrixRotationZ(-XM_PI / 18)
		* XMMatrixScaling(4 / width, -3 / width, 1) * XMMatrixTranslation(0.5, 0.5, 0);
	m_posterTexCB->Update(m_context, posterMatrix);
}

void Room::InitializeRenderStates()
{
	auto rsDesc = m_device.DefaultRasterizerDesc();
	rsDesc.CullMode = D3D11_CULL_FRONT;
	m_rsCullFront = m_device.CreateRasterizerState(rsDesc);

	auto bsDesc = m_device.DefaultBlendDesc();
	bsDesc.RenderTarget[0].BlendEnable = true;
	bsDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bsDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bsDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	m_bsAlpha = m_device.CreateBlendState(bsDesc);

	auto dssDesc = m_device.DefaultDepthStencilDesc();
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	m_dssNoWrite = m_device.CreateDepthStencilState(dssDesc);
}

bool Room::LoadContent()
{
	InitializeConstantBuffers();
	InitializeCamera();
	InitializeTextures();
	InitializeRenderStates();
	CreateScene();
	m_phongEffect.reset(new PhongEffect(m_device, m_layout));
	m_phongEffect->SetProjMtxBuffer(m_projCB);
	m_phongEffect->SetViewMtxBuffer(m_viewCB);
	m_phongEffect->SetWorldMtxBuffer(m_worldCB);
	m_phongEffect->SetLightPosBuffer(m_lightPosCB);
	m_phongEffect->SetSurfaceColorBuffer(m_surfaceColorCB);

	m_textureEffect.reset(new TextureEffect(m_device, m_layout));
	m_textureEffect->SetProjMtxBuffer(m_projCB);
	m_textureEffect->SetViewMtxBuffer(m_viewCB);
	m_textureEffect->SetWorldMtxBuffer(m_worldCB);
	m_textureEffect->SetTextureMtxBuffer(m_textureCB);
	m_textureEffect->SetSamplerState(m_samplerWrap);
	m_textureEffect->SetTexture(m_wallTexture);

	m_colorTexEffect.reset(new ColorTexEffect(m_device, m_layout));
	m_colorTexEffect->SetProjMtxBuffer(m_projCB);
	m_colorTexEffect->SetViewMtxBuffer(m_viewCB);
	m_colorTexEffect->SetWorldMtxBuffer(m_worldCB);
	m_colorTexEffect->SetTextureMtxBuffer(m_textureCB);
	m_colorTexEffect->SetSamplerState(m_samplerWrap);
	m_colorTexEffect->SetTexture(m_perlinTexture);
	m_colorTexEffect->SetSurfaceColorBuffer(m_surfaceColorCB);

	//TODO: Initialize multi-texture effect
	m_multiTexEffect.reset(new MultiTexEffect(m_device, m_layout));
	m_multiTexEffect->SetProjMtxBuffer(m_projCB);
	m_multiTexEffect->SetViewMtxBuffer(m_viewCB);
	m_multiTexEffect->SetWorldMtxBuffer(m_worldCB);
	m_multiTexEffect->SetSamplerState(m_samplerBorder);
	// Wall
	m_multiTexEffect->SetTextureMtxBuffer(m_textureCB);
	m_multiTexEffect->SetTexture(m_wallTexture);
	// Poster
	m_multiTexEffect->SetTextureMtxBuffer2(m_posterTexCB);
	m_multiTexEffect->SetTexture2(m_posterTexture);


	m_environmentMapper.reset(new EnvironmentMapper(m_device, m_layout, m_context, 0.4f, 8.0f,
													XMFLOAT3(-1.3f, -0.74f, -0.6f)));
	m_environmentMapper->SetProjMtxBuffer(m_projCB);
	m_environmentMapper->SetViewMtxBuffer(m_viewCB);
	m_environmentMapper->SetWorldMtxBuffer(m_worldCB);
	m_environmentMapper->SetSamplerState(m_samplerWrap);
	m_environmentMapper->SetCameraPosBuffer(m_cameraPosCB);
	m_environmentMapper->SetSurfaceColorBuffer(m_surfaceColorCB);

	m_particles.reset(new ParticleSystem(m_device,  XMFLOAT3(-1.3f, -0.6f, -0.14f)));
	m_particles->SetViewMtxBuffer(m_viewCB);
	m_particles->SetProjMtxBuffer(m_projCB);
	m_particles->SetSamplerState(m_samplerWrap);
	return true;
}

void Room::UnloadContent()
{

}

void Room::UpdateCamera() const
{
	XMMATRIX view;
	m_camera.GetViewMatrix(view);
	m_viewCB->Update(m_context, view);
	m_cameraPosCB->Update(m_context, m_camera.GetPosition());
}

void Room::UpdateLamp(float dt)
{
	static auto time = 0.0f;
	time += dt;
	auto swing = 0.3f * XMScalarSin(XM_2PI*time/8);
	auto rot = XM_2PI*time/20;
	auto lamp = XMMatrixTranslation(0.0f, -0.4f, 0.0f) * XMMatrixRotationX(swing) * XMMatrixRotationY(rot) *
				XMMatrixTranslation(0.0f, 2.0f, 0.0f);
	m_lamp.setWorldMatrix(lamp);
}

void Room::Update(float dt)
{
	UpdateLamp(dt);
	static MouseState prevState;
	MouseState currentState;
	if (m_mouse->GetState(currentState))
	{
		bool change = true;
		if (prevState.isButtonDown(0))
		{
			auto d = currentState.getMousePositionChange();
			m_camera.Rotate(d.y/300.f, d.x/300.f);
		}
		else if (prevState.isButtonDown(1))
		{
			auto d = currentState.getMousePositionChange();
			m_camera.Zoom(d.y/10.0f);
		}
		else
			change = false;
		prevState = currentState;
		if (change)
			UpdateCamera();
	}
	m_particles->Update(m_context, dt, m_camera.GetPosition());
}

void Room::DrawWalls() const
{
	//Draw floor
	m_textureCB->Update(m_context, XMMatrixScaling(0.25f, 4.0f, 1.0f) * XMMatrixTranslation(0.5f, 0.5f, 0.0f));
	m_textureEffect->SetTexture(m_woodTexture);
	m_textureEffect->Begin(m_context);
	m_worldCB->Update(m_context, m_walls[4].getWorldMatrix());
	m_walls[4].Render(m_context);
	m_textureEffect->End();

	//Draw ceiling
	m_surfaceColorCB->Update(m_context, XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));
	m_textureCB->Update(m_context, XMMatrixScaling(0.25f, 0.25f, 0.25f) * XMMatrixTranslation(0.5f, 0.5f, 0.0f));
	m_colorTexEffect->Begin(m_context);
	m_worldCB->Update(m_context, m_walls[5].getWorldMatrix());
	m_walls[5].Render(m_context);
	m_colorTexEffect->End();
	m_surfaceColorCB->Update(m_context, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	//draw back wall
	//TODO: Replace with multi-texture effect
	m_multiTexEffect->Begin(m_context);
	m_worldCB->Update(m_context, m_walls[0].getWorldMatrix());
	m_walls[0].Render(m_context);
	m_multiTexEffect->End();

	//draw remaining walls
	m_textureEffect->SetTexture(m_wallTexture);
	m_textureEffect->Begin(m_context);
	for (auto i = 1; i < 4; ++i)
	{
		m_worldCB->Update(m_context, m_walls[i].getWorldMatrix());
		m_walls[i].Render(m_context);
	}
	m_textureEffect->End();

}

void Room::DrawTeapot() const
{
	//TODO: Replace with environment map effect
	m_phongEffect->Begin(m_context);
	m_surfaceColorCB->Update(m_context, XMFLOAT4(0.8f, 0.7f, 0.65f, 1.0f));

	m_worldCB->Update(m_context, m_teapot.getWorldMatrix());
	m_teapot.Render(m_context);
	/*m_worldCB->Update(m_context, m_debugSphere.getWorldMatrix());
	m_debugSphere.Render(m_context);*/

	m_surfaceColorCB->Update(m_context, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	m_phongEffect->End();
}

void Room::DrawTableElement(Mesh& element) const
{
	m_worldCB->Update(m_context, element.getWorldMatrix());
	m_context->RSSetState(m_rsCullFront.get());
	element.Render(m_context);
	m_context->RSSetState(nullptr);
	element.Render(m_context);
}

void Room::DrawTableLegs(XMVECTOR camVec)
{
	XMFLOAT4 v(1.0f, 0.0f, 0.0f, 0.0f);
	auto plane1 = XMLoadFloat4(&v);
	v = XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f);
	auto plane2 = XMLoadFloat4(&v);
	auto left = XMVector3Dot(camVec, plane1).m128_f32[0] > 0;
	auto back = XMVector3Dot(camVec, plane2).m128_f32[0] > 0;
	if (left)
	{
		if (back)
		{
			DrawTableElement(m_tableLegs[2]);
			DrawTableElement(m_tableLegs[3]);
			DrawTableElement(m_tableLegs[1]);
			DrawTableElement(m_tableLegs[0]);
		}
		else
		{
			DrawTableElement(m_tableLegs[3]);
			DrawTableElement(m_tableLegs[2]);
			DrawTableElement(m_tableLegs[0]);
			DrawTableElement(m_tableLegs[1]);
		}
	}
	else
	{

		if (back)
		{
			DrawTableElement(m_tableLegs[1]);
			DrawTableElement(m_tableLegs[0]);
			DrawTableElement(m_tableLegs[2]);
			DrawTableElement(m_tableLegs[3]);
		}
		else
		{
			DrawTableElement(m_tableLegs[0]);
			DrawTableElement(m_tableLegs[1]);
			DrawTableElement(m_tableLegs[3]);
			DrawTableElement(m_tableLegs[2]);
		}
	}
}

void Room::DrawTransparentObjects()
{
	m_context->OMSetBlendState(m_bsAlpha.get(), nullptr, BS_MASK);
	m_context->OMSetDepthStencilState(m_dssNoWrite.get(), 0);

	m_surfaceColorCB->Update(m_context, XMFLOAT4(0.1f, 0.1f, 0.1f, 0.9f));
	auto v = m_camera.GetPosition();
	auto camVec = XMVector3Normalize(XMLoadFloat4(&v) - XMLoadFloat4(&TABLE_POS));
	v = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);
	auto plane = XMLoadFloat4(&v);
	if (XMVector3Dot(camVec, plane).m128_f32[0] > 0)
	{
		m_phongEffect->Begin(m_context);
		m_context->RSSetState(m_rsCullFront.get());
		m_worldCB->Update(m_context, m_tableSide.getWorldMatrix());
		m_tableSide.Render(m_context);
		m_context->RSSetState(nullptr);
		DrawTableLegs(camVec);
		m_worldCB->Update(m_context, m_tableSide.getWorldMatrix());
		m_tableSide.Render(m_context);
		DrawTableElement(m_tableTop);
		m_phongEffect->End();
		m_particles->Render(m_context);
	}
	else
	{
		m_particles->Render(m_context);
		m_phongEffect->Begin(m_context);
		DrawTableElement(m_tableTop);
		m_context->RSSetState(m_rsCullFront.get());
		m_worldCB->Update(m_context, m_tableSide.getWorldMatrix());
		m_tableSide.Render(m_context);
		m_context->RSSetState(nullptr);
		DrawTableLegs(camVec);
		m_worldCB->Update(m_context, m_tableSide.getWorldMatrix());
		m_tableSide.Render(m_context);
		m_phongEffect->End();
	}
	m_surfaceColorCB->Update(m_context, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	m_context->OMSetDepthStencilState(nullptr, 0);
	m_context->OMSetBlendState(nullptr, nullptr, BS_MASK);
}

void Room::DrawScene()
{

	DrawWalls();
	DrawTeapot();
	m_phongEffect->Begin(m_context);
	//Draw shelf
	m_worldCB->Update(m_context, m_box.getWorldMatrix());
	m_box.Render(m_context);
	//Draw lamp
	m_worldCB->Update(m_context, m_lamp.getWorldMatrix());
	m_lamp.Render(m_context);
	//Draw chair seat
	m_worldCB->Update(m_context, m_chairSeat.getWorldMatrix());
	m_chairSeat.Render(m_context);
	//Draw chairframe
	m_worldCB->Update(m_context, m_chairBack.getWorldMatrix());
	m_chairBack.Render(m_context);
	//Draw monitor
	m_worldCB->Update(m_context, m_monitor.getWorldMatrix());
	m_monitor.Render(m_context);
	//Draw screen
	m_worldCB->Update(m_context, m_screen.getWorldMatrix());
	m_screen.Render(m_context);
	m_phongEffect->End();
	DrawTransparentObjects();
}

void Room::Render()
{
	if (m_context == nullptr)
		return;

	//TODO: Render scene to each environment cube map face

	ResetRenderTarget();
	m_projCB->Update(m_context, m_projMtx);
	UpdateCamera();
	//Clear buffers
	float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_context->ClearRenderTargetView(m_backBuffer.get(), clearColor);
	m_context->ClearDepthStencilView(m_depthStencilView.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	DrawScene();
	m_swapChain->Present(0, 0);
}

#include "gk2_butterfly.h"
#include "gk2_utils.h"
#include "gk2_vertices.h"
#include "gk2_window.h"

using namespace std;
using namespace gk2;

#define RESOURCES_PATH L"resources/"
const wstring Butterfly::ShaderFile = RESOURCES_PATH L"shaders/Butterfly.hlsl";

const float Butterfly::DODECAHEDRON_R = sqrtf(0.375f + 0.125f * sqrtf(5.0f));
const float Butterfly::DODECAHEDRON_H = 1.0f + 2.0f * Butterfly::DODECAHEDRON_R;
const float Butterfly::DODECAHEDRON_A = XMScalarACos(-0.2f * sqrtf(5.0f));

const float Butterfly::MOEBIUS_R = 1.0f;
const float Butterfly::MOEBIUS_W = 0.1f;
const int Butterfly::MOEBIUS_N = 128;

const float Butterfly::LAP_TIME = 10.0f;
const float Butterfly::FLAP_TIME = 2.0f;
const float Butterfly::WING_W = 0.15f;
const float Butterfly::WING_H = 0.1f;
const float Butterfly::WING_MAX_A = 8.0f * XM_PIDIV2 / 9.0f; //80 degrees

const unsigned int Butterfly::VB_STRIDE = sizeof(VertexPosNormal);
const unsigned int Butterfly::VB_OFFSET = 0;
const unsigned int Butterfly::BS_MASK = 0xffffffff;

const XMFLOAT4 Butterfly::COLORS[] = 
	{ 
		XMFLOAT4(253.0f/255.0f, 198.0f/255.0f, 137.0f/255.0f, 100.0f/255.0f),
		XMFLOAT4(255.0f/255.0f, 247.0f/255.0f, 153.0f/255.0f, 100.0f/255.0f),
		XMFLOAT4(196.0f/255.0f, 223.0f/255.0f, 155.0f/255.0f, 100.0f/255.0f),
		XMFLOAT4(162.0f/255.0f, 211.0f/255.0f, 156.0f/255.0f, 100.0f/255.0f),
		XMFLOAT4(130.0f/255.0f, 202.0f/255.0f, 156.0f/255.0f, 100.0f/255.0f),
		XMFLOAT4(122.0f/255.0f, 204.0f/255.0f, 200.0f/255.0f, 100.0f/255.0f),
		XMFLOAT4(109.0f/255.0f, 207.0f/255.0f, 246.0f/255.0f, 100.0f/255.0f),
		XMFLOAT4(125.0f/255.0f, 167.0f/255.0f, 216.0f/255.0f, 100.0f/255.0f),
		XMFLOAT4(131.0f/255.0f, 147.0f/255.0f, 202.0f/255.0f, 100.0f/255.0f),
		XMFLOAT4(135.0f/255.0f, 129.0f/255.0f, 189.0f/255.0f, 100.0f/255.0f),
		XMFLOAT4(161.0f/255.0f, 134.0f/255.0f, 190.0f/255.0f, 100.0f/255.0f),
		XMFLOAT4(244.0f/255.0f, 154.0f/255.0f, 193.0f/255.0f, 100.0f/255.0f)
	};

void* Butterfly::operator new(size_t size)
{
	return Utils::New16Aligned(size);
}

void Butterfly::operator delete(void* ptr)
{
	Utils::Delete16Aligned(ptr);
}

Butterfly::Butterfly(HINSTANCE hInstance)
	: ApplicationBase(hInstance), m_camera(0.01f, 100.0f)
{

}

Butterfly::~Butterfly()
{

}

void Butterfly::InitializeShaders()
{
	shared_ptr<ID3DBlob> vsByteCode = m_device.CompileD3DShader(ShaderFile, "VS_Main", "vs_4_0");
	shared_ptr<ID3DBlob> psByteCode = m_device.CompileD3DShader(ShaderFile, "PS_Main", "ps_4_0");
	m_vertexShader = m_device.CreateVertexShader(vsByteCode);
	m_pixelShader = m_device.CreatePixelShader(psByteCode);
	m_inputLayout = m_device.CreateInputLayout<VertexPosNormal>(vsByteCode);
}

void Butterfly::InitializeConstantBuffers()
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.ByteWidth = sizeof(XMMATRIX);
	desc.Usage = D3D11_USAGE_DEFAULT;
	m_cbWorld = m_device.CreateBuffer(desc);
	m_cbView = m_device.CreateBuffer(desc);
	m_cbProj = m_device.CreateBuffer(desc);
	desc.ByteWidth = sizeof(XMFLOAT4) * 3;
	m_cbLightPos = m_device.CreateBuffer(desc);
	desc.ByteWidth = sizeof(XMFLOAT4) * 5;
	m_cbLightColors = m_device.CreateBuffer(desc);
	desc.ByteWidth = sizeof(XMFLOAT4);
	m_cbSurfaceColor = m_device.CreateBuffer(desc);
}

void Butterfly::InitializeRenderStates()
//Setup render states used in various stages of the scene rendering
{
	D3D11_DEPTH_STENCIL_DESC dssDesc = m_device.DefaultDepthStencilDesc();
	//Setup depth stencil state for writing
	m_dssWrite = m_device.CreateDepthStencilState(dssDesc);
	//Setup depth stencil state for testing
	m_dssTest = m_device.CreateDepthStencilState(dssDesc);

	D3D11_RASTERIZER_DESC rsDesc = m_device.DefaultRasterizerDesc();
	//Set rasterizer state front face to ccw
	m_rsCounterClockwise = m_device.CreateRasterizerState(rsDesc);

	D3D11_BLEND_DESC bsDesc = m_device.DefaultBlendDesc();
	//Setup alpha blending
	m_bsAlpha = m_device.CreateBlendState(bsDesc);
}

void Butterfly::InitializeCamera()
{
	SIZE s = getMainWindow()->getClientSize();
	float ar = static_cast<float>(s.cx) / s.cy;
	m_projMtx = XMMatrixPerspectiveFovLH(XM_PIDIV4, ar, 0.01f, 100.0f);
	m_context->UpdateSubresource(m_cbProj.get(), 0, 0, &m_projMtx, 0, 0);
	m_camera.Zoom(5);
	UpdateCamera();
}

void Butterfly::InitializeBox()
{
	VertexPosNormal vertices[] =
	{
		//Front face
		{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.5f, 0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-0.5f, 0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f) },

		//Left face
		{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-0.5f, 0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-0.5f, 0.5f, 0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f) },

		//Bottom face
		{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f) },

		//Back face
		{ XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(-0.5f, 0.5f, 0.5f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT3(0.0f, 0.0f, -1.0f) },

		//Right face
		{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(0.5f, 0.5f, -0.5f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },

		//Top face
		{ XMFLOAT3(-0.5f, 0.5f, -0.5f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(0.5f, 0.5f, -0.5f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(-0.5f, 0.5f, 0.5f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
	};
	m_vbBox = m_device.CreateVertexBuffer(vertices, 24);
	unsigned short indices[] =
	{
		0, 1, 2, 0, 2, 3,		//Front face
		4, 5, 6, 4, 6, 7,		//Left face
		8, 9, 10, 8, 10, 11,	//Botton face
		12, 13, 14, 12, 14, 15,	//Back face
		16, 17, 18, 16, 18, 19,	//Right face
		20, 21, 22, 20, 22, 23	//Top face
	};
	m_ibBox = m_device.CreateIndexBuffer(indices, 36);
}

void Butterfly::InitializePentagon()
{
	VertexPosNormal vertices[5];
	float a=0, da = XM_2PI / 5.0f;
	for (int i = 0; i < 5; ++i, a -= da)
	{
		float sina, cosa;
		XMScalarSinCos(&sina, &cosa, a);
		vertices[i].Pos = XMFLOAT3(cosa, sina, 0.0f);
		vertices[i].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	}
	m_vbPentagon = m_device.CreateVertexBuffer(vertices, 5);
	unsigned short indices[] = { 0, 1, 2, 0, 2, 3, 0, 3, 4 };
	m_ibPentagon = m_device.CreateIndexBuffer(indices, 9);
}

void Butterfly::InitializeDodecahedron()
//Compute dodecahedronMtx and mirrorMtx
{
	//TODO: write code here

	VertexPosNormal vertices[5];
	float a=0, da = XM_2PI / 5.0f;
	for (int i = 0; i < 5; ++i, a -= da)
	{
		float sina, cosa;
		XMScalarSinCos(&sina, &cosa, a);
		vertices[i].Pos = XMFLOAT3(cosa, sina, 0.0f);
		vertices[i].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	}
	m_vbPentagon = m_device.CreateVertexBuffer(vertices, 5);
	unsigned short indices[] = { 0, 1, 2, 0, 2, 3, 0, 3, 4 };
	m_ibPentagon = m_device.CreateIndexBuffer(indices, 9);

	int currentFaceIndex = 0;

	// Bottom
	m_dodecahedronMtx[currentFaceIndex] = XMMatrixRotationX(XM_PI/2) * XMMatrixTranslation(0, -(DODECAHEDRON_H/2.0f), 0);
	currentFaceIndex++;
	// Bottom Base - First Left Face
	m_dodecahedronMtx[currentFaceIndex] = XMMatrixRotationX(-(XM_PI/2)) * XMMatrixTranslation(DODECAHEDRON_R, 0, 0)
		* XMMatrixRotationZ(DODECAHEDRON_A) * XMMatrixTranslation(-DODECAHEDRON_R, -(DODECAHEDRON_H/2), 0);
	currentFaceIndex++;

	// Bottom Base - Remaining 4 faces
	const int BASE_COUNT = 4;
	for(int i = 0; i < BASE_COUNT; i++){
		m_dodecahedronMtx[currentFaceIndex] = m_dodecahedronMtx[currentFaceIndex - 1] * XMMatrixRotationY(2*XM_PI/5);
		currentFaceIndex++;
	}
	
	// Top Base - Including the Top and the Top Base
	const int BASE_COUNT_TOP = 6;
	for(int i = 0; i < BASE_COUNT_TOP; i++){
		m_dodecahedronMtx[currentFaceIndex] = m_dodecahedronMtx[currentFaceIndex - 6] * XMMatrixRotationZ(XM_PI);
		currentFaceIndex++;
	}

}

XMFLOAT3 Butterfly::MoebiusStripPos(float t, float s)
//Compute the position of point on the Moebius strip for parameters t and s
{
	return XMFLOAT3(0.0f, 0.0f, 0.0f); //TODO: replace with correct version
}

XMVECTOR Butterfly::MoebiusStripDt(float t, float s)
//Compute the t-derivative of point on the Moebius strip for parameters t and s
{
	XMFLOAT3 dt(1.0f, 0.0f, 0.0f); //TODO: replace with correct version
	return XMLoadFloat3(&dt);
}

XMVECTOR Butterfly::MoebiusStripDs(float t, float s)
// Return the s-derivative of point on the Moebius strip for parameters t and s
{
	XMFLOAT3 dt(1.0f, 0.0f, 0.0f); //TODO: replace with correct version
	return XMLoadFloat3(&dt);
}

void Butterfly::InitializeMoebiusStrip()
//Create vertex and index buffers for the Moebius strip
{
	//TODO: write code here
}

void Butterfly::InitializeButterfly()
//Create vertex and index buffers for the butterfly wing
{
	//TODO: write code here
}

void Butterfly::InitializeBilboards()
//Initialize bilboard resources (vertex, pixel shaders, input layout, vertex, index buffers etc.)
{

}

void Butterfly::SetShaders()
{
	m_context->IASetInputLayout(m_inputLayout.get());
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->VSSetShader(m_vertexShader.get(), 0, 0);
	m_context->PSSetShader(m_pixelShader.get(), 0, 0);
}

void Butterfly::SetConstantBuffers()
{
	ID3D11Buffer* vsb[] = { m_cbWorld.get(),  m_cbView.get(),  m_cbProj.get(), m_cbLightPos.get() };
	m_context->VSSetConstantBuffers(0, 4, vsb);
	ID3D11Buffer* psb[] = { m_cbLightColors.get(), m_cbSurfaceColor.get() };
	m_context->PSSetConstantBuffers(0, 2, psb);
}

bool Butterfly::LoadContent()
{
	InitializeShaders();
	InitializeConstantBuffers();
	InitializeRenderStates();
	InitializeCamera();
	InitializeBox();
	InitializePentagon();
	InitializeDodecahedron();
	InitializeMoebiusStrip();
	InitializeButterfly();
	InitializeBilboards();

	SetShaders();
	SetConstantBuffers();

	return true;
}

void Butterfly::UnloadContent()
{
	m_vertexShader.reset();
	m_pixelShader.reset();
	m_inputLayout.reset();

	m_dssWrite.reset();
	m_dssTest.reset();
	m_rsCounterClockwise.reset();
	m_bsAlpha.reset();

	m_vbBox.reset();
	m_ibBox.reset();
	m_vbPentagon.reset();
	m_ibPentagon.reset();
	m_vbMoebius.reset();
	m_ibMoebius.reset();

	m_cbWorld.reset();
	m_cbView.reset();
	m_cbProj.reset();
	m_cbLightPos.reset();
	m_cbLightColors.reset();
	m_cbSurfaceColor.reset();
}

void Butterfly::UpdateCamera()
{
	XMMATRIX viewMtx;
	m_camera.GetViewMatrix(viewMtx);
	m_context->UpdateSubresource(m_cbView.get(), 0, 0, &viewMtx, 0, 0);
}

void Butterfly::UpdateButterfly(float dtime)
//Compute the matrices for butterfly wings. Position on the strip is determined based on time
{
	//Time passed since the current lap started
	static float lap = 0.0f;
	lap += dtime;
	while (lap > LAP_TIME)
		lap -= LAP_TIME;
	//Value of the Moebius strip t parameter
	float t = 2 * lap/LAP_TIME;
	//Angle between wing current and vertical position
	float a = t * WING_MAX_A;
	t *= XM_2PI;
	if (a > WING_MAX_A)
		a = 2 * WING_MAX_A - a;

	//TODO: write the rest of code here
}

void Butterfly::SetLight0()
//Setup one positional light at the camera
{
	XMFLOAT4 positions[3];
	ZeroMemory(positions, sizeof(XMFLOAT4) * 3);
	positions[0] = m_camera.GetPosition();
	m_context->UpdateSubresource(m_cbLightPos.get(), 0, 0, positions, 0, 0);

	XMFLOAT4 colors[5];
	ZeroMemory(colors, sizeof(XMFLOAT4) * 5);
	colors[0] = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f); //ambient color
	colors[1] = XMFLOAT4(1.0f, 0.8f, 1.0f, 100.0f); //surface [ka, kd, ks, m]
	colors[2] = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); //light0 color
	m_context->UpdateSubresource(m_cbLightColors.get(), 0, 0, colors, 0, 0);
}

void Butterfly::SetLight1()
//Setup one white positional light at the camera
//Setup two additional positional lights, green and blue.
{
	XMFLOAT4 positions[3];
	ZeroMemory(positions, sizeof(XMFLOAT4) * 3);
	positions[0] = m_camera.GetPosition(); //white light position
	//TODO: write the rest of code here
	m_context->UpdateSubresource(m_cbLightPos.get(), 0, 0, positions, 0, 0);

	XMFLOAT4 colors[5];
	ZeroMemory(colors, sizeof(XMFLOAT4) * 5);
	colors[0] = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f); //ambient color
	colors[1] = XMFLOAT4(1.0f, 0.8f, 1.0f, 100.0f); //surface [ka, kd, ks, m]
	colors[2] = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); //white light color
	//TODO: write the rest of code here
	m_context->UpdateSubresource(m_cbLightColors.get(), 0, 0, colors, 0, 0);
}

void Butterfly::SetSurfaceColor(const XMFLOAT4& color)
{
	m_context->UpdateSubresource(m_cbSurfaceColor.get(), 0, 0, &color, 0, 0);
}

void Butterfly::Update(float dt)
{
	UpdateButterfly(dt);
	static MouseState prevState;
	MouseState currentState;
	if (!m_mouse->GetState(currentState))
		return;
	bool change = true;
	if (prevState.isButtonDown(0))
	{
		POINT d = currentState.getMousePositionChange();
		m_camera.Rotate(d.y/300.f, d.x/300.f);
	}
	else if (prevState.isButtonDown(1))
	{
		POINT d = currentState.getMousePositionChange();
		m_camera.Zoom(d.y/10.0f);
	}
	else
		change = false;
	prevState = currentState;
	if (change)
		UpdateCamera();
}

void Butterfly::DrawBox()
{
	const XMMATRIX worldMtx = XMMatrixIdentity();
	m_context->UpdateSubresource(m_cbWorld.get(), 0, 0, &worldMtx, 0, 0);
	
	ID3D11Buffer* b = m_vbBox.get();
	m_context->IASetVertexBuffers(0, 1, &b, &VB_STRIDE, &VB_OFFSET);
	m_context->IASetIndexBuffer(m_ibBox.get(), DXGI_FORMAT_R16_UINT, 0);
	m_context->DrawIndexed(36, 0, 0);
}

void Butterfly::DrawDodecahedron(bool colors)
//Draw dodecahedron. If color is true, use render faces with coresponding colors. Otherwise render using white color
{
	//TODO: write code here
	const int FACE_COUNT = 12;
	for(int i = 0; i < FACE_COUNT; i++){
		m_context->UpdateSubresource(m_cbWorld.get(), 0, 0, &m_dodecahedronMtx[i], 0, 0);
	
		ID3D11Buffer* b = m_vbPentagon.get();
		m_context->IASetVertexBuffers(0, 1, &b, &VB_STRIDE, &VB_OFFSET);
		m_context->IASetIndexBuffer(m_ibPentagon.get(), DXGI_FORMAT_R16_UINT, 0);
		m_context->DrawIndexed(9, 0, 0);
	}
}

void Butterfly::DrawMoebiusStrip()
//Draw the Moebius strip
{
	//TODO: write code here
}

void Butterfly::DrawButterfly()
//Draw the butterfly
{
	//TODO: write code here
}

void Butterfly::DrawBilboards()
//Setup bilboards rendering and draw them
{

}

void Butterfly::DrawMirroredWorld(int i)
//Draw the mirrored scene reflected in the i-th dodecahedron face
{
	//Setup render state for writing to the stencil buffer

	//Draw the i-th face

	//Setup render state and view matrix for rendering the mirrored world

	//Draw objects

	//Restore rendering state to it's original values

}

void Butterfly::Render()
{
	if (m_context == nullptr)
		return;
	//Clear buffers
	float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_context->ClearRenderTargetView(m_backBuffer.get(), clearColor);
	m_context->ClearDepthStencilView(m_depthStencilView.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	
	//Render box with all three lights
	SetLight1();
	SetSurfaceColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	DrawBox();

	//render mirrored worlds
	for (int i = 0; i < 12; ++i)
		DrawMirroredWorld(i);

	//render dodecahedron with one light and alpha blending
	m_context->OMSetBlendState(m_bsAlpha.get(), 0, BS_MASK);
	SetLight0();
	DrawDodecahedron(true);
	m_context->OMSetBlendState(0, 0, BS_MASK);

	//render the rest of the scene with all lights
	SetLight1();
	DrawMoebiusStrip();
	DrawButterfly();
	DrawBilboards();

	m_swapChain->Present(0, 0);
}
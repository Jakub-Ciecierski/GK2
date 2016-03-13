#include "gk2_butterfly.h"
#include "gk2_utils.h"
#include "gk2_vertices.h"
#include "gk2_window.h"
#include "resource.h"

using namespace std;
using namespace gk2;
using namespace DirectX;

#define RESOURCES_PATH L"resources/"
//const wstring Butterfly::ShaderFile = RESOURCES_PATH L"shaders/Butterfly.hlsl";

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

const XMFLOAT4 Butterfly::GREEN_LIGHT_POS = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
const XMFLOAT4 Butterfly::BLUE_LIGHT_POS = XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f);

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
	auto hMod = GetModuleHandle(nullptr);
	auto hRsrc = FindResource(hMod, MAKEINTRESOURCE(ID_VS), L"bytearray");
	auto hByteCode = LoadResource(hMod, hRsrc);
	const void* vsByteCode = LockResource(hByteCode);
	size_t vsByteCodeLength = SizeofResource(hMod, hRsrc);
	m_vertexShader = m_device.CreateVertexShader(vsByteCode, vsByteCodeLength);
	m_inputLayout = m_device.CreateInputLayout<VertexPosNormal>(vsByteCode, vsByteCodeLength);
	hRsrc = FindResource(hMod, MAKEINTRESOURCE(ID_PS), L"bytearray");
	hByteCode = LoadResource(hMod, hRsrc);
	m_pixelShader = m_device.CreatePixelShader(LockResource(hByteCode), SizeofResource(hMod, hRsrc));
	hRsrc = FindResource(hMod, MAKEINTRESOURCE(ID_BILBVS), L"bytearray");
	hByteCode = LoadResource(hMod, hRsrc);
	vsByteCode = LockResource(hByteCode);
	vsByteCodeLength = SizeofResource(hMod, hRsrc);
	m_vsBilboard = m_device.CreateVertexShader(vsByteCode, vsByteCodeLength);
	m_ilBilboard = m_device.CreateInputLayout<VertexPos>(vsByteCode, vsByteCodeLength);
	hRsrc = FindResource(hMod, MAKEINTRESOURCE(ID_BILBPS), L"bytearray");
	hByteCode = LoadResource(hMod, hRsrc);
	m_psBilboard = m_device.CreatePixelShader(LockResource(hByteCode), SizeofResource(hMod, hRsrc));
}

void Butterfly::InitializeConstantBuffers()
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.ByteWidth = sizeof(XMMATRIX);
	desc.Usage = D3D11_USAGE_DEFAULT;
	m_cbWorld = m_device.CreateBuffer(desc);
	m_cbProj = m_device.CreateBuffer(desc);
	desc.ByteWidth = sizeof(XMMATRIX)*2;
	m_cbView = m_device.CreateBuffer(desc);
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

	////  DSS WRITE ///
	// Front Face
	dssDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS; // Always pass FrontFace
	dssDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE; // When pass. replace with the stencil buffer
	dssDesc.StencilEnable = true;

	// Back Faces never pass
	dssDesc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

	//Setup depth stencil state for writing
	m_dssWrite = m_device.CreateDepthStencilState(dssDesc);

	////  DSS TEST ///
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	dssDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;

	//Setup depth stencil state for testing
	m_dssTest = m_device.CreateDepthStencilState(dssDesc);

	D3D11_RASTERIZER_DESC rsDesc = m_device.DefaultRasterizerDesc();
	rsDesc.FrontCounterClockwise = true;

	//Set rasterizer state front face to ccw
	m_rsCounterClockwise = m_device.CreateRasterizerState(rsDesc);

	D3D11_BLEND_DESC bsDesc = m_device.DefaultBlendDesc();
	//Setup alpha blending
	
	bsDesc.RenderTarget[0].BlendEnable = true;
	bsDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bsDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bsDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bsDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bsDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	bsDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bsDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	m_bsAlpha = m_device.CreateBlendState(bsDesc);

	m_context->OMSetBlendState(m_bsAlpha.get(), 0, 0xffffffff);
}

void Butterfly::InitializeCamera()
{
	auto s = getMainWindow()->getClientSize();
	auto ar = static_cast<float>(s.cx) / s.cy;
	m_projMtx = XMMatrixPerspectiveFovLH(XM_PIDIV4, ar, 0.01f, 100.0f);
	m_context->UpdateSubresource(m_cbProj.get(), 0, nullptr, &m_projMtx, 0, 0);
	m_camera.Zoom(5);
	UpdateCamera(m_camera.GetViewMatrix());
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
	for (auto i = 0; i < 5; ++i, a -= da)
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
	m_dodecahedronMtx[0] = XMMatrixRotationX(XM_PIDIV2) * XMMatrixTranslation(0, -DODECAHEDRON_H/2, 0) * XMMatrixScaling(2.0f, 2.0f, 2.0f);
	auto da = XM_2PI / 5.0f, a = da;
	for (auto i = 1; i < 6; ++i, a+= da)
	{
		m_dodecahedronMtx[i] =
			XMMatrixRotationZ(XM_PIDIV2) *
			XMMatrixTranslation(0, DODECAHEDRON_R, 0) *
			XMMatrixRotationX(DODECAHEDRON_A - XM_PIDIV2) *
			XMMatrixTranslation(0, -DODECAHEDRON_H/2, DODECAHEDRON_R) *
			XMMatrixRotationY(a - XM_PIDIV2) *
			XMMatrixScaling(2.0f, 2.0f, 2.0f);
		m_dodecahedronMtx[i + 6] = m_dodecahedronMtx[i] * XMMatrixRotationZ(XM_PI);
	}
	m_dodecahedronMtx[6] = m_dodecahedronMtx[0]  * XMMatrixRotationZ(XM_PI);

	//Draw the i-th face
	const int DODECANHEDRON_FACES_COUNT = 12;
	XMVECTOR det;
	for (int j = 0; j < DODECANHEDRON_FACES_COUNT; j++) {
		m_mirrorMtx[j] = XMMatrixInverse(&det, m_dodecahedronMtx[j])
			* XMMatrixScaling(1, 1, -1) * m_dodecahedronMtx[j];
	}
}

XMFLOAT3 Butterfly::MoebiusStripPos(float t, float s)
//Compute the position of point on the Moebius strip for parameters t and s
{
	auto temp = MOEBIUS_R + MOEBIUS_W * s * XMScalarCos(0.5f * t);
	return XMFLOAT3(XMScalarCos(t) * temp, MOEBIUS_W * s * XMScalarSin(0.5f * t), XMScalarSin(t) * temp);
}

XMVECTOR Butterfly::MoebiusStripDt(float t, float s)
//Compute the t-derivative of point on the Moebius strip for parameters t and s
{
	auto rpwscost2 = MOEBIUS_R + MOEBIUS_W*s*XMScalarCos(0.5f*t);
	auto ws2sint2 = 0.5f*MOEBIUS_W*s*XMScalarSin(0.5f*t);
	auto cost =  XMScalarCos(t);
	auto sint = XMScalarSin(t);
	XMFLOAT3 dt(-rpwscost2 * sint - ws2sint2 * cost,
				0.5f*MOEBIUS_W*s*cost,
				rpwscost2 * cost - ws2sint2 * sint);
	return XMVector3Normalize(XMLoadFloat3(&dt));
}

XMVECTOR Butterfly::MoebiusStripDs(float t, float s)
// Return the s-derivative of point on the Moebius strip for parameters t and s
{
	auto temp = XMScalarCos(0.5f * t);
	XMFLOAT3 ds(temp * XMScalarCos(t), XMScalarSin(0.5f * t), temp * XMScalarSin(t));
	return XMVector3Normalize(XMLoadFloat3(&ds));
}

void Butterfly::InitializeMoebiusStrip()
//Create vertex and index buffers for the Moebius strip
{
	VertexPosNormal vertices[4 * MOEBIUS_N]; //2 sides, N parts per side, 2 vertices per part
	float t = 0, dt = XM_2PI / MOEBIUS_N;
	auto i = 0;
	for (; i < 2 * MOEBIUS_N; ++i, t += dt)
	{
		vertices[2 * i].Pos = MoebiusStripPos(t, -1);
		XMVECTOR normal =  XMVector3Normalize(XMVector3Cross(MoebiusStripDs(t, -1), MoebiusStripDt(t, -1)));
		XMStoreFloat3(&vertices[2 * i].Normal, normal);
		vertices[2 * i + 1].Pos = MoebiusStripPos(t, 1);
		normal =  XMVector3Normalize(XMVector3Cross(MoebiusStripDs(t, 1), MoebiusStripDt(t, 1)));
		XMStoreFloat3(&vertices[2 * i + 1].Normal, normal);
	}
	m_vbMoebius = m_device.CreateVertexBuffer(vertices, 4 * MOEBIUS_N);
	unsigned short indices[12 * MOEBIUS_N]; //2 sides, N parts per side, 2 triangles per part, 3 veritces per triangle
	i = 0;
	for (; i < 2 * MOEBIUS_N - 1; ++i)
	{
		indices[6 * i] = 2 * i; indices[6 * i + 1] = 2 * i + 1; indices[6 * i + 2] = 2 * i + 3;
		indices[6 * i + 3] = 2 * i; indices[6 * i + 4] = 2 * i + 3; indices[6 * i + 5] = 2 * i + 2;
	}
	indices[6 * i] = 2 * i; indices[6 * i + 1] = 2 * i + 1; indices[6 * i + 2] = 1;
	indices[6 * i + 3] = 2 * i; indices[6 * i + 4] = 1; indices[6 * i + 5] = 0;
	m_ibMoebius = m_device.CreateIndexBuffer(indices, 12 * MOEBIUS_N);
}

void Butterfly::InitializeButterfly()
//Create vertex and index buffers for the butterfly wing
{
	//TODO: write code here
	
	VertexPosNormal vertices[] =
	{
		{ XMFLOAT3(-1, 1, 0),	XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1, 1, 0),	XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1, -1, 0),	XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-1, -1, 0),	XMFLOAT3(0.0f, 0.0f, 1.0f) },
		// DUPLICATE
		{ XMFLOAT3(-1, 1, 0),	XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(1, 1, 0),	XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(1, -1, 0),	XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(-1, -1, 0),	XMFLOAT3(0.0f, 0.0f, -1.0f) }
	};
	
	m_vbWing = m_device.CreateVertexBuffer(vertices, 8);
	unsigned short indices[] =
	{
		0,1,2,
		0,2,3,
		5,4,7,
		6,5,7
	};
	m_ibWing = m_device.CreateIndexBuffer(indices, 12);
	
}

void Butterfly::InitializeBilboards()
//Initialize bilboard resources (vertex, pixel shaders, input layout, vertex, index buffers etc.)
{
	//TODO: write code here
	VertexPos vertices[] =
	{
		{ XMFLOAT3(-1,	1,	0)},
		{ XMFLOAT3(1,	1,	0)},
		{ XMFLOAT3(1,	-1, 0)},
		{ XMFLOAT3(-1,	-1, 0)}
	};
	m_vbBilboard = m_device.CreateVertexBuffer(vertices, 4);
	unsigned short indices[] = { 0, 1, 2, 0, 2, 3 };
	m_ibBilboard = m_device.CreateIndexBuffer(indices, 6);

	
	m_vsBilboard;
	m_psBilboard;

}

void Butterfly::SetShaders() const
{
	m_context->VSSetShader(m_vertexShader.get(), nullptr, 0);
	m_context->PSSetShader(m_pixelShader.get(), nullptr, 0);
	m_context->IASetInputLayout(m_inputLayout.get());
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Butterfly::SetBilboardShaders() const
{
	m_context->VSSetShader(m_vsBilboard.get(), nullptr, 0);
	m_context->PSSetShader(m_psBilboard.get(), nullptr, 0);
	m_context->IASetInputLayout(m_ilBilboard.get());
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Butterfly::SetConstantBuffers() const
{
	ID3D11Buffer* vsb[] = { m_cbWorld.get(),  m_cbView.get(),  m_cbProj.get(), m_cbLightPos.get() };
	m_context->VSSetConstantBuffers(0, 4, vsb);
	ID3D11Buffer* psb[] = { m_cbSurfaceColor.get(), m_cbLightColors.get() };
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
	m_vsBilboard.reset();
	m_psBilboard.reset();
	m_ilBilboard.reset();

	m_dssWrite.reset();
	m_dssTest.reset();
	m_rsCounterClockwise.reset();
	m_bsAlpha.reset();
	m_bsAdd.reset();

	m_vbBox.reset();
	m_ibBox.reset();
	m_vbPentagon.reset();
	m_ibPentagon.reset();
	m_vbMoebius.reset();
	m_ibMoebius.reset();
	m_vbWing.reset();
	m_ibWing.reset();
	m_vbBilboard.reset();
	m_ibBilboard.reset();

	m_cbWorld.reset();
	m_cbView.reset();
	m_cbProj.reset();
	m_cbLightPos.reset();
	m_cbLightColors.reset();
	m_cbSurfaceColor.reset();
}

void Butterfly::UpdateCamera(const XMMATRIX& view) const
{
	XMMATRIX viewMtx[2];
	viewMtx[0] = view;
	XMVECTOR det;
	viewMtx[1] = XMMatrixInverse(&det, viewMtx[0]);
	m_context->UpdateSubresource(m_cbView.get(), 0, nullptr, viewMtx, 0, 0);
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
	float t = 2 * lap / LAP_TIME;
	//Angle between wing current and vertical position
	float a = t * WING_MAX_A;
	t *= XM_2PI;
	if (a > WING_MAX_A)
		a = 2 * WING_MAX_A - a;

	//TODO: write the rest of code here

	
	XMFLOAT3 p_tmp = MoebiusStripPos(t, 0);
	XMVECTOR p = XMLoadFloat3(&p_tmp);
	XMVECTOR ps = MoebiusStripDs(t, 0);
	XMVECTOR pt = MoebiusStripDt(t, 0);
	XMVECTOR pn = XMVector3Normalize(XMVector3Cross(pt, ps));
	XMMATRIX B;
	B.r[0] = pt;
	B.r[1] = pn;
	B.r[2] = ps;
	B.r[3] = p;
	B.r[3].m128_f32[3] = 1;

	for (int i = 0; i < 2; i++) {
		m_wingMtx[i] = XMMatrixTranslation(0, 1, 0) *
			XMMatrixScaling(WING_W / 2, WING_H / 2, 1) *
			XMMatrixRotationX(pow(-1,i) * a) * B;
	}
	

}

void Butterfly::SetLight0() const
//Setup one positional light at the camera
{
	XMFLOAT4 positions[3];
	ZeroMemory(positions, sizeof(XMFLOAT4) * 3);
	positions[0] = m_camera.GetPosition();
	m_context->UpdateSubresource(m_cbLightPos.get(), 0, nullptr, positions, 0, 0);

	XMFLOAT4 colors[5];
	ZeroMemory(colors, sizeof(XMFLOAT4) * 5);
	colors[0] = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f); //ambient color
	colors[1] = XMFLOAT4(1.0f, 0.8f, 1.0f, 100.0f); //surface [ka, kd, ks, m]
	colors[2] = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); //light0 color
	m_context->UpdateSubresource(m_cbLightColors.get(), 0, nullptr, colors, 0, 0);
}

void Butterfly::SetLight1() const
//Setup one white positional light at the camera
//Setup two additional positional lights, green and blue.
{
	XMFLOAT4 positions[3];
	ZeroMemory(positions, sizeof(XMFLOAT4) * 3);
	positions[0] = m_camera.GetPosition(); //white light position
	positions[1] = GREEN_LIGHT_POS; //green light position
	positions[2] = BLUE_LIGHT_POS; //blue light position
	m_context->UpdateSubresource(m_cbLightPos.get(), 0, nullptr, positions, 0, 0);

	XMFLOAT4 colors[5];
	ZeroMemory(colors, sizeof(XMFLOAT4) * 5);
	colors[0] = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f); //ambient color
	colors[1] = XMFLOAT4(1.0f, 0.8f, 1.0f, 200.0f); //surface [ka, kd, ks, m]
	colors[2] = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); //white light color
	colors[3] = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); // Green light
	colors[4] = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f); // Blue light
	//TODO: write the rest of code here
	m_context->UpdateSubresource(m_cbLightColors.get(), 0, nullptr, colors, 0, 0);
}

void Butterfly::SetSurfaceColor(const XMFLOAT4& color) const
{
	m_context->UpdateSubresource(m_cbSurfaceColor.get(), 0, nullptr, &color, 0, 0);
}

void Butterfly::Update(float dt)
{
	UpdateButterfly(dt);
	static MouseState prevState;
	MouseState currentState;
	if (!m_mouse->GetState(currentState))
		return;
	auto change = true;

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
		UpdateCamera(m_camera.GetViewMatrix());
}

void Butterfly::DrawBox() const
{
	const auto worldMtx = XMMatrixIdentity();
	m_context->UpdateSubresource(m_cbWorld.get(), 0, nullptr, &worldMtx, 0, 0);

	auto b = m_vbBox.get();
	m_context->IASetVertexBuffers(0, 1, &b, &VB_STRIDE, &VB_OFFSET);
	m_context->IASetIndexBuffer(m_ibBox.get(), DXGI_FORMAT_R16_UINT, 0);
	m_context->DrawIndexed(36, 0, 0);
}

void Butterfly::DrawDodecahedron(bool colors) const
//Draw dodecahedron. If color is true, use render faces with coresponding colors. Otherwise render using white color
{
	auto b = m_vbPentagon.get();
	m_context->IASetVertexBuffers(0, 1, &b, &VB_STRIDE, &VB_OFFSET);
	m_context->IASetIndexBuffer(m_ibPentagon.get(), DXGI_FORMAT_R16_UINT, 0);
	for (auto i = 0; i < 12; ++i)
	{
		if (colors) {
			SetSurfaceColor(COLORS[i]);
		}
		m_context->UpdateSubresource(m_cbWorld.get(), 0, nullptr, &m_dodecahedronMtx[i], 0, 0);
		m_context->DrawIndexed(9, 0, 0);
	}
}

void Butterfly::DrawMoebiusStrip() const
//Draw the Moebius strip
{
	const auto worldMtx = XMMatrixIdentity();
	m_context->UpdateSubresource(m_cbWorld.get(), 0, nullptr, &worldMtx, 0, 0);

	auto b = m_vbMoebius.get();
	m_context->IASetVertexBuffers(0, 1, &b, &VB_STRIDE, &VB_OFFSET);
	m_context->IASetIndexBuffer(m_ibMoebius.get(), DXGI_FORMAT_R16_UINT, 0);
	m_context->DrawIndexed(12 * MOEBIUS_N, 0, 0);
}

void Butterfly::DrawButterfly() const
//Draw the butterfly
{
	for (int i = 0;i < 2; i++) {
		const auto worldMtx = m_wingMtx[i];
		m_context->UpdateSubresource(m_cbWorld.get(), 0, nullptr, &worldMtx, 0, 0);

		auto b = m_vbWing.get();
		m_context->IASetVertexBuffers(0, 1, &b, &VB_STRIDE, &VB_OFFSET);
		m_context->IASetIndexBuffer(m_ibWing.get(), DXGI_FORMAT_R16_UINT, 0);
		m_context->DrawIndexed(12, 0, 0);
	}
}

void Butterfly::DrawBilboards() const
//Setup bilboards rendering and draw them
{
	SetBilboardShaders();

	const auto worldMtx = XMMatrixTranslation(GREEN_LIGHT_POS.x, GREEN_LIGHT_POS.y, GREEN_LIGHT_POS.z);
	m_context->UpdateSubresource(m_cbWorld.get(), 0, nullptr, &worldMtx, 0, 0);

	auto b = m_vbBilboard.get();
	m_context->IASetVertexBuffers(0, 1, &b, &VB_STRIDE, &VB_OFFSET);
	m_context->IASetIndexBuffer(m_ibBilboard.get(), DXGI_FORMAT_R16_UINT, 0);
	m_context->DrawIndexed(6, 0, 0);

	const int BILBOARD_COUNT = 0;
	//TODO: write code here
	for (int i = 0; i < BILBOARD_COUNT; i++) {

	}

	SetShaders();
}

void Butterfly::DrawMirroredWorld(int i)
//Draw the mirrored scene reflected in the i-th dodecahedron face
{
	m_context->OMSetDepthStencilState(m_dssWrite.get(), i+1); // NEW
	
	// Draw i-th Face
	auto b = m_vbPentagon.get();
	m_context->IASetVertexBuffers(0, 1, &b, &VB_STRIDE, &VB_OFFSET);
	m_context->IASetIndexBuffer(m_ibPentagon.get(), DXGI_FORMAT_R16_UINT, 0);
	m_context->UpdateSubresource(m_cbWorld.get(), 0, nullptr, &m_dodecahedronMtx[i], 0, 0);
	m_context->DrawIndexed(9, 0, 0);

	m_context->OMSetDepthStencilState(m_dssTest.get(),i+ 1); // NEW

	//Setup render state and view matrix for rendering the mirrored world
	DirectX::XMMATRIX ViewMirror = m_mirrorMtx[i] * m_camera.GetViewMatrix();
	UpdateCamera(ViewMirror);

	//Setup render state for writing to the stencil buffer
	m_context->RSSetState(m_rsCounterClockwise.get());

	//Draw objects
	//DrawBox();
	DrawDodecahedron(false);
	DrawMoebiusStrip();
	DrawButterfly();
	DrawBilboards();

	//Restore Camera to its original values
	UpdateCamera(m_camera.GetViewMatrix());

	//Restore rendering state to it's original values
	m_context->RSSetState(nullptr);
	m_context->OMSetDepthStencilState(nullptr,0);
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
	//DrawBox();

	//render mirrored worlds
	for (int i = 0; i < 12; ++i)
		DrawMirroredWorld(i);

	
	//render dodecahedron with one light and alpha blending
	m_context->OMSetBlendState(m_bsAlpha.get(), nullptr, BS_MASK);
	SetLight0();
	DrawDodecahedron(true);

	m_context->OMSetBlendState(nullptr, nullptr, BS_MASK);

	//render the rest of the scene with all lights
	SetLight1();
	DrawMoebiusStrip();
	DrawButterfly();
	DrawBilboards();

	m_swapChain->Present(0, 0);
}
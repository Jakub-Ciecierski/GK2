#include "gk2_applicationBase.h"
#include "gk2_window.h"

using namespace std;
using namespace gk2;

ApplicationBase::ApplicationBase(HINSTANCE hInstance)
	: m_driverType(D3D_DRIVER_TYPE_NULL), m_featureLevel(D3D_FEATURE_LEVEL_11_0),
	  m_hInstance(hInstance)
{

}

ApplicationBase::~ApplicationBase()
{
	Shutdown();
}

bool ApplicationBase::LoadContent()
{
	return true;
}

void ApplicationBase::UnloadContent()
{

}

void ApplicationBase::FillSwapChainDesc(DXGI_SWAP_CHAIN_DESC& desc, int width, int height) const
{
	ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC));
	desc.BufferCount = 1;
	desc.BufferDesc.Width = width;
	desc.BufferDesc.Height = height;
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BufferDesc.RefreshRate.Numerator = 60;
	desc.BufferDesc.RefreshRate.Denominator = 1;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.OutputWindow = getMainWindow()->getHandle();
	desc.Windowed = true;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
}

void ApplicationBase::CreateDeviceAndSwapChain(SIZE windowSize)
{
	D3D_DRIVER_TYPE driverTypes[] = { D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP, D3D_DRIVER_TYPE_SOFTWARE };
	UINT driverTypesCount = ARRAYSIZE(driverTypes);
	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
	UINT featureLevelsCout = ARRAYSIZE(featureLevels);
	DXGI_SWAP_CHAIN_DESC desc;
	FillSwapChainDesc(desc, windowSize.cx, windowSize.cy);
	unsigned int creationFlags = 0;
#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	HRESULT result;
	for (unsigned int driver = 0; driver < driverTypesCount; ++driver)
	{
		ID3D11Device* device = nullptr;
		ID3D11DeviceContext* context = nullptr;
		IDXGISwapChain* swapChain = nullptr;
		result = D3D11CreateDeviceAndSwapChain(nullptr, driverTypes[driver], nullptr, creationFlags, featureLevels,
			featureLevelsCout, D3D11_SDK_VERSION, &desc, &swapChain, &device, &m_featureLevel, &context);
		m_device.m_deviceObject.reset(device, Utils::COMRelease);
		m_swapChain = unique_ptr_del<IDXGISwapChain>(swapChain, Utils::COMRelease);
		m_context = unique_ptr_del<ID3D11DeviceContext>(context, Utils::COMRelease);
		if (SUCCEEDED(result))
		{
			m_driverType = driverTypes[driver];
			return;
		}
	}
	THROW_DX11(result);
}

void ApplicationBase::CreateBackBuffers(SIZE windowSize)
{
	ID3D11Texture2D* bbt;
	HRESULT result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&bbt));
	unique_ptr_del<ID3D11Texture2D> backBufferTexture(bbt, Utils::COMRelease);
	if (FAILED(result))
		THROW_DX11(result);
	m_backBuffer = m_device.CreateRenderTargetView(backBufferTexture);
	backBufferTexture.reset();
	m_depthStencilTexture = m_device.CreateDepthStencilTexture(windowSize);
	m_depthStencilView = m_device.CreateDepthStencilView(m_depthStencilTexture);
	ID3D11RenderTargetView* backBuffer = m_backBuffer.get();
	m_context->OMSetRenderTargets(1, &backBuffer, m_depthStencilView.get());
}

void ApplicationBase::SetViewPort(SIZE windowSize) const
{
	D3D11_VIEWPORT viewport;
	viewport.Width = static_cast<float>(windowSize.cx);
	viewport.Height = static_cast<float>(windowSize.cy);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	m_context->RSSetViewports(1, &viewport);
}

void ApplicationBase::InitializeDirectInput()
{
	IDirectInput8W* di;
	HRESULT resutl = DirectInput8Create(getHandle(), DIRECTINPUT_VERSION, IID_IDirectInput8W,
		reinterpret_cast<void**>(&di), nullptr);
	m_input.m_inputObject = unique_ptr_del<IDirectInput8W>(di, Utils::COMRelease);
	if (FAILED(resutl))
		THROW_DX11(resutl);
	m_keyboard.reset(new Keyboard(m_input.CreateInputDevice(m_mainWindow->getHandle(), GUID_SysKeyboard, c_dfDIKeyboard)));
	m_mouse.reset(new Mouse(m_input.CreateInputDevice(m_mainWindow->getHandle(), GUID_SysMouse, c_dfDIMouse)));
}

bool ApplicationBase::Initialize()
{
	auto windowSize = getMainWindow()->getClientSize();
	CreateDeviceAndSwapChain(windowSize);
	CreateBackBuffers(windowSize);
	SetViewPort(windowSize);
	InitializeDirectInput();
	return LoadContent();
}


int ApplicationBase::MainLoop()
{
	MSG msg = { nullptr };
	DWORD dwTimeStart = 0;
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			auto dwTimeCur = GetTickCount();
			if( dwTimeStart == 0 )
				dwTimeStart = dwTimeCur;
			auto t = ( dwTimeCur - dwTimeStart ) / 1000.0f;
			dwTimeStart = dwTimeCur;
			Update(t);
			Render();
		}
	}
	Shutdown();
	return static_cast<int>(msg.wParam);
}

void ApplicationBase::Shutdown()
{
	UnloadContent();
	m_depthStencilTexture.reset();
	m_depthStencilView.reset();
	m_backBuffer.reset();
	m_swapChain.reset();
	m_context.reset();
	m_device.m_deviceObject.reset();
	m_keyboard.reset();
	m_mouse.reset();
	m_input.m_inputObject.reset();
}

int ApplicationBase::Run(unique_ptr<Window>&& w, int cmdShow)
{
	m_mainWindow = move(w);
	if (!Initialize())
		return -1;
	m_mainWindow->Show(cmdShow);
	return MainLoop();
}

#ifndef __GK2_APPLICATION_BASE_H_
#define __GK2_APPLICATION_BASE_H_

#include <d3d11.h>
#include <D3DX11.h>
#include <dinput.h>
#include "gk2_input.h"
#include "gk2_deviceHelper.h"

namespace gk2
{
	class Window;

	class ApplicationBase
	{
	public:
		ApplicationBase(HINSTANCE hInstance);
		virtual ~ApplicationBase();

		int Run(gk2::Window* window, int cmdShow);
		
		inline HINSTANCE getHandle() const { return m_hInstance; }
		inline gk2::Window* getMainWindow() const { return m_mainWindow; }

	protected:
		bool Initialize();
		int MainLoop();
		void Shutdown();

		virtual bool LoadContent();
		virtual void UnloadContent();
		virtual void Update(float dt) = 0;
		virtual void Render() = 0;

		D3D_DRIVER_TYPE m_driverType;
		D3D_FEATURE_LEVEL m_featureLevel;

		gk2::DeviceHelper m_device;
		std::shared_ptr<ID3D11DeviceContext> m_context;
		std::shared_ptr<IDXGISwapChain> m_swapChain;
		std::shared_ptr<ID3D11RenderTargetView> m_backBuffer;
		std::shared_ptr<ID3D11Texture2D> m_depthStencilTexture;
		std::shared_ptr<ID3D11DepthStencilView> m_depthStencilView;
		gk2::InputHelper m_input;
		std::shared_ptr<Keyboard> m_keyboard;
		std::shared_ptr<Mouse> m_mouse;

	private:
		HINSTANCE m_hInstance;
		gk2::Window* m_mainWindow;

		void FillSwapChainDesc(DXGI_SWAP_CHAIN_DESC& desc, int width, int height);
		void CreateDeviceAndSwapChain(SIZE windowSize);
		void CreateBackBuffers(SIZE windowSize);
		void InitializeDirectInput();
		void SetViewPort(SIZE windowSize);
	};
}

#endif __GK2_APPLICATION_BASE_H_

#ifndef __GK2_DEVICE_HELPER_H_
#define __GK2_DEVICE_HELPER_H_

#include <memory>
#include <d3d11.h>
#include <vector>
#include "gk2_utils.h"

namespace gk2
{
	class DeviceHelper
	{
	public:
		std::shared_ptr<ID3D11Device> m_deviceObject;

		/*std::shared_ptr<ID3DBlob> CompileD3DShader(const std::wstring& filePath, const std::string&  entry,
												   const std::string&  shaderModel);*/
		unique_ptr_del<ID3D11VertexShader> CreateVertexShader(const void* byteCode, size_t byteCodeLength) const;
		unique_ptr_del<ID3D11PixelShader> CreatePixelShader(const void* byteCode, size_t byteCodeLength) const;
		unique_ptr_del<ID3D11InputLayout> CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* layout,
															 unsigned int layoutElements,
															 const void* vsByteCode,
															 size_t vsByteCodeLength) const;

		template<typename T>
		unique_ptr_del<ID3D11InputLayout> CreateInputLayout(const void* vsByteCode, size_t vsByteCodeLength)
		{
			return CreateInputLayout(T::Layout, T::LayoutElements, vsByteCode, vsByteCodeLength);
		}


		template<typename T>
		unique_ptr_del<ID3D11Buffer> CreateVertexBuffer(const std::vector<T>& vertices)
		{
			return _CreateBufferInternal(reinterpret_cast<const void*>(vertices.data()),	sizeof(T) * vertices.size(),
				D3D11_BIND_VERTEX_BUFFER);
		}

		template<typename T>
		unique_ptr_del<ID3D11Buffer> CreateVertexBuffer(const T* vertices, unsigned int count)
		{
			return _CreateBufferInternal(reinterpret_cast<const void*>(vertices), sizeof(T) * count,
				D3D11_BIND_VERTEX_BUFFER);
		}

		unique_ptr_del<ID3D11Buffer> CreateIndexBuffer(const unsigned short* indices, unsigned int count) const
		{
			return _CreateBufferInternal(reinterpret_cast<const void*>(indices), sizeof(unsigned short) * count,
				D3D11_BIND_INDEX_BUFFER);
		}
		unique_ptr_del<ID3D11Buffer> CreateBuffer(const D3D11_BUFFER_DESC& desc, const void* pData = nullptr) const;

		//std::shared_ptr<ID3D11ShaderResourceView> CreateShaderResourceView(const std::wstring& fileName);
		unique_ptr_del<ID3D11SamplerState> CreateSamplerState() const;
		unique_ptr_del<ID3D11Texture2D> CreateDepthStencilTexture(SIZE size) const;
		unique_ptr_del<ID3D11RenderTargetView> CreateRenderTargetView(
													const unique_ptr_del<ID3D11Texture2D>& backBufferTexture) const;
		unique_ptr_del<ID3D11DepthStencilView> CreateDepthStencilView(
													const unique_ptr_del<ID3D11Texture2D>& depthStencilTexture) const;
		static D3D11_DEPTH_STENCIL_DESC DefaultDepthStencilDesc();
		unique_ptr_del<ID3D11DepthStencilState> CreateDepthStencilState(D3D11_DEPTH_STENCIL_DESC& desc) const;
		static D3D11_RASTERIZER_DESC DefaultRasterizerDesc();
		unique_ptr_del<ID3D11RasterizerState> CreateRasterizerState(D3D11_RASTERIZER_DESC& desc) const;
		static D3D11_BLEND_DESC DefaultBlendDesc();
		unique_ptr_del<ID3D11BlendState> CreateBlendState(D3D11_BLEND_DESC& desc) const;

	private:
		unique_ptr_del<ID3D11Buffer> _CreateBufferInternal(const void* pData, unsigned int byteWidth,
			D3D11_BIND_FLAG bindFlags) const;
	};
}

#endif __GK2_DEVICE_HELPER_H_

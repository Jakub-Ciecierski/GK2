#ifndef __GK2_ENVIRONMENT_MAPPER_H_
#define __GK2_ENVIRONMENT_MAPPER_H_

#include <memory>
#include <d3d11.h>
#include <string>
#include "gk2_constantBuffer.h"
#include "gk2_deviceHelper.h"
#include "gk2_effectBase.h"

namespace gk2
{
	class EnvironmentMapper : public EffectBase
	{
	public:
		EnvironmentMapper(DeviceHelper& device, std::shared_ptr<ID3D11InputLayout>& layout,
						  const std::shared_ptr<ID3D11DeviceContext>& context, float nearP, float farP, DirectX::XMFLOAT3 pos);
		
		void SetSamplerState(const std::shared_ptr<ID3D11SamplerState>& samplerState);
		void SetCameraPosBuffer(const std::shared_ptr<ConstantBuffer<DirectX::XMFLOAT4>>& cameraPos);
		void SetSurfaceColorBuffer(const std::shared_ptr<ConstantBuffer<DirectX::XMFLOAT4>>& surfaceColor);

		void SetupFace(const std::shared_ptr<ID3D11DeviceContext>& context, D3D11_TEXTURECUBE_FACE face);
		void EndFace();
		
	protected:
		void SetVertexShaderData() override;
		void SetPixelShaderData() override;

	private:
		static const int TEXTURE_SIZE;
		static const std::wstring ShaderName;

		std::shared_ptr<ID3D11SamplerState> m_samplerState;
		std::shared_ptr<ConstantBuffer<DirectX::XMFLOAT4>> m_cameraPosCB;
		std::shared_ptr<ConstantBuffer<DirectX::XMFLOAT4>> m_surfaceColorCB;

		std::shared_ptr<ID3D11Texture2D> m_envTexture;
		std::shared_ptr<ID3D11ShaderResourceView> m_envTextureView;
		std::shared_ptr<ID3D11Texture2D> m_envFaceRenderTexture;
		std::shared_ptr<ID3D11RenderTargetView> m_envFaceRenderTarget;
		std::shared_ptr<ID3D11Texture2D> m_envFaceDepthTexture;
		std::shared_ptr<ID3D11DepthStencilView> m_envFaceDepthView;
		
		float m_nearPlane;
		float m_farPlane;
		DirectX::XMFLOAT4 m_position;
		D3D11_TEXTURECUBE_FACE m_face;

		void InitializeTextures(DeviceHelper& device);
	};
}

#endif __GK2_ENVIRONMENT_MAPPER_H_

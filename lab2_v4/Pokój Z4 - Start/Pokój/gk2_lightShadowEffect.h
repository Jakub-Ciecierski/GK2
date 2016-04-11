#ifndef __GK2_LIGHT_SHADOW_EFFECT_H_
#define __GK2_LIGHT_SHADOW_EFFECT_H_

#include "gk2_effectBase.h"

namespace gk2
{
	class LightShadowEffect : public EffectBase
	{
	public:
		LightShadowEffect(DeviceHelper& device, std::shared_ptr<ID3D11InputLayout>& layout,
			std::shared_ptr<ID3D11DeviceContext> context = nullptr);

		static void* operator new(size_t size);
		static void operator delete(void* ptr);

		void SetLightPosBuffer(const std::shared_ptr<ConstantBuffer<DirectX::XMFLOAT4>>& lightPos);
		void SetSurfaceColorBuffer(const std::shared_ptr<ConstantBuffer<DirectX::XMFLOAT4>>& surfaceColor);

		DirectX::XMMATRIX UpdateLight(float dt, std::shared_ptr<ID3D11DeviceContext> context);
		void SetupShadow(const std::shared_ptr<ID3D11DeviceContext>& context);
		void EndShadow();

	protected:
		void SetVertexShaderData() override;
		void SetPixelShaderData() override;

	private:
		static const std::wstring ShaderName;

		static const float LIGHT_NEAR;
		static const float LIGHT_FAR;
		static const float LIGHT_ANGLE;
		static const int TEXTURE_SIZE;

		std::shared_ptr<ConstantBuffer<DirectX::XMFLOAT4>> m_lightPosCB;
		std::shared_ptr<ConstantBuffer<DirectX::XMFLOAT4>> m_surfaceColorCB;
		std::shared_ptr<CBMatrix> m_mapMtxCB;
		std::shared_ptr<ID3D11SamplerState> m_samplerState;

		std::shared_ptr<ID3D11ShaderResourceView> m_lightMap;
		std::shared_ptr<ID3D11Texture2D> m_shadowMap;
		std::shared_ptr<ID3D11DepthStencilView> m_shadowMapDepthView;
		std::shared_ptr<ID3D11ShaderResourceView> m_shadowMapView;

		DirectX::XMMATRIX m_lightProjMtx;
		DirectX::XMMATRIX m_lightViewMtx;

		void InitializeTextures(DeviceHelper& device);
	};
}

#endif __GK2_LIGHT_SHADOW_EFFECT_H_

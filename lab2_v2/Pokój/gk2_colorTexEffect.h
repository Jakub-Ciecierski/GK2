#ifndef __GK2_COLOR_TEX_EFFECT_H_
#define __GK2_COLOR_TEX_EFFECT_H_

#include "gk2_effectBase.h"

namespace gk2
{
	class ColorTexEffect : public EffectBase
	{
	public:
		ColorTexEffect(DeviceHelper& device, std::shared_ptr<ID3D11InputLayout>& layout,
					  std::shared_ptr<ID3D11DeviceContext> context = nullptr);

		void SetTextureMtxBuffer(const std::shared_ptr<CBMatrix>& textureMtx);
		void SetSamplerState(const std::shared_ptr<ID3D11SamplerState>& samplerState);
		void SetTexture(const std::shared_ptr<ID3D11ShaderResourceView>& texture);
		void SetSurfaceColorBuffer(const std::shared_ptr<ConstantBuffer<DirectX::XMFLOAT4>>& surfaceColor);

	protected:
		void SetVertexShaderData() override;
		void SetPixelShaderData() override;

	private:
		static const std::wstring ShaderName;

		std::shared_ptr<CBMatrix> m_textureMtxCB;
		std::shared_ptr<ID3D11SamplerState> m_samplerState;
		std::shared_ptr<ID3D11ShaderResourceView> m_texture;
		std::shared_ptr<ConstantBuffer<DirectX::XMFLOAT4>> m_surfaceColorCB;
	};
}

#endif __GK2_COLOR_TEX_EFFECT_H_

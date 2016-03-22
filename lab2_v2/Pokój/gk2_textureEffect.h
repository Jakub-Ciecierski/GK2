#ifndef __GK2_TEXTURE_EFFECT_H_
#define __GK2_TEXTURE_EFFECT_H_

#include "gk2_effectBase.h"

namespace gk2
{
	class TextureEffect : public EffectBase
	{
	public:
		TextureEffect(DeviceHelper& device, std::shared_ptr<ID3D11InputLayout>& layout,
					  std::shared_ptr<ID3D11DeviceContext> context = nullptr);

		void SetTextureMtxBuffer(const std::shared_ptr<CBMatrix>& textureMtx);
		void SetSamplerState(const std::shared_ptr<ID3D11SamplerState>& samplerState);
		void SetTexture(const std::shared_ptr<ID3D11ShaderResourceView>& texture);

	protected:
		void SetVertexShaderData() override;
		void SetPixelShaderData() override;

	private:
		static const std::wstring ShaderName;

		std::shared_ptr<CBMatrix> m_textureMtxCB;
		std::shared_ptr<ID3D11SamplerState> m_samplerState;
		std::shared_ptr<ID3D11ShaderResourceView> m_texture;
	};
}

#endif __GK2_TEXTURE_EFFECT_H_

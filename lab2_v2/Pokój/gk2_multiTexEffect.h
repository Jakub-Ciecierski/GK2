#ifndef __GK2_MULTI_TEX_EFFECT_H_
#define __GK2_MULTI_TEX_EFFECT_H_

#include "gk2_effectBase.h"

namespace gk2
{
	class MultiTexEffect : public EffectBase
	{
	public:
		MultiTexEffect(DeviceHelper& device, std::shared_ptr<ID3D11InputLayout>& layout,
			std::shared_ptr<ID3D11DeviceContext> context = nullptr);

		void Set1stTextureMtxBuffer(const std::shared_ptr<CBMatrix>& textureMtx);
		void Set2ndTextureMtxBuffer(const std::shared_ptr<CBMatrix>& textureMtx);
		void SetSamplerState(const std::shared_ptr<ID3D11SamplerState>& samplerState);
		void Set1stTexture(const std::shared_ptr<ID3D11ShaderResourceView>& texture);
		void Set2ndTexture(const std::shared_ptr<ID3D11ShaderResourceView>& texture);

	protected:
		void SetVertexShaderData() override;
		void SetPixelShaderData() override;

	private:
		static const std::wstring ShaderName;

		std::shared_ptr<CBMatrix> m_1stTextureMtxCB;
		std::shared_ptr<CBMatrix> m_2ndTextureMtxCB;
		std::shared_ptr<ID3D11SamplerState> m_samplerState;
		std::shared_ptr<ID3D11ShaderResourceView> m_1stTexture;
		std::shared_ptr<ID3D11ShaderResourceView> m_2ndTexture;
	};
}

#endif __GK2_MULTI_TEX_EFFECT_H_

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
		
		void SetTextureMtxBuffer(const std::shared_ptr<CBMatrix>& textureMtx);
		//TODO: Add method to set buffer for second texture transform matrix
		void SetTextureMtxBuffer2(const std::shared_ptr<CBMatrix>& textureMtx);

		void SetSamplerState(const std::shared_ptr<ID3D11SamplerState>& samplerState);
		void SetTexture(const std::shared_ptr<ID3D11ShaderResourceView>& texture);
		//TODO: Add method to set the second texture
		void SetTexture2(const std::shared_ptr<ID3D11ShaderResourceView>& texture);
		

	protected:
		void SetVertexShaderData() override;
		void SetPixelShaderData() override;

	private:
		static const std::wstring ShaderName;

		std::shared_ptr<ID3D11SamplerState> m_samplerState;
		std::shared_ptr<ID3D11ShaderResourceView> m_texture;
		std::shared_ptr<CBMatrix> m_textureMtxCB;
		//TODO: Add fields to store second texture and its transform matrix buffer
		std::shared_ptr<ID3D11ShaderResourceView> m_texture2;
		std::shared_ptr<CBMatrix> m_textureMtxCB2;
	};
}

#endif __GK2_MULTI_TEX_EFFECT_H_

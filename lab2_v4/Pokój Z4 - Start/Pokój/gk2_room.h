#ifndef __GK2_ROOM_H_
#define __GK2_ROOM_H_

#include "gk2_applicationBase.h"
#include "gk2_meshLoader.h"
#include "gk2_camera.h"
#include "gk2_phongEffect.h"
#include "gk2_lightShadowEffect.h"
#include "gk2_constantBuffer.h"
#include "gk2_particles.h"

namespace gk2
{
	class Room : public ApplicationBase
	{
	public:
		explicit Room(HINSTANCE hInstance);
		virtual ~Room();

		static void* operator new(size_t size);
		static void operator delete(void* ptr);

	protected:
		bool LoadContent() override;
		void UnloadContent() override;

		void Update(float dt) override;
		void Render() override;

	private:
		static const float TABLE_H;
		static const float TABLE_TOP_H;
		static const float TABLE_R;
		static const DirectX::XMFLOAT4 TABLE_POS;
		static const unsigned int BS_MASK;

		Mesh m_walls[6];
		Mesh m_teapot;
		Mesh m_debugSphere;
		Mesh m_box;
		Mesh m_lamp;
		Mesh m_chairSeat;
		Mesh m_chairBack;
		Mesh m_tableLegs[4];
		Mesh m_tableSide;
		Mesh m_tableTop;
		Mesh m_monitor;
		Mesh m_screen;

		DirectX::XMMATRIX m_projMtx;

		Camera m_camera;

		std::shared_ptr<CBMatrix> m_worldCB;
		std::shared_ptr<CBMatrix> m_viewCB;
		std::shared_ptr<CBMatrix> m_projCB;
		std::shared_ptr<ConstantBuffer<DirectX::XMFLOAT4>> m_lightPosCB;
		std::shared_ptr<ConstantBuffer<DirectX::XMFLOAT4>> m_surfaceColorCB;
		std::shared_ptr<ConstantBuffer<DirectX::XMFLOAT4>> m_cameraPosCB;
		
		std::shared_ptr<PhongEffect> m_phongEffect;
		std::shared_ptr<LightShadowEffect> m_lightShadowEffect;
		std::shared_ptr<ParticleSystem> m_particles;
		std::shared_ptr<ID3D11InputLayout> m_layout;

		std::shared_ptr<ID3D11RasterizerState> m_rsCullNone;
		std::shared_ptr<ID3D11BlendState> m_bsAlpha;
		std::shared_ptr<ID3D11DepthStencilState> m_dssNoWrite;

		void InitializeConstantBuffers();
		void InitializeCamera();
		void InitializeRenderStates();
		void CreateScene();
		void UpdateCamera() const;

		void DrawScene();
	};
}

#endif __GK2_ROOM_H_
#ifndef __GK2_PARTICLES_H_
#define __GK2_PARTICLES_H_

#include <d3d11.h>
#include <DirectXMath.h>
#include <list>
#include <memory>
#include "gk2_deviceHelper.h"
#include "gk2_constantBuffer.h"
#include <random>

namespace gk2
{
	struct ParticleVertex
	{
		DirectX::XMFLOAT3 Pos;
		float Age;
		float Angle;
		float Size;
		static const unsigned int LayoutElements = 4;
		static const D3D11_INPUT_ELEMENT_DESC Layout[LayoutElements];

		ParticleVertex() : Pos(0.0f, 0.0f, 0.0f), Age(0.0f), Angle(0.0f), Size(0.0f) { }
	};
	
	struct ParticleVelocities
	{
		DirectX::XMFLOAT3 Velocity;
		float AngleVelocity;

		ParticleVelocities() : Velocity(0.0f, 0.0f, 0.0f), AngleVelocity(0.0f) { }
	};

	struct Particle
	{
		ParticleVertex Vertex;
		ParticleVelocities Velocities;
	};

	class ParticleComparer
	{
	public:
		ParticleComparer(DirectX::XMFLOAT4 camDir, DirectX::XMFLOAT4 camPos) : m_camDir(camDir), m_camPos(camPos) { }

		bool operator()(const ParticleVertex& p1, const ParticleVertex& p2) const;

	private:
		DirectX::XMFLOAT4 m_camDir, m_camPos;
	};

	class ParticleSystem
	{
	public:
		ParticleSystem(DeviceHelper& device, std::shared_ptr<ID3D11DeviceContext> context,
			DirectX::XMFLOAT3 emitterPos);

		void SetViewMtxBuffer(const std::shared_ptr<CBMatrix>& view);
		void SetProjMtxBuffer(const std::shared_ptr<CBMatrix>& proj);
		void SetSamplerState(const std::shared_ptr<ID3D11SamplerState>& samplerState);

		void Update(std::shared_ptr<ID3D11DeviceContext>& context, float dt, DirectX::XMFLOAT4 cameraPos);
		void Render(std::shared_ptr<ID3D11DeviceContext>& context) const;

	private:
		static const DirectX::XMFLOAT3 EMITTER_DIR;	//mean direction of particles' velocity
		static const float TIME_TO_LIVE;	//time of particle's life in seconds
		static const float EMISSION_RATE;	//number of particles to be born per second
		static const float MAX_ANGLE;		//maximal angle declination from mean direction
		static const float MIN_VELOCITY;	//minimal value of particle's velocity
		static const float MAX_VELOCITY;	//maximal value of particle's velocity
		static const float PARTICLE_SIZE;	//initial size of a particle
		static const float PARTICLE_SCALE;	//size += size*scale*dtime
		static const float MIN_ANGLE_VEL;	//minimal rotation speed
		static const float MAX_ANGLE_VEL;	//maximal rotation speed
		static const int MAX_PARTICLES;		//maximal number of particles in the system
		
		static const unsigned int OFFSET;
		static const unsigned int STRIDE;

		DirectX::XMFLOAT3 m_emitterPos;
		float m_particlesToCreate;
		unsigned int m_particlesCount;
		
		std::list<Particle> m_particles;

		std::shared_ptr<ID3D11Buffer> m_vertices;
		
		std::shared_ptr<CBMatrix> m_viewCB;
		std::shared_ptr<CBMatrix> m_projCB;
		
		std::shared_ptr<ID3D11SamplerState> m_samplerState;
		std::shared_ptr<ID3D11ShaderResourceView> m_cloudTexture;
		std::shared_ptr<ID3D11ShaderResourceView> m_opacityTexture;

		std::shared_ptr<ID3D11VertexShader> m_vs;
		std::shared_ptr<ID3D11GeometryShader> m_gs;
		std::shared_ptr<ID3D11PixelShader> m_ps;
		std::shared_ptr<ID3D11InputLayout> m_layout;

		std::random_device m_random;
		std::uniform_real_distribution<float> m_dirCoordDist;
		std::uniform_real_distribution<float> m_velDist;
		std::uniform_real_distribution<float> m_angleVelDist;

		DirectX::XMFLOAT3 RandomVelocity();
		void AddNewParticle();
		static void UpdateParticle(Particle& p, float dt);
		void UpdateVertexBuffer(std::shared_ptr<ID3D11DeviceContext>& context, DirectX::XMFLOAT4 cameraPos);
	};
}

#endif __GK2_PARTICLES_H_

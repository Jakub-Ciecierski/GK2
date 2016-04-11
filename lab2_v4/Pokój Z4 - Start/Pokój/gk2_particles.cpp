#include "gk2_particles.h"
#include <vector>
#include <algorithm>
#include "gk2_exceptions.h"

using namespace std;
using namespace gk2;
using namespace DirectX;

const D3D11_INPUT_ELEMENT_DESC ParticleVertex::Layout[ParticleVertex::LayoutElements] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 2, DXGI_FORMAT_R32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

bool ParticleComparer::operator()(const ParticleVertex& p1, const ParticleVertex& p2) const
{
	auto p1Pos = XMLoadFloat3(&(p1.Pos));
	p1Pos.m128_f32[3] = 1.0f;
	auto p2Pos = XMLoadFloat3(&(p2.Pos));
	p2Pos.m128_f32[3] = 1.0f;
	auto camDir = XMLoadFloat4(&m_camDir);
	auto camPos = XMLoadFloat4(&m_camPos);
	auto d1 = XMVector3Dot(p1Pos - camPos, camDir).m128_f32[0];
	auto d2 = XMVector3Dot(p2Pos - camPos, camDir).m128_f32[0];
	return d1 > d2;
}

const XMFLOAT3 ParticleSystem::EMITTER_DIR = XMFLOAT3(0.0f, 1.0f, 0.0f);
const float ParticleSystem::TIME_TO_LIVE = 4.0f;
const float ParticleSystem::EMISSION_RATE = 10.0f;
const float ParticleSystem::MAX_ANGLE = XM_PIDIV2 / 9.0f;
const float ParticleSystem::MIN_VELOCITY = 0.2f;
const float ParticleSystem::MAX_VELOCITY = 0.33f;
const float ParticleSystem::PARTICLE_SIZE = 0.08f;
const float ParticleSystem::PARTICLE_SCALE = 1.0f;
const float ParticleSystem::MIN_ANGLE_VEL = -XM_PI;
const float ParticleSystem::MAX_ANGLE_VEL = XM_PI;
const int ParticleSystem::MAX_PARTICLES = 500;

const unsigned int ParticleSystem::STRIDE = sizeof(ParticleVertex);
const unsigned int ParticleSystem::OFFSET = 0;

ParticleSystem::ParticleSystem(DeviceHelper& device, XMFLOAT3 emitterPos)
	: m_emitterPos(emitterPos), m_particlesToCreate(0.0f), m_particlesCount(0), m_dirCoordDist(-1.0f, 1.0f),
	  m_velDist(MIN_VELOCITY, MAX_VELOCITY), m_angleVelDist(MIN_ANGLE_VEL, MAX_ANGLE_VEL)
{
	m_vertices = device.CreateVertexBuffer<ParticleVertex>(MAX_PARTICLES, D3D11_USAGE_DYNAMIC);
	auto vsByteCode = device.LoadByteCode(L"particlesVS.cso");
	auto gsByteCode = device.LoadByteCode(L"particlesGS.cso");
	auto psByteCode = device.LoadByteCode(L"particlesPS.cso");
	m_vs = device.CreateVertexShader(vsByteCode);
	m_gs = device.CreateGeometryShader(gsByteCode);
	m_ps = device.CreatePixelShader(psByteCode);
	m_layout = device.CreateInputLayout<ParticleVertex>(vsByteCode);
	m_cloudTexture = device.CreateShaderResourceView(L"resources/textures/smoke.png");
	m_opacityTexture = device.CreateShaderResourceView(L"resources/textures/smokecolors.png");
	auto sd = device.DefaultSamplerDesc();
	sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	m_samplerState = device.CreateSamplerState(sd);
}

void ParticleSystem::SetViewMtxBuffer(const shared_ptr<CBMatrix>& view)
{
	if (view != nullptr)
		m_viewCB = view;
}

void ParticleSystem::SetProjMtxBuffer(const shared_ptr<CBMatrix>& proj)
{
	if (proj != nullptr)
		m_projCB = proj;
}

XMFLOAT3 ParticleSystem::RandomVelocity()
{
	float x, y;
	do 
	{
		x = m_dirCoordDist(m_random);
		y = m_dirCoordDist(m_random);
	} while (x*x + y*y > 1.0f);
	auto a = tan(MAX_ANGLE);
	XMFLOAT3 v(x * a, 1.0f, y * a);
	auto velocity = XMLoadFloat3(&v);
	auto len = m_velDist(m_random);
	velocity = len * XMVector3Normalize(velocity);
	XMStoreFloat3(&v, velocity);
	return v;
}

void ParticleSystem::AddNewParticle()
{
	Particle p;
	p.Vertex.Pos = m_emitterPos;
	p.Vertex.Age = 0.0f;
	p.Vertex.Angle = 0.0f;
	p.Vertex.Size = PARTICLE_SIZE;
	p.Velocities.Velocity = RandomVelocity();
	p.Velocities.AngleVelocity = m_angleVelDist(m_random);
	m_particles.push_back(p);
}

XMFLOAT3 operator *(const XMFLOAT3& v1, float d)
{
	return XMFLOAT3(v1.x * d , v1.y * d, v1.z * d);
}

XMFLOAT3 operator +(const XMFLOAT3& v1, const XMFLOAT3& v2)
{
	return XMFLOAT3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

XMFLOAT4 operator -(const XMFLOAT4& v1, const XMFLOAT4& v2)
{
	XMFLOAT4 res;
	res.x = v1.x - v2.x;
	res.y = v1.y - v2.y;
	res.z = v1.z - v2.z;
	res.w = v1.w - v2.w;
	return res;
}

void ParticleSystem::UpdateParticle(Particle& p, float dt)
{
	p.Vertex.Age += dt;
	p.Vertex.Pos = p.Vertex.Pos + p.Velocities.Velocity * dt;
	p.Vertex.Size += PARTICLE_SCALE * PARTICLE_SIZE * dt;
	p.Vertex.Angle += p.Velocities.AngleVelocity * dt;
}

void ParticleSystem::UpdateVertexBuffer(shared_ptr<ID3D11DeviceContext>& context, XMFLOAT4 cameraPos)
{
	vector<ParticleVertex> vertices(MAX_PARTICLES);
	auto vit = vertices.begin();
	for (auto lit = m_particles.begin(); lit != m_particles.end(); ++vit, ++lit)
		*vit = lit->Vertex;
	XMFLOAT4 cameraTarget(0.0f, 0.0f, 0.0f, 1.0f);
	sort(vertices.begin(), vit, ParticleComparer(cameraTarget - cameraPos, cameraPos));
	D3D11_MAPPED_SUBRESOURCE resource;
	auto hr = context->Map(m_vertices.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	if (FAILED(hr))
		THROW_WINAPI;
	memcpy(resource.pData, vertices.data(), MAX_PARTICLES * sizeof(ParticleVertex));
	context->Unmap(m_vertices.get(), 0);
}

void ParticleSystem::Update(shared_ptr<ID3D11DeviceContext>& context, float dt, XMFLOAT4 cameraPos)
{
	for (auto it = m_particles.begin(); it != m_particles.end(); )
	{
		UpdateParticle(*it, dt);
		auto prev = it++;
		if (prev->Vertex.Age >= TIME_TO_LIVE)
		{
			m_particles.erase(prev);
			--m_particlesCount;
		}
	}
	m_particlesToCreate += dt * EMISSION_RATE;
	while (m_particlesToCreate >= 1.0f)
	{
		--m_particlesToCreate;
		if (m_particlesCount < MAX_PARTICLES)
		{
			AddNewParticle();
			++m_particlesCount;
		}
	}
	UpdateVertexBuffer(context, cameraPos);
}

void ParticleSystem::Render(shared_ptr<ID3D11DeviceContext>& context) const
{
	context->VSSetShader(m_vs.get(), nullptr, 0);
	context->GSSetShader(m_gs.get(), nullptr, 0);
	context->PSSetShader(m_ps.get(), nullptr, 0);
	context->IASetInputLayout(m_layout.get());
	ID3D11Buffer* vsb[1] = { m_viewCB->getBufferObject().get() };
	context->VSSetConstantBuffers(0, 1, vsb);
	ID3D11Buffer* gsb[1] = { m_projCB->getBufferObject().get() };
	context->GSSetConstantBuffers(0, 1, gsb);
	ID3D11ShaderResourceView* psv[2] = { m_cloudTexture.get(), m_opacityTexture.get() };
	context->PSSetShaderResources(0, 2, psv);
	ID3D11SamplerState* pss[1] = { m_samplerState.get() };
	context->PSSetSamplers(0, 1, pss);
	ID3D11Buffer* vb[1] = { m_vertices.get() };
	context->IASetVertexBuffers(0, 1, vb, &STRIDE, &OFFSET);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	context->Draw(m_particlesCount, 0);
	context->GSSetShader(nullptr, nullptr, 0);
}

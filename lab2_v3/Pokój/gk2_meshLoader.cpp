#include "gk2_meshLoader.h"
#include <vector>
#include "gk2_vertices.h"
#include <DirectXMath.h>
#include <fstream>

using namespace std;
using namespace gk2;
using namespace DirectX;

Mesh MeshLoader::GetSphere(int stacks, int slices, float radius /* = 0.5f */)
{
	int n = (stacks - 1) * slices + 2;
	vector<VertexPosNormal> vertices(n);
	vertices[0].Pos = XMFLOAT3(0.0f, radius, 0.0f);
	vertices[0].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	auto dp = XM_PI / stacks;
	auto phi = dp;
	auto k = 1;
	for (auto i = 0; i < stacks - 1; ++i, phi += dp)
	{
		float cosp, sinp;
		XMScalarSinCos(&sinp, &cosp, phi);
		auto thau = 0.0f;
		auto dt = XM_2PI / slices;
		auto stackR = radius * sinp;
		auto stackY = radius * cosp;
		for (auto j = 0; j < slices; ++j, thau += dt)
		{
			float cost, sint;
			XMScalarSinCos(&sint, &cost, thau);
			vertices[k].Pos = XMFLOAT3(stackR * cost, stackY, stackR * sint);
			vertices[k++].Normal = XMFLOAT3(cost * sinp, cosp, sint * sinp);
		}
	}
	vertices[k].Pos = XMFLOAT3(0.0f, -radius, 0.0f);
	vertices[k].Normal = XMFLOAT3(0.0f, -1.0f, 0.0f);
	auto in = (stacks - 1) * slices * 6;
	vector<unsigned short> indices(in);
	k = 0;
	for (auto j = 0; j < slices - 1; ++j)
	{
		indices[k++] = 0;
		indices[k++] = j + 2;
		indices[k++] = j + 1;
	}
	indices[k++] = 0;
	indices[k++] = 1;
	indices[k++] = slices;
	auto i = 0;
	for (; i < stacks - 2; ++i)
	{
		auto j = 0;
		for (; j < slices - 1; ++j)
		{
			indices[k++] = i*slices + j + 1;
			indices[k++] = i*slices + j + 2;
			indices[k++] = (i + 1)*slices + j + 2;
			indices[k++] = i*slices + j + 1;
			indices[k++] = (i + 1)*slices + j + 2;
			indices[k++] = (i + 1)*slices + j + 1;
		}
		indices[k++] = i*slices + j + 1;
		indices[k++] = i*slices + 1;
		indices[k++] = (i + 1)*slices + 1;
		indices[k++] = i*slices + j + 1;
		indices[k++] = (i + 1)*slices + 1;
		indices[k++] = (i + 1)*slices + j + 1;
	}
	for (auto j = 0; j < slices - 1; ++j)
	{
		indices[k++] = i*slices + j + 1;
		indices[k++] = i*slices + j + 2;
		indices[k++] = n - 1;
	}
	indices[k++] = (i + 1)*slices;
	indices[k++] = i*slices + 1;
	indices[k] = n - 1;
	return Mesh(m_device.CreateVertexBuffer(vertices), sizeof(VertexPosNormal),
				m_device.CreateIndexBuffer(indices), in);
}

Mesh MeshLoader::GetCylinder(int stacks, int slices, float radius /* = 0.5f */, float height /* = 1.0f */)
{
	auto n = (stacks + 1) * slices;
	vector<VertexPosNormal> vertices(n);
	auto y = height / 2;
	auto dy = height / stacks;
	auto dp = XM_2PI / slices;
	auto k = 0;
	for (auto i = 0; i <= stacks; ++i, y -= dy)
	{
		auto phi = 0.0f;
		for (auto j = 0; j < slices; ++j, phi += dp)
		{
			float sinp, cosp;
			XMScalarSinCos(&sinp, &cosp, phi);
			vertices[k].Pos = XMFLOAT3(radius*cosp, y, radius*sinp);
			vertices[k++].Normal = XMFLOAT3(cosp, 0, sinp);
		}
	}
	auto in = 6 * stacks * slices;
	vector<unsigned short> indices(in);
	k = 0;
	for (auto i = 0; i < stacks; ++i)
	{
		auto j = 0;
		for (; j < slices - 1; ++j)
		{
			indices[k++] = i*slices + j;
			indices[k++] = i*slices + j + 1;
			indices[k++] = (i + 1)*slices + j + 1;
			indices[k++] = i*slices + j;
			indices[k++] = (i + 1)*slices + j + 1;
			indices[k++] = (i + 1)*slices + j;
		}
		indices[k++] = i*slices + j;
		indices[k++] = i*slices;
		indices[k++] = (i + 1)*slices;
		indices[k++] = i*slices + j;
		indices[k++] = (i + 1)*slices;
		indices[k++] = (i + 1)*slices + j;
	}
	return Mesh(m_device.CreateVertexBuffer(vertices), sizeof(VertexPosNormal),
				m_device.CreateIndexBuffer(indices), in);
}

Mesh MeshLoader::GetBox(float side /* = 1.0f */)
{
	side /= 2;
	VertexPosNormal vertices[] =
	{
		//Front face
		{ XMFLOAT3(-side, -side, -side), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(-side, side, -side), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(side, side, -side), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(side, -side, -side), XMFLOAT3(0.0f, 0.0f, -1.0f) },

		//Left face
		{ XMFLOAT3(-side, -side, -side), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-side, -side, side), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-side, side, side), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-side, side, -side), XMFLOAT3(-1.0f, 0.0f, 0.0f) },

		//Bottom face
		{ XMFLOAT3(-side, -side, -side), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(side, -side, -side), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(side, -side, side), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(-side, -side, side), XMFLOAT3(0.0f, -1.0f, 0.0f) },

		//Back face
		{ XMFLOAT3(-side, -side, side), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(side, -side, side), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(side, side, side), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-side, side, side), XMFLOAT3(0.0f, 0.0f, 1.0f) },

		//Right face
		{ XMFLOAT3(side, -side, -side), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(side, side, -side), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(side, side, side), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(side, -side, side), XMFLOAT3(1.0f, 0.0f, 0.0f) },

		//Top face
		{ XMFLOAT3(-side, side, -side), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(-side, side, side), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(side, side, side), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(side, side, -side), XMFLOAT3(0.0f, 1.0f, 0.0f) },
	};
	unsigned short indices[] =
	{
		0, 1, 2, 0, 2, 3,		//Front face
		4, 5, 6, 4, 6, 7,		//Left face
		8, 9, 10, 8, 10, 11,	//Botton face
		12, 13, 14, 12, 14, 15,	//Back face
		16, 17, 18, 16, 18, 19,	//Right face
		20, 21, 22, 20, 22, 23	//Top face
	};
	return Mesh(m_device.CreateVertexBuffer(vertices, 24), sizeof(VertexPosNormal),
				m_device.CreateIndexBuffer(indices, 36), 36);
}

Mesh MeshLoader::GetQuad(float side /* = 1.0f */)
{
	side /= 2;
	VertexPosNormal vertices[] =
	{
		{ XMFLOAT3(-side, -side, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(-side, side, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(side, side, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(side, -side, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) }
	};
	unsigned short indices[] = { 0, 1, 2, 0, 2, 3 };
	return Mesh(m_device.CreateVertexBuffer(vertices, 4), sizeof(VertexPosNormal),
				m_device.CreateIndexBuffer(indices, 6), 6);
}

Mesh MeshLoader::GetDisc(int slices, float radius /* = 0.5f */)
{
	auto n = slices + 1;
	vector<VertexPosNormal> vertices(n);
	vertices[0].Pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertices[0].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	auto phi = 0.0f;
	auto dp = XM_2PI / slices;
	auto k = 1;
	for (auto i = 1; i <= slices; ++i, phi += dp)
	{
		float cosp, sinp;
		XMScalarSinCos(&sinp, &cosp, phi);
		vertices[k].Pos = XMFLOAT3(radius * cosp, 0.0f, radius * sinp);
		vertices[k++].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	}
	auto in = slices * 3;
	vector<unsigned short> indices(in);
	k = 0;
	for (auto i = 0; i < slices - 1; ++i)
	{
		indices[k++] = 0;
		indices[k++] = i + 2;
		indices[k++] = i + 1;
	}
	indices[k++] = 0;
	indices[k++] = 1;
	indices[k] = slices;
	return Mesh(m_device.CreateVertexBuffer(vertices), sizeof(VertexPosNormal),
				m_device.CreateIndexBuffer(indices), in);
}

Mesh MeshLoader::LoadMesh(const wstring& fileName)
{
	ifstream input;
	input.exceptions(ios::badbit | ios::failbit | ios::eofbit); //Most of the time you really shouldn't throw
																//exceptions in case of eof, but here if end of file was
																//reached before the whole mesh was loaded, we would
																//have had to throw an exception anyway.
	int n, in;
	input.open(fileName);
	input >> n >> in;
	vector<VertexPosNormal> vertices(n);
	XMFLOAT2 texDummy;
	for (auto i = 0; i < n; ++ i)
	{
		input >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		input >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
		input >> texDummy.x >> texDummy.y;
	}
	vector<unsigned short> indices(in);
	for (auto i = 0; i < in; ++i)
		input >> indices[i];
	input.close();
	return Mesh(m_device.CreateVertexBuffer(vertices), sizeof(VertexPosNormal),
				m_device.CreateIndexBuffer(indices), in);
}

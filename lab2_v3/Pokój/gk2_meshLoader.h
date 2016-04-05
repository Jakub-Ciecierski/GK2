#ifndef __GK2_MESH_LOADER_H_
#define __GK2_MESH_LOADER_H_

#include "gk2_deviceHelper.h"
#include "gk2_mesh.h"
#include <string>

namespace gk2
{
	class MeshLoader
	{
	public:
		explicit MeshLoader(const DeviceHelper& device): m_device(device) { }

		Mesh GetSphere(int stacks, int slices, float radius = 0.5f);
		Mesh GetCylinder(int stacks, int slices, float radius = 0.5f, float height = 1.0f);
		Mesh GetDisc(int slices, float radius = 0.5f);
		Mesh GetBox(float side = 1.0f);
		Mesh GetQuad(float side = 1.0f);
		Mesh LoadMesh(const std::wstring& fileName);

	private:
		DeviceHelper m_device;
	};
}

#endif __GK2_MESH_LOADER_H_

#include "gk2_utils.h"

using namespace gk2;

void Utils::COMRelease(IUnknown* comObject)
{
	if (comObject != nullptr)
		comObject->Release();
}

void Utils::DI8DeviceRelease(IDirectInputDevice8W* device)
{
	if (device != nullptr)
	{
		device->Unacquire();
		device->Release();
	}
}

void* Utils::New16Aligned(size_t size)
{
	BYTE* ptr = new BYTE[size + 16];
	BYTE* shifted = ptr + 16;
	BYTE missalignment = (BYTE)((ULONG)shifted & 0xf);
	shifted = (BYTE*)((ULONG)shifted &~(ULONG)0xf);
	shifted[-1] = missalignment;
	return (void*)shifted;
}

void Utils::Delete16Aligned(void* ptr)
{
	BYTE* shifted = (BYTE*)ptr;
	BYTE missalignment = shifted[-1];
	shifted = (BYTE*)((ULONG)shifted | (ULONG)missalignment);
	BYTE* original = shifted - 16;
	delete [] original;
}
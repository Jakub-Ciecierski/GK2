#include "gk2_input.h"

using namespace std;
using namespace gk2;

bool Keyboard::GetState(KeyboardState& state) const
{
	return DeviceBase::GetState(KeyboardState::STATE_TAB_LENGTH*sizeof(BYTE), reinterpret_cast<void*>(&state.m_keys));
}

bool Mouse::GetState(MouseState& state) const
{
	return DeviceBase::GetState(sizeof(DIMOUSESTATE), reinterpret_cast<void*>(&state.m_state));
}

unique_ptr_del<IDirectInputDevice8W> InputHelper::CreateInputDevice(HWND hWnd, const GUID& deviceGuid,
	const DIDATAFORMAT& dataFormat) const
{
	IDirectInputDevice8W* d;
	HRESULT result = m_inputObject->CreateDevice(deviceGuid, &d, nullptr);
	unique_ptr_del<IDirectInputDevice8W> device(d, Utils::DI8DeviceRelease);
	if (FAILED(result))
		THROW_DX11(result);
	result = device->SetDataFormat(&dataFormat);
	if (FAILED(result))
		THROW_DX11(result);
	result = device->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(result))
		THROW_DX11(result);
	return device;
}

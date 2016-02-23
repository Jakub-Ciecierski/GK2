#include "gk2_input.h"

using namespace std;
using namespace gk2;

bool Keyboard::GetState(KeyboardState& state)
{
	return DeviceBase::GetState(KeyboardState::STATE_TAB_LENGTH*sizeof(BYTE), reinterpret_cast<void*>(&state.m_keys));
}

bool Mouse::GetState(MouseState& state)
{
	return DeviceBase::GetState(sizeof(DIMOUSESTATE), reinterpret_cast<void*>(&state.m_state));
}

shared_ptr<IDirectInputDevice8W> InputHelper::CreateInputDevice(HWND hWnd, const GUID& deviceGuid,
	const DIDATAFORMAT& dataFormat)
{
	IDirectInputDevice8W* d;
	HRESULT result = m_inputObject->CreateDevice(deviceGuid, &d, 0);
	shared_ptr<IDirectInputDevice8W> device(d, Utils::DI8DeviceRelease);
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
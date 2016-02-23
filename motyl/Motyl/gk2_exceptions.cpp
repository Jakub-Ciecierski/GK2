#include "gk2_exceptions.h"

using namespace std;
using namespace gk2;

WinAPIException::WinAPIException(const wchar_t* location, DWORD errorCode)
	: Exception(location), m_code(errorCode)
{

}

wstring WinAPIException::getMessage() const
{
	wstring message;
	try
	{
		LPWSTR lpMsgBuf;
		if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, m_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &lpMsgBuf, 0, NULL))
		{
			message = lpMsgBuf;
			LocalFree(lpMsgBuf);
		}
		message += wstring(L"\nLocation: ") + getErrorLocation();
	}
	catch(...)
	{	}
	return message;
}

Dx11Exception::Dx11Exception(const wchar_t* location, HRESULT result)
	: Exception(location), m_result(result)
{

}

wstring Dx11Exception::getMessage() const
{
	try
	{
		return wstring(DXGetErrorDescriptionW(m_result)) + L"\nLocation: " + getErrorLocation();
	}
	catch (...)
	{
		return wstring();
	}
}
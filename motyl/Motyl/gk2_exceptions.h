#ifndef __GK2_EXCEPTIONS_H_
#define __GK2_EXCEPTIONS_H_

#include <Windows.h>
#include <string>
#include <DxErr.h>
#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)

#define STRINGIFY(x) #x
#define TOWSTRING(x) WIDEN(STRINGIFY(x))
#define __AT__ __WFILE__ L":" TOWSTRING(__LINE__)

namespace gk2
{
	class Exception
	{
	public:
		Exception(const wchar_t* location) { m_location = location; }
		virtual std::wstring getMessage() const = 0;
		virtual int getExitCode() const = 0;
		const wchar_t* getErrorLocation() const { return m_location; }
	private:
		const wchar_t* m_location;
	};

	class WinAPIException : public gk2::Exception
	{
	public:
		WinAPIException(const wchar_t* location, DWORD errorCode = GetLastError());
		virtual int getExitCode() const { return getErrorCode(); }
		inline DWORD getErrorCode() const { return m_code; }
		virtual std::wstring getMessage() const;

	private:
		DWORD m_code;
	};

	class Dx11Exception : public gk2::Exception
	{
	public:
		Dx11Exception(const wchar_t* location, HRESULT result);
		virtual int getExitCode() const { return getResultCode(); }
		inline HRESULT getResultCode() const { return m_result; }
		virtual std::wstring getMessage() const;

	private:
		HRESULT m_result;
	};
}

#define THROW_WINAPI throw gk2::WinAPIException(__AT__)
#define THROW_DX11(hr) throw gk2::Dx11Exception(__AT__, hr)

#endif __GK2_EXCEPTIONS_H_

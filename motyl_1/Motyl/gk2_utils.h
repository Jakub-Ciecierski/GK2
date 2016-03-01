#ifndef __GK2_UTILS_H_
#define __GK2_UTILS_H_
#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <d3d11.h>
#include <dinput.h>
#include <memory>
#include <functional>

namespace gk2
{
	class Utils
	{
	public:
		static void COMRelease(IUnknown* comObject);
		static void DI8DeviceRelease(IDirectInputDevice8W* device);

		template<typename T>
		static void DeleteArray(T* arrayPtr)
		{
			if (arrayPtr != nullptr)
				delete [] arrayPtr;
		}

		//Alokowanie pamiêci wyrównanej do 16 bajtów.
		//Pamiêæ tak zaalokowan¹ nale¿y zwolnic za pomoc¹ metody Utils::Delete16Aligned.
		static void* New16Aligned(size_t size);
		static void Delete16Aligned(void* ptr);
	};

	template<class T>
	using unique_ptr_del = std::unique_ptr<T, std::function<void(T*)>>;
}

#endif __GK2_UTILS_H_

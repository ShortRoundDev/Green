#include "LocalShaderIncluder.h"

#include <Windows.h>

LocalShaderIncluder::LocalShaderIncluder()
{

}

LocalShaderIncluder::~LocalShaderIncluder()
{

}

HRESULT LocalShaderIncluder::Open(
	D3D_INCLUDE_TYPE	IncludeType,
	LPCSTR				pFileName,
	LPCVOID				pParentData,
	LPCVOID* ppData,
	UINT* pBytes
)
{
	auto handle = CreateFileA(
		pFileName,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (handle == INVALID_HANDLE_VALUE)
	{
		return E_FAIL;
	}
	DWORD size = GetFileSize(handle, NULL);
	*ppData = (LPCVOID)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
	DWORD read;
	auto res = ReadFile(handle, (LPVOID)*ppData, size, &read, NULL);
	if (!res)
	{
		CloseHandle(handle);
		return E_FAIL;
	}
	CloseHandle(handle);
	*pBytes = size;
	return S_OK;
}

HRESULT LocalShaderIncluder::Close(
	LPCVOID pData
)
{
	HeapFree(GetProcessHeap(), 0, (LPVOID)pData);
	return S_OK;
}
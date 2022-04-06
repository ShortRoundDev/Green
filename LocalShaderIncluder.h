#pragma once

#include <d3dcommon.h>

class LocalShaderIncluder : public ID3DInclude
{
public:
	LocalShaderIncluder();
	~LocalShaderIncluder();

	HRESULT Open(
		D3D_INCLUDE_TYPE	IncludeType,
		LPCSTR				pFileName,
		LPCVOID				pParentData,
		LPCVOID* ppData,
		UINT* pBytes
	);

	HRESULT Close(
		LPCVOID pData
	);
};


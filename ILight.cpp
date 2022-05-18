#include "ILight.h"

XMFLOAT4 ILight::getPos()
{
    return m_pos;
}

ID3D11ShaderResourceView* ILight::getDepthMapSrv()
{
    return m_depthMapSrv.Get();
}

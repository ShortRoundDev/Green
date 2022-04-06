#include "Texture.h"

#include "GraphicsManager.h"

#include <DirectXTK/WICTextureLoader.h>

#include <codecvt>

using namespace DirectX;

Texture::Texture(std::string path)
{
    std::wstring winPath = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(path);
    
    HRESULT result = CreateWICTextureFromFile(
        Graphics.getDevice(),
        winPath.c_str(),
        m_resource.GetAddressOf(),
        m_view.GetAddressOf()
    );

    if (FAILED(result))
    {
        m_status = false;
        return;
    }

    result = m_resource->QueryInterface(IID_ID3D11Texture2D, (void**)m_texture.GetAddressOf());
    if (FAILED(result))
    {
        m_status = false;
        return;
    }
    m_status = true;
}

Texture::~Texture()
{

}

void Texture::use()
{
    if (m_view == NULL)
    {
        return;
    }
    
    Graphics.getContext()->PSSetShaderResources(0, 1, m_view.GetAddressOf());
}
#pragma once

#include <d3d11.h>

#include <wrl/client.h>

#include <string>

using namespace Microsoft::WRL;



class Texture
{
public:
    Texture(std::string path);
    ~Texture();

    void use();
private:
    bool m_status;

    ComPtr<ID3D11Resource> m_resource;
    ComPtr<ID3D11Texture2D> m_texture;
    ComPtr<ID3D11ShaderResourceView> m_view;
};


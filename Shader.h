#pragma once

#include "GTypes.h"

#include <d3d11.h>
#include <wrl/client.h>
#include "DirectXMath.h"

#include <string>

using namespace Microsoft::WRL;
using namespace DirectX;

class PointLight;

class Shader
{
public:
    Shader(
        ID3D11Device* device,
        std::wstring vertexPath,
        std::wstring pixelPath,
        size_t bufferSize
    );

    ~Shader();

    bool getStatus();
    bool bindCBuffer(void* cBuffer);
    bool bindPointLight(PointLight* pointLight);

    bool bindModelMatrix(const XMMATRIX& modelTransform);
    void use();
private:

    bool m_status;
    size_t m_bufferSize;

    ComPtr<ID3D11VertexShader> m_vertex;
    ComPtr<ID3D11PixelShader> m_pixel;
    ComPtr<ID3D11InputLayout> m_inputLayout;
    ComPtr<ID3D11Buffer> m_matrixBuffer;
    ComPtr<ID3D11SamplerState> m_samplerState;
    ComPtr<ID3D11SamplerState> m_pointShadowSampler;
    ComPtr<ID3D11SamplerState> m_spotShadowSampler;
    ComPtr<ID3D11BlendState> m_blendState;
    ComPtr<ID3D11Buffer> m_cBuffer;
    ComPtr<ID3D11Buffer> m_modelBuffer;

    bool initShaderCode(
        ID3D11Device* device,
        std::wstring vertexPath,
        std::wstring pixelPath,
        u8** vertexByteCode,
        size_t* vertexByteCodeLength
    );
    bool initLayout(ID3D11Device* device, u8* byteCode, size_t byteCodeLength);
    bool initSampler(ID3D11Device* device);
    bool initBlendState(ID3D11Device* device);
    bool initCBuffer(ID3D11Device* device, size_t bufferSize);

    void logShaderError(ID3D10Blob* shaderError, std::wstring path);
};


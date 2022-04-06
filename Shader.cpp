#include "Shader.h"

#include "GraphicsManager.h"
#include "SystemManager.h"
#include "GTypes.h"
#include "LocalShaderIncluder.h"
#include "Logger.h"

#include <d3dcompiler.h>

////////// PUBLIC //////////

static Logger logger = CreateLogger("Shader");

static LocalShaderIncluder g_includer;

Shader::Shader(
    ID3D11Device* device,
    std::wstring vertexPath,
    std::wstring pixelPath
)
{
    u8* byteCode;
    size_t byteCodeLength;
    m_status = initShaderCode(device, vertexPath, pixelPath, &byteCode, &byteCodeLength);
    if (!m_status)
    {
        return;
    }

    m_status = initLayout(device, byteCode, byteCodeLength);
    if (!m_status)
    {
        return;
    }

    m_status = initSampler(device);
    if (!m_status)
    {
        return;
    }

    m_status = initBlendState(device);
    if(!m_status)
    {
        return;
    }
}

Shader::~Shader()
{

}

bool Shader::getStatus()
{
    return m_status;
}

void Shader::use()
{
    auto context = Graphics.getContext();
    context->IASetInputLayout(m_inputLayout.Get());
    context->VSSetShader(m_vertex.Get(), NULL, 0);
    context->PSSetShader(m_pixel.Get(), NULL, 0);
    context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
    float factor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    context->OMSetBlendState(m_blendState.Get(), factor, 0xffffffff);
}

////////// PRIVATE //////////

bool Shader::initShaderCode(
    ID3D11Device* device,
    std::wstring vertexPath,
    std::wstring pixelPath,
    u8** vertexByteCode,
    size_t* vertexByteCodeLength
)
{
    m_status = false;

    if (!System.readFile(vertexPath.c_str(), vertexByteCode, 0, vertexByteCodeLength))
    {
        logger.err("Could not open Vertex Shader %s! Got 0x%x", vertexPath.c_str(), GetLastError());
        return false;
    }

    HRESULT result;
    result = device->CreateVertexShader(
        *vertexByteCode,
        *vertexByteCodeLength,
        nullptr,
        m_vertex.GetAddressOf()
    );

    //HeapFree(GetProcessHeap(), NULL, buffer);

    if (result != S_OK)
    {
        logger.err(L"Failed to load Vertex shader %s! Got 0x%x", vertexPath.c_str(), result);
        logger.err("See https://docs.microsoft.com/en-us/windows/win32/direct3d11/d3d11-graphics-reference-returnvalues");
        HeapFree(GetProcessHeap(), NULL, *vertexByteCode);
        return false;
    }

    u8* buffer;
    size_t size = 0;

    if (!System.readFile(pixelPath.c_str(), &buffer, 0, &size))
    {
        logger.err("Could not open Pixel Shader %s! Got 0x%x", vertexPath.c_str(), GetLastError());
        HeapFree(GetProcessHeap(), NULL, *vertexByteCode);
        return false;
    }

    result = device->CreatePixelShader(
        buffer,
        size,
        nullptr,
        m_pixel.GetAddressOf()
    );

    HeapFree(GetProcessHeap(), NULL, buffer);
    buffer = NULL;

    if (result != S_OK)
    {
        logger.err(L"Failed to load Pixel shader %s! Got 0x%x", pixelPath.c_str(), result);
        logger.err("See https://docs.microsoft.com/en-us/windows/win32/direct3d11/d3d11-graphics-reference-returnvalues");
        HeapFree(GetProcessHeap(), NULL, *vertexByteCode);
        return false;
    }
    return true;
}

bool Shader::initLayout(ID3D11Device* device, u8* byteCode, size_t byteCodeLength)
{
    D3D11_INPUT_ELEMENT_DESC polygonLayout[3];
    ZeroMemory(polygonLayout, sizeof(D3D11_INPUT_ELEMENT_DESC) * 3);
    polygonLayout[0].SemanticName = "POSITION";
    polygonLayout[0].SemanticIndex = 0;
    polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[0].InputSlot = 0;
    polygonLayout[0].AlignedByteOffset = 0;
    polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[0].InstanceDataStepRate = 0;

    polygonLayout[1].SemanticName = "NORMAL";
    polygonLayout[1].SemanticIndex = 0;
    polygonLayout[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[1].InputSlot = 0;
    polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[1].InstanceDataStepRate = 0;

    polygonLayout[2].SemanticName = "TEXCOORD";
    polygonLayout[2].SemanticIndex = 0;
    polygonLayout[2].Format = DXGI_FORMAT_R32G32_FLOAT;
    polygonLayout[2].InputSlot = 0;
    polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[2].InstanceDataStepRate = 0;

    int numElements = _countof(polygonLayout);

    HRESULT result = device->CreateInputLayout(
        polygonLayout,
        numElements,
        byteCode,
        byteCodeLength,
        m_inputLayout.GetAddressOf()
    );

    HeapFree(GetProcessHeap(), NULL, byteCode);
    if (FAILED(result))
    {
        logger.err("Shader input layout error!");
        return false;
    }
    return true;
}

bool Shader::initSampler(ID3D11Device* device)
{
    D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    HRESULT result = device->CreateSamplerState(&samplerDesc, m_samplerState.GetAddressOf());
    if (FAILED(result))
    {
        logger.err("Shader sampler creation error!");
        return false;
    }

    return true;
}

bool Shader::initBlendState(ID3D11Device* device)
{
    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;
    blendDesc.RenderTarget[0].BlendEnable = true;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    HRESULT result = device->CreateBlendState(&blendDesc, m_blendState.GetAddressOf());
    if (FAILED(result))
    {
        logger.err("Shader blend state creation error!");
        return false;
    }

    return true;
}

void Shader::logShaderError(ID3D10Blob* shaderError, std::wstring path)
{
    char* compileErrors;
    u64 bufferSize;

    compileErrors = (char*)(shaderError->GetBufferPointer());
    bufferSize = shaderError->GetBufferSize();

    std::string errorMsg = std::string(compileErrors, bufferSize);
    std::string spath = std::string(path.begin(), path.end());

    logger.err("Error compiling [%s]:\n%s", spath.c_str(), errorMsg.c_str());
}

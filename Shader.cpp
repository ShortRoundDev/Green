#include "Shader.h"

#include "GraphicsManager.h"
#include "SystemManager.h"
#include "LocalShaderIncluder.h"
#include "Logger.h"
#include "PointLight.h"

#include <d3dcompiler.h>
#include <codecvt>
#include <cmath>

////////// PUBLIC //////////

static Logger logger = CreateLogger("Shader");

static LocalShaderIncluder g_includer;

Shader::Shader(
    ID3D11Device* device,
    std::wstring vertexPath,
    std::wstring pixelPath,
    sz bufferSize
):
    m_bufferSize(bufferSize)
{
    u8* byteCode;
    sz byteCodeLength;
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
    
    m_status = initCBuffer(device, bufferSize);
    if (!m_status)
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
    context->PSSetSamplers(1, 1, m_pointShadowSampler.GetAddressOf());
    context->PSSetSamplers(2, 1, m_spotShadowSampler.GetAddressOf());

    float factor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    context->OMSetBlendState(m_blendState.Get(), factor, 0xffffffff);
}

////////// PRIVATE //////////

bool Shader::initShaderCode(
    ID3D11Device* device,
    std::wstring vertexPath,
    std::wstring pixelPath,
    u8** vertexByteCode,
    sz* vertexByteCodeLength
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
    sz size = 0;

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

bool Shader::initLayout(ID3D11Device* device, u8* byteCode, sz byteCodeLength)
{
    const i32 INPUT_SIZE = 5;
    D3D11_INPUT_ELEMENT_DESC polygonLayout[INPUT_SIZE];
    ZeroMemory(polygonLayout, sizeof(D3D11_INPUT_ELEMENT_DESC) * INPUT_SIZE);
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

    polygonLayout[3].SemanticName = "BONEINDICES";
    polygonLayout[3].SemanticIndex = 0;
    polygonLayout[3].Format = DXGI_FORMAT_R32G32B32A32_SINT;
    polygonLayout[3].InputSlot = 0;
    polygonLayout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[3].InstanceDataStepRate = 0;

    polygonLayout[4].SemanticName = "WEIGHTS";
    polygonLayout[4].SemanticIndex = 0;
    polygonLayout[4].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    polygonLayout[4].InputSlot = 0;
    polygonLayout[4].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[4].InstanceDataStepRate = 0;

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
        Graphics.printAndClearDebug(&logger);
        return false;
    }
    return true;
}

bool Shader::initSampler(ID3D11Device* device)
{
    D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
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

    ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
    samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    result = device->CreateSamplerState(&samplerDesc, m_pointShadowSampler.GetAddressOf());
    if (FAILED(result))
    {
        logger.err("Point shadow sampler creation error!");
        return false;
    }

    ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
    samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_GREATER;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    result = device->CreateSamplerState(&samplerDesc, m_spotShadowSampler.GetAddressOf());
    if (FAILED(result))
    {
        logger.err("Spot shadow sampler creation error!");
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

bool Shader::initCBuffer(ID3D11Device* device, sz bufferSize)
{
    D3D11_BUFFER_DESC matrixBufferDesc;
    ZeroMemory(&matrixBufferDesc, sizeof(D3D11_BUFFER_DESC));
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = (u32)(std::ceil((float)bufferSize/16.0f) * 16.0f);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    HRESULT result = Graphics.getDevice()->CreateBuffer(&matrixBufferDesc, NULL, m_cBuffer.GetAddressOf());
    if (FAILED(result))
    {
        return false;
    }
    // model buffer
    ZeroMemory(&matrixBufferDesc, sizeof(D3D11_BUFFER_DESC));
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = (u32)(std::ceil((float)sizeof(ModelBuffer) / 16.0f) * 16.0f);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    result = Graphics.getDevice()->CreateBuffer(&matrixBufferDesc, NULL, m_modelBuffer.GetAddressOf());
    if (FAILED(result))
    {
        return false;
    }


    return true;
}

bool Shader::bindModelMatrix(const XMMATRIX& modelTransform)
{
    return bindModelMatrix(modelTransform, nullptr, 0);
}

bool Shader::bindModelMatrix(const XMMATRIX& modelTransform, const std::vector<XMMATRIX>* bones, u32 totalBones)
{
    ModelBuffer model = { };

    model.modelTransform = XMMatrixTranspose(modelTransform);
    for(u32 i = 0; i < totalBones; i++){
        auto& matrix = (*bones)[i];
        model.bones[i] = XMMatrixTranspose(matrix);
    }
    model.totalBones = totalBones;

    D3D11_MAPPED_SUBRESOURCE mBufferResource;
    HRESULT result = Graphics.getContext()->Map(m_modelBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mBufferResource);
    if (FAILED(result))
    {
        logger.err("Failed to lock model buffer! Got 0x%x", result);
        return false;
    }

    void* localBuffer = (void*)mBufferResource.pData;
    CopyMemory(localBuffer, &model, sizeof(ModelBuffer));
    Graphics.getContext()->Unmap(m_modelBuffer.Get(), 0);
    ////////// END CRITICAL SECTION //////////

    // Put CBuffer in correct register slot
    Graphics.getContext()->VSSetConstantBuffers(1, 1, m_modelBuffer.GetAddressOf());
    Graphics.getContext()->PSSetConstantBuffers(1, 1, m_modelBuffer.GetAddressOf());
    return true;
}

bool Shader::bindCBuffer(void* cBuffer)
{
    D3D11_MAPPED_SUBRESOURCE gBufferResource;
    ////////// CRITICAL SECTION //////////
    HRESULT result = Graphics.getContext()->Map(m_cBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &gBufferResource);
    if (FAILED(result))
    {
        logger.err("Failed to lock global buffer! Got 0x%x", result);
        return false;
    }

    void* localBuffer = (void*)gBufferResource.pData;
    CopyMemory(localBuffer, cBuffer, m_bufferSize);
    Graphics.getContext()->Unmap(m_cBuffer.Get(), 0);
    ////////// END CRITICAL SECTION //////////

    // Put CBuffer in correct register slot
    Graphics.getContext()->VSSetConstantBuffers(2, 1, m_cBuffer.GetAddressOf());
    Graphics.getContext()->PSSetConstantBuffers(2, 1, m_cBuffer.GetAddressOf());

    return true;
}

void Shader::logShaderError(ID3D10Blob* shaderError, std::wstring path)
{
    char* compileErrors;
    u64 bufferSize;

    compileErrors = (char*)(shaderError->GetBufferPointer());
    bufferSize = shaderError->GetBufferSize();

    std::string errorMsg = std::string(compileErrors, bufferSize);
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

    std::string spath = converter.to_bytes(path);

    logger.err("Error compiling [%s]:\n%s", spath.c_str(), errorMsg.c_str());
}

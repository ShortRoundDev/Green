#include "MeshViewModel.h"

#include "GraphicsManager.h"

#include "Mesh.h"
#include "ILight.h"
#include "SpotLight.h"
#include "PointLight.h"
#include "Shader.h"

#include "Logger.h"

static Logger logger = CreateLogger("MeshViewModel");

MeshViewModel::MeshViewModel(
    Mesh* mesh,
    std::vector<SpotLight*> spotLights,
    std::vector<PointLight*> pointLights
) :
    m_mesh(mesh),
    m_numSpotLights(std::min((int)spotLights.size(), 3)),
    m_numPointLights(std::min((int)pointLights.size(), 3)),
    m_shader(Graphics.getShader(L"World"))
{
    for (int i = 0; i < m_numPointLights; i++)
    {
        m_pointLights[i] = pointLights[i];
    }

    for (int i = 0; i < m_numSpotLights; i++)
    {
        m_spotLights[i] = spotLights[i];
    }
    m_pointLightBuffer = new ID3D11ShaderResourceView*[m_numPointLights];
    m_spotLightBuffer = new ID3D11ShaderResourceView*[m_numSpotLights];
}

MeshViewModel::~MeshViewModel()
{

}

void MeshViewModel::draw()
{
    MeshLightBuffer lightBuffer;

    ZeroMemory(&lightBuffer, sizeof(lightBuffer));
    lightBuffer.nPointLights = m_numPointLights;
    for (int i = 0; i < m_numPointLights; i++)
    {
        //m_lights[i]->use(i + 1);
        m_pointLightBuffer[i] = m_pointLights[i]->getDepthMapSrv();
        auto buffer = m_pointLights[i]->getCBuffer();
        lightBuffer.pointLights[i] = buffer;
    }
    Graphics.getContext()->PSSetShaderResources(1, m_numPointLights, m_pointLightBuffer);

    lightBuffer.nSpotLights = m_numSpotLights;
    for (int i = 0; i < m_numSpotLights; i++)
    {
        m_spotLightBuffer[i] = m_spotLights[i]->getDepthMapSrv();
        SpotLightBuffer buffer = m_spotLights[i]->getCBuffer();
        lightBuffer.spotLights[i] = buffer;
    }
    Graphics.getContext()->PSSetShaderResources(17, m_numSpotLights, m_spotLightBuffer);

    m_shader->bindModelMatrix(XMMatrixIdentity());
    m_shader->bindCBuffer(&lightBuffer);
    m_mesh->draw();
}
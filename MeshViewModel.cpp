#include "MeshViewModel.h"

#include "GraphicsManager.h"

#include "Mesh.h"
#include "ILight.h"
#include "Shader.h"

MeshViewModel::MeshViewModel(Mesh* mesh, std::vector<ILight*> lights) :
    m_mesh(mesh),
    m_numLights(std::min((int)lights.size(), 3)),
    m_shader(Graphics.getShader(L"World"))
{
    for (int i = 0; i < m_numLights; i++)
    {
        m_lights[i] = lights[i];
    }
}

MeshViewModel::~MeshViewModel()
{

}

void MeshViewModel::draw()
{
    MeshLightBuffer lightBuffer;
    lightBuffer.nPointLights = m_numLights;

    if (m_numLights)
    {
        ID3D11ShaderResourceView** _lights = new ID3D11ShaderResourceView*[m_numLights];

        for (int i = 0; i < m_numLights; i++)
        {
            //m_lights[i]->use(i + 1);
            _lights[i] = m_lights[i]->getDepthMapSrv();
            auto buffer = ((PointLight*)m_lights[i])->m_cBuffer;
            lightBuffer.pointLights[i] = buffer;
        }
        Graphics.getContext()->PSSetShaderResources(1, m_numLights, _lights);
        m_shader->bindCBuffer(&lightBuffer);
    }

    m_mesh->draw();
}
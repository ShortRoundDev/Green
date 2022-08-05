#include "MeshViewModelBuilder.h"

#include "Mesh.h"
#include "DirectXMath.h"
#include "ILight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Logger.h"
#include "MeshViewModel.h"

#include <algorithm>

static Logger logger = CreateLogger("MeshViewModelBuilder");

MeshViewModelBuilder::MeshViewModelBuilder(Mesh* mesh) :
    m_mesh(mesh),
    m_pointLights(),
    m_spotLights()
{

}

const std::vector<SpotLight*>& MeshViewModelBuilder::getSpotLights()
{
    return m_spotLights;
}

const std::vector<PointLight*>& MeshViewModelBuilder::getPointLights()
{
    return m_pointLights;
}

void MeshViewModelBuilder::addSpotLight(SpotLight* spotLight)
{
    m_spotLights.push_back(spotLight);
}

void MeshViewModelBuilder::addPointLight(PointLight* pointLight)
{
    m_pointLights.push_back(pointLight);
}

MeshViewModel* MeshViewModelBuilder::build() {
    auto centroid = m_mesh->getCentroid();
    auto centroidV = XMLoadFloat3(&centroid);

    if (m_spotLights.size() > MAX_LIGHT)
    {
        std::sort(m_spotLights.begin(), m_spotLights.end(), [centroidV](ILight* a, ILight* b) {
            auto aPos = a->getPos();
            XMVECTOR aV = XMLoadFloat4(&aPos);

            auto bPos = b->getPos();
            XMVECTOR bV = XMLoadFloat4(&bPos);

            XMVECTOR aDiff = XMVectorSubtract(centroidV, aV);
            XMVECTOR bDiff = XMVectorSubtract(centroidV, bV);

            auto aLengthV = XMVector3LengthSq(aDiff);
            auto bLengthV = XMVector3LengthSq(bDiff);

            f32 aLength, bLength;
            XMStoreFloat(&aLength, aLengthV);
            XMStoreFloat(&bLength, bLengthV);

            return aLength > bLength;
        });
        if (m_spotLights.size() > MAX_LIGHT)
        {
            logger.warn("Spot Light limit threshold exceeded!");
        }
        m_spotLights.erase(m_spotLights.begin() + 3, m_spotLights.end());
    }

    if (m_pointLights.size() > MAX_LIGHT)
    {
        std::sort(m_pointLights.begin(), m_pointLights.end(), [centroidV](ILight* a, ILight* b) {
            auto aPos = a->getPos();
            XMVECTOR aV = XMLoadFloat4(&aPos);

            auto bPos = b->getPos();
            XMVECTOR bV = XMLoadFloat4(&bPos);

            XMVECTOR aDiff = XMVectorSubtract(centroidV, aV);
            XMVECTOR bDiff = XMVectorSubtract(centroidV, bV);

            auto aLengthV = XMVector3LengthSq(aDiff);
            auto bLengthV = XMVector3LengthSq(bDiff);

            f32 aLength, bLength;
            XMStoreFloat(&aLength, aLengthV);
            XMStoreFloat(&bLength, bLengthV);

            return aLength > bLength;
        });
        if (m_pointLights.size() > MAX_LIGHT)
        {
            logger.warn("Point Light limit threshold exceeded!");
        }
        m_pointLights.erase(m_pointLights.begin() + 3, m_pointLights.end());
    }

    std::vector<SpotLight*> spotLightsSorted;
    std::vector<PointLight*> pointLightsSorted;

    std::transform(m_spotLights.begin(), m_spotLights.end(), std::back_inserter(spotLightsSorted), [](ILight* light) {
        return (SpotLight*)light;
    });

    std::transform(m_pointLights.begin(), m_pointLights.end(), std::back_inserter(pointLightsSorted), [](ILight* light) {
        return (PointLight*)light;
    });

    return new MeshViewModel(m_mesh, spotLightsSorted, pointLightsSorted);
}
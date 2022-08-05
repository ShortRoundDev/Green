#include "LightVolume.h"

LightVolume::LightVolume(ILight* light) : MeshEntity(
    new Mesh(light->getBounds()), LIGHT_VOLUME
),
    m_light(light)
{

}

LightVolume::~LightVolume()
{

}

ILight* LightVolume::getLight()
{
    return m_light;
}

#include "AmbientLightVolume.h"

#include "DirectXMath.h"

#include "Mesh.h"
#include "MapFile_Parse.h"
#include "Logger.h"

using namespace DirectX;

static Logger logger = CreateLogger("AmbientLightVolume");

AmbientLightVolume* AmbientLightVolume::Create(MF_MeshEntity* meshEntity)
{
    DirectionalLight light;
    MF_Vector4 ambientA;
    MF_Vector4 ambientB;
    MF_Vector3 ambientDirection;
    float hardness;

    auto entity = meshEntity->rawEntity;

    if (!MF_GetAttributeVec4(entity, "ambientA", &ambientA))
    {
        logger.err("No ambient A component!");
        return NULL;
    }
    if (!MF_GetAttributeVec4(entity, "ambientB", &ambientB))
    {
        logger.err("No ambient B component!");
        return NULL;
    }
    if (!MF_GetAttributeVec3(entity, "ambientDirection", &ambientDirection))
    {
        logger.err("No ambient direction!");
        return NULL;
    }
    if (!MF_GetAttributeFloat(entity, "hardness", &hardness))
    {
        logger.err("No Ambient hardness!");
        return NULL;
    }

    light.ambientA = MFCOL_TO_XM4(ambientA);
    light.ambientB = MFCOL_TO_XM4(ambientB);
    light.ambientDirection = MF3_TO_XM4(ambientDirection);
    light.hardness = hardness;

    if (entity->totalBrushes != 1)
    {
        logger.err("Ambient Light Volume can only have 1 brush! Got %d", entity->totalBrushes);
        return NULL;
    }

    //AABB aabb;
    auto brush = meshEntity->meshes[0];
    auto firstVertex = brush.vertices[0];
    XMFLOAT3 min = MF3_TO_XM3(firstVertex.vertex);
    XMFLOAT3 max = MF3_TO_XM3(firstVertex.vertex);

    for (int i = 1; i < brush.totalVertices; i++)
    {
        XMFLOAT3 vertex = MF3_TO_XM3(brush.vertices[i].vertex);
        if (min.x > vertex.x)
        {
            min.x = vertex.x;
        }
        if (min.y > vertex.y)
        {
            min.y = vertex.y;
        }
        if (min.z > vertex.z)
        {
            min.z = vertex.z;
        }

        if (max.x < vertex.x)
        {
            max.x = vertex.x;
        }
        if (max.y < vertex.y)
        {
            max.y = vertex.y;
        }
        if (max.z < vertex.z)
        {
            max.z = vertex.z;
        }
    }

    AABB aabb = AABB(min, max);
    Mesh* mesh = new Mesh(aabb);
    
    return new AmbientLightVolume(mesh, light);
}

AmbientLightVolume::AmbientLightVolume(Mesh* mesh, DirectionalLight light) :
    MeshEntity(mesh, AMBIENT_LIGHT_VOLUME),
    m_directionalLight(light)
{

}

AmbientLightVolume::~AmbientLightVolume()
{

}

const DirectionalLight& AmbientLightVolume::getLightDesc()
{
    return m_directionalLight;
}
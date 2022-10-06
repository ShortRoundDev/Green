#pragma once

#include "GameManager.h"
#include "GameObject.h"
#include "Mesh.h"
#include "Shader.h"
#include "Logger.h"
#include "GltfMeshFactory.h"
#include "AnimationSkeleton.h"

#include <DirectXMath.h>

#include <string>
#include <map>

using namespace physx;
using namespace DirectX;

class Actor : public GameObject
{
public:
    Actor(std::string model, XMFLOAT3 pos, u64 typeId) : Actor(
        model, pos, typeId, &Game
    )
    {

    }

    Actor(std::string model, XMFLOAT3 pos, u64 typeId, GameManager* game) :
        GameObject(pos, typeId, game),
        m_mesh(nullptr),
        m_controller(nullptr),
        m_shader(nullptr),
        m_animations()
    {
        GltfMeshFactory factory(model);
        MeshActor meshActor;
        if (factory.createMesh(meshActor))
        {
            m_mesh = meshActor.mesh;
            m_animations = meshActor.animations;
        }
    }

    virtual ~Actor()
    {

    }

    virtual void think() = 0;

protected:
    PxController* m_controller;
    Mesh* m_mesh;
    std::map<std::string, AnimationSkeleton*> m_animations;
    std::vector<XMMATRIX> m_inverseBindMatrices;
    Shader* m_shader;
};


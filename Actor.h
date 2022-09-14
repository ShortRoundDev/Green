#pragma once

#include "GameManager.h"
#include "GameObject.h"
#include "Mesh.h"
#include "Shader.h"
#include "Logger.h"

#include <string>
#include "GltfMeshFactory.h"

using namespace physx;

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
        m_shader(nullptr)
    {
        GltfMeshFactory factory(model);
        MeshActor meshActor;
        if (factory.createMesh(meshActor))
        {
            m_mesh = meshActor.mesh;
        }
    }

    virtual ~Actor()
    {

    }

    virtual void think() = 0;

protected:
    PxController* m_controller;
    Mesh* m_mesh;
    Shader* m_shader;
};


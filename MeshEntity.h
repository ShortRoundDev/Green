#pragma once

#include "Mesh.h"

#include "GTypes.h"

constexpr u64 WORLDSPAWN            = 1L;
constexpr u64 AMBIENT_LIGHT_VOLUME  = 2L;

class MeshEntity
{
public:
    MeshEntity(Mesh* mesh, u64 type) :
        m_mesh(mesh),
        m_type(type){ }

    ~MeshEntity() { }

    Mesh* getMesh()
    {
        return m_mesh;
    }

    u64 getType()
    {
        return m_type;
    }

private:
    Mesh* m_mesh;
    u64 m_type;
};


#pragma once

#include "MeshEntity.h"

class WorldSpawn : public MeshEntity
{
public:
    WorldSpawn(Mesh* mesh) : MeshEntity(mesh, WORLDSPAWN) { }
    virtual ~WorldSpawn() { }
};


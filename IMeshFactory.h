#pragma once

#include <vector>
#include <PxActor.h>

using namespace physx;

class Mesh;

//Combination of drawable mesh and physics actors
struct MeshActor
{
    Mesh* mesh;
    PxActor* actor;
};

__interface IMeshFactory
{
public:
    bool createMesh(MeshActor& meshActor);
    void createMeshes(std::vector<MeshActor>& meshActors);
};
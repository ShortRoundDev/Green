#pragma once

#include <PxActor.h>

#include "AnimationSkeleton.h"

#include <vector>
#include <string>
#include <map>

using namespace physx;

class Mesh;

//Combination of drawable mesh and physics actors
struct MeshActor
{
    Mesh* mesh;
    PxActor* actor;
    std::map<std::string, AnimationSkeleton*> animations;
};

__interface IMeshFactory
{
public:
    bool createMesh(MeshActor& meshActor);
    void createMeshes(std::vector<MeshActor>& meshActors);
};
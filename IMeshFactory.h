#pragma once

#include <PxActor.h>

#include "AnimationSkeleton.h"

#include <DirectXMath.h>

#include <vector>
#include <string>
#include <map>

using namespace physx;
using namespace DirectX;

class Mesh;

//Combination of drawable mesh and physics actors
struct MeshActor
{
    Mesh* mesh;
    PxActor* actor;
    std::map<std::string, AnimationSkeleton*> animations;
    std::vector<XMMATRIX> inverseBindMatrices;
};

__interface IMeshFactory
{
public:
    bool createMesh(MeshActor& meshActor);
    void createMeshes(std::vector<MeshActor>& meshActors);
};
#pragma once
#include "IMeshFactory.h"
#include "GameManager.h"

#include "GLTFSDK/Document.h"
#include "GLTFSDK/GLTFResourceReader.h"

#include <string>

using namespace Microsoft;

class GltfMeshFactory : public IMeshFactory
{
public:
    GltfMeshFactory(std::string path, GameManager* game = &Game);

    bool createMesh(MeshActor& meshActor);
    void createMeshes(std::vector<MeshActor>& meshActors);

private:
    std::string m_path;
    GameManager* m_game;
    u32 m_nodeIterator;
    bool m_status;

    bool createMesh(MeshActor&, const glTF::Document& document, glTF::GLTFResourceReader* reader);
};

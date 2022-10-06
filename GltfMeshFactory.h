#pragma once
#include "IMeshFactory.h"
#include "GameManager.h"
#include "AnimationSkeleton.h"

#include "GLTFSDK/Document.h"
#include "GLTFSDK/GLTFResourceReader.h"

#include <string>
#include <map>
#include <vector>

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

    bool createMesh(
        MeshActor& meshActor,
        const glTF::Document& document,
        glTF::GLTFResourceReader* reader
    );

    template<typename T, typename U, auto V>
    void getAttributeData(
        const glTF::Document& document,
        const glTF::MeshPrimitive& primitive,
        glTF::GLTFResourceReader* reader,
        std::string accessor,
        std::vector<U>& list
    );

    template<typename T, typename W, typename U, auto V>
    void getAttributeData(
        const glTF::Document& document,
        const glTF::MeshPrimitive& primitive,
        glTF::GLTFResourceReader* reader,
        std::string accessor,
        std::vector<U>& list
    );

    void meshFromPrimitive(
        Mesh*& mesh,
        const glTF::MeshPrimitive& primitive,
        const glTF::Document& document,
        glTF::GLTFResourceReader* reader
    );

    void animationFromSkin(
        std::map<std::string, AnimationSkeleton*>& animations,
        const glTF::Skin& skin,
        const glTF::Document& document,
        glTF::GLTFResourceReader* reader
    );
};

#pragma once

#include "GTypes.h"
#include "GVertex.h"
#include "AABB.h"
#include "SkinnedData.h"

#include <d3d11.h>
#include <wrl/client.h>

#include "PxPhysicsAPI.h"

#include <vector>
#include <string>
#include <map>

using namespace Microsoft::WRL;
using namespace physx;

class Texture;
class GameManager;
class AABB;
class ILight;
class MeshViewModel;
class NavMesh;

class Mesh
{
public:

    static bool loadObj(
        std::string path,
        std::vector<Mesh*>& meshes,
        GameManager* gameManager
    );

    static bool loadObj(
        std::string path,
        std::vector<Mesh*>& meshes,
        NavMesh* navMesh,
        GameManager* gameManager,
        bool flipX = false
    );

    static bool loadGltf(
        std::string path,
        Mesh** mesh,
        GameManager* gameManager
    );

    static bool loadMemory(
        std::vector<GVertex>& vertices,
        sz vertCount,
        std::vector<u32>& indices,
        sz indexCount,
        Texture* texture,
        Mesh*& mesh
    );

    static bool createBbox(
        AABB aabb,
        Texture* texture,
        Mesh*& mesh
    );

    void draw();

    Mesh(AABB aabb);
    ~Mesh();

    AABB getBox();
    XMFLOAT3 getCentroid();

    void addLight(ILight* light);

    void getTransforms(const std::string& name, f32 timePos, std::vector<XMMATRIX>& transforms);

    Texture* getTexture();
    void setTexture(Texture* texture);

private:
    Mesh();
    Mesh(
        const std::vector<GVertex>& vertices,
        sz vertCount,
        const std::vector<u32>& indices,
        sz indexCount,
        Texture* texture
    );

    bool initialize(
        const std::vector<GVertex>& vertices,
        sz vertCount,
        const std::vector<u32>& indices,
        sz indexCount,
        Texture* texture
    );

    ComPtr<ID3D11Buffer> m_vertexBuffer;
    u32 m_vertCount;

    ComPtr<ID3D11Buffer> m_indexBuffer;
    u32 m_indexCount;
    std::vector<u32> m_indices;

    Texture* m_texture;
    u32 m_textureCount;

    bool m_status;

    AABB m_box;


    std::vector<GVertex> m_vertices;
    std::vector<ILight*> m_lights;

    /*std::vector<u32> m_boneHierarchy;
    std::vector<XMMATRIX> m_boneOffsets;
    std::map<std::string, AnimationClip> m_animations;*/

    SkinnedData* m_animationData;

    void initAABB(const std::vector<GVertex>& vertices);
    void concatenateVertices(std::vector<GVertex>& out, const std::vector<GVertex>& a, const std::vector<GVertex>& b);
    void concatenateIndices(std::vector<u32>& out, const std::vector<u32>& a, const std::vector<u32>& b);
    static void initSkinnedData(std::string gltfPath);

    static bool GenerateNavMesh(GameManager* game, NavMesh* navMesh, std::vector<Mesh*> meshes);
};

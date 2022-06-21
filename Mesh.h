#pragma once

#include "GTypes.h"
#include "GVertex.h"
#include "AABB.h"
#include "AnimationAction.h"

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

class Mesh
{
public:

    static bool createMapFromFile(
        std::string path,
        std::vector<Mesh*>& meshes,
        GameManager* gameManager,
        bool flipX
    );

    static bool loadGltf(
        std::string path,
        Mesh** mesh,
        GameManager* gameManager
    );

    static bool createMapFromFile(
        std::string path,
        std::vector<Mesh*>& meshes,
        GameManager* gameManager
    );

    void draw();

    Mesh(AABB aabb);
    ~Mesh();

    AABB getBox();

    void addLight(ILight* light);

    MeshViewModel* getViewModel();
    void getTransforms(const std::string& name, f32 timePos, std::vector<XMMATRIX>& transforms);

    Texture* getTexture();
    void setTexture(Texture* texture);

private:
    Mesh();
    bool initialize(const std::vector<GVertex>& vertices, size_t vertCount, const std::vector<u32>& indices, size_t indexCount, Texture* texture);

    ComPtr<ID3D11Buffer> m_vertexBuffer;
    u32 m_vertCount;

    ComPtr<ID3D11Buffer> m_indexBuffer;
    u32 m_indexCount;

    Texture* m_texture;
    u32 m_textureCount;

    bool m_status;

    AABB m_box;

    XMFLOAT3 getCentroid();

    std::vector<GVertex> m_vertices;
    std::vector<ILight*> m_lights;

    std::vector<u32> m_boneHierarchy;
    std::vector<XMMATRIX> m_boneOffsets;
    std::map<std::string, AnimationAction> m_animations;

    void initAABB(const std::vector<GVertex>& vertices);
    void concatenateVertices(std::vector<GVertex>& out, const std::vector<GVertex>& a, const std::vector<GVertex>& b);
    void concatenateIndices(std::vector<u32>& out, const std::vector<u32>& a, const std::vector<u32>& b);
};

#pragma once

#include "GTypes.h"
#include "GVertex.h"
#include "AABB.h"

#include <d3d11.h>
#include <wrl/client.h>

#include "PxPhysicsAPI.h"

#include <vector>
#include <string>

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

    static bool createFromFile(
        std::string path,
        std::vector<Mesh*>& meshes,
        GameManager* gameManager,
        bool flipX
    );

    static bool createFromFile(
        std::string path,
        std::vector<Mesh*>& meshes,
        GameManager* gameManager
    );
    void draw();

    Mesh();
    ~Mesh();

    AABB getBox();

    void addLight(ILight* light);

    MeshViewModel* getViewModel();

    Texture* getTexture();
    void setTexture(Texture* texture);

private:

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

    void initAABB(const std::vector<GVertex>& vertices);
    void concatenateVertices(std::vector<GVertex>& out, const std::vector<GVertex>& a, const std::vector<GVertex>& b);
    void concatenateIndices(std::vector<u32>& out, const std::vector<u32>& a, const std::vector<u32>& b);
};

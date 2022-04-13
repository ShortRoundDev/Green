#pragma once

#include "GTypes.h"
#include "GVertex.h"

#include <d3d11.h>
#include <wrl/client.h>

#include "reactphysics3d/reactphysics3d.h"

#include <vector>
#include <string>

using namespace Microsoft::WRL;
using namespace reactphysics3d;

class Texture;
class GameManager;

class Mesh
{
public:
    static bool createFromFile(
        std::string path,
        Mesh** meshes, size_t* totalMeshes,
        ConvexMeshShape*** physicsMeshes, size_t* totalPhysicsMeshes,
        GameManager* gameManager
    );
    void draw();

    Mesh();
    ~Mesh();

private:

    bool initialize(const std::vector<GVertex>& vertices, size_t vertCount, const std::vector<u32>& indices, size_t indexCount, Texture* texture);

    ComPtr<ID3D11Buffer> m_vertexBuffer;
    u32 m_vertCount;

    ComPtr<ID3D11Buffer> m_indexBuffer;
    u32 m_indexCount;

    Texture* m_texture;
    u32 m_textureCount;

    bool m_status;

    void concatenateVertices(std::vector<GVertex>& out, const std::vector<GVertex>& a, const std::vector<GVertex>& b);
    void concatenateIndices(std::vector<u32>& out, const std::vector<u32>& a, const std::vector<u32>& b);
};

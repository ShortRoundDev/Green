#pragma once

#include "GTypes.h"
#include "GVertex.h"

#include <d3d11.h>
#include <wrl/client.h>

#include <vector>
#include <string>

using namespace Microsoft::WRL;

class Texture;

class Mesh
{
public:
    static bool createFromFile(std::string path, Mesh** meshes, size_t* totalMeshes);
    void draw();

private:
    Mesh();
    ~Mesh();

    bool initialize(const std::vector<GVertex>& vertices, size_t vertCount, const std::vector<u32>& indices, size_t indexCount, Texture* texture);

    ComPtr<ID3D11Buffer> m_vertexBuffer;
    u32 m_vertCount;

    ComPtr<ID3D11Buffer> m_indexBuffer;
    u32 m_indexCount;

    Texture* m_texture;
    u32 m_textureCount;

    bool m_status;
};

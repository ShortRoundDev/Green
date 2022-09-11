#include "OctreeNode.h"

#include "Shader.h"
#include "GraphicsManager.h"
#include "MeshEntity.h"

#include <queue>

using namespace _NodeType;
using namespace _NodeDir;

/***** STATIC FIELDS *****/
#pragma region STATIC_FIELDS
bool OctreeNode::s_showGreen = false;
int OctreeNode::s_showSize = 0;
#pragma endregion

/***** ADJACENCY, FACE, REFL, ETC. TABLES *****/
#pragma region ADJACENCY_TABLES
u8 OctreeNode::AdjTable[26][8] = {
    /* L   */ { true,   true,   true,   true,   false,  false,  false,  false },
    /* R   */ { false,  false,  false,  false,  true,   true,   true,   true  },
    /* D   */ { true,   true,   false,  false,  true,   true,   false,  false },
    /* U   */ { false,  false,  true,   true,   false,  false,  true,   true  },
    /* B   */ { true,   false,  true,   false,  true,   false,  true,   false },
    /* F   */ { false,  true,   false,  true,   false,  true,   false,  true  },
    /* LD  */ { true,   true,   false,  false,  false,  false,  false,  false },
    /* LU  */ { false,  false,  true,   true,   false,  false,  false,  false },
    /* LB  */ { true,   false,  true,   false,  false,  false,  false,  false },
    /* LF  */ { false,  true,   false,  true,   false,  false,  false,  false },
    /* RD  */ { false,  false,  false,  false,  true,   true,   false,  false },
    /* RU  */ { false,  false,  false,  false,  false,  false,  true,   true  },
    /* RB  */ { false,  false,  false,  false,  true,   false,  true,   false },
    /* RF  */ { false,  false,  false,  false,  false,  true,   false,  true  },
    /* DB  */ { true,   false,  false,  false,  true,   false,  false,  false },
    /* DF  */ { false,  true,   false,  false,  false,  true,   false,  false },
    /* UB  */ { false,  false,  true,   false,  false,  false,  true,   false },
    /* UF  */ { false,  false,  false,  true,   false,  false,  false,  true  },
    /* LDB */ { true,   false,  false,  false,  false,  false,  false,  false },
    /* LDF */ { false,  true,   false,  false,  false,  false,  false,  false },
    /* LUB */ { false,  false,  true,   false,  false,  false,  false,  false },
    /* LUF */ { false,  false,  false,  true,   false,  false,  false,  false },
    /* RDB */ { false,  false,  false,  false,  true,   false,  false,  false },
    /* RDF */ { false,  false,  false,  false,  false,  true,   false,  false },
    /* RUB */ { false,  false,  false,  false,  false,  false,  true,   false },
    /* RUF */ { false,  false,  false,  false,  false,  false,  false,  true  }
};

NodeType OctreeNode::CommonFaceChildren[6][4] = {
    /* L */ { NodeType::RDB, NodeType::RDF, NodeType::RUB, NodeType::RUF },
    /* R */ { NodeType::LDB, NodeType::LDF, NodeType::LUB, NodeType::LUF },
    /* D */ { NodeType::LUB, NodeType::LUF, NodeType::RUB, NodeType::RUF },
    /* U */ { NodeType::LDB, NodeType::LDF, NodeType::RDB, NodeType::RDF },
    /* B */ { NodeType::LDF, NodeType::LUF, NodeType::RDF, NodeType::RUF },
    /* F */ { NodeType::LDB, NodeType::LUB, NodeType::RDB, NodeType::RUB }
};

NodeType OctreeNode::ReflTable[26][8] = {
    { NodeType::RDB, NodeType::RDF, NodeType::RUB, NodeType::RUF, NodeType::LDB, NodeType::LDF, NodeType::LUB, NodeType::LUF, },
    { NodeType::RDB, NodeType::RDF, NodeType::RUB, NodeType::RUF, NodeType::LDB, NodeType::LDF, NodeType::LUB, NodeType::LUF, },
    { NodeType::LUB, NodeType::LUF, NodeType::LDB, NodeType::LDF, NodeType::RUB, NodeType::RUF, NodeType::RDB, NodeType::RDF, },
    { NodeType::LUB, NodeType::LUF, NodeType::LDB, NodeType::LDF, NodeType::RUB, NodeType::RUF, NodeType::RDB, NodeType::RDF, },
    { NodeType::LDF, NodeType::LDB, NodeType::LUF, NodeType::LUB, NodeType::RDF, NodeType::RDB, NodeType::RUF, NodeType::RUB, },
    { NodeType::LDF, NodeType::LDB, NodeType::LUF, NodeType::LUB, NodeType::RDF, NodeType::RDB, NodeType::RUF, NodeType::RUB, },
    { NodeType::RUB, NodeType::RUF, NodeType::RDB, NodeType::RDF, NodeType::LUB, NodeType::LUF, NodeType::LDB, NodeType::LDF, },
    { NodeType::RUB, NodeType::RUF, NodeType::RDB, NodeType::RDF, NodeType::LUB, NodeType::LUF, NodeType::LDB, NodeType::LDF, },
    { NodeType::RDF, NodeType::RDB, NodeType::RUF, NodeType::RUB, NodeType::LDF, NodeType::LDB, NodeType::LUF, NodeType::LUB, },
    { NodeType::RDF, NodeType::RDB, NodeType::RUF, NodeType::RUB, NodeType::LDF, NodeType::LDB, NodeType::LUF, NodeType::LUB, },
    { NodeType::RUB, NodeType::RUF, NodeType::RDB, NodeType::RDF, NodeType::LUB, NodeType::LUF, NodeType::LDB, NodeType::LDF, },
    { NodeType::RUB, NodeType::RUF, NodeType::RDB, NodeType::RDF, NodeType::LUB, NodeType::LUF, NodeType::LDB, NodeType::LDF, },
    { NodeType::RDF, NodeType::RDB, NodeType::RUF, NodeType::RUB, NodeType::LDF, NodeType::LDB, NodeType::LUF, NodeType::LUB, },
    { NodeType::RDF, NodeType::RDB, NodeType::RUF, NodeType::RUB, NodeType::LDF, NodeType::LDB, NodeType::LUF, NodeType::LUB, },
    { NodeType::LUF, NodeType::LUB, NodeType::LDF, NodeType::LDB, NodeType::RUF, NodeType::RUB, NodeType::RDF, NodeType::RDB, },
    { NodeType::LUF, NodeType::LUB, NodeType::LDF, NodeType::LDB, NodeType::RUF, NodeType::RUB, NodeType::RDF, NodeType::RDB, },
    { NodeType::LUF, NodeType::LUB, NodeType::LDF, NodeType::LDB, NodeType::RUF, NodeType::RUB, NodeType::RDF, NodeType::RDB, },
    { NodeType::LUF, NodeType::LUB, NodeType::LDF, NodeType::LDB, NodeType::RUF, NodeType::RUB, NodeType::RDF, NodeType::RDB, },
    { NodeType::RUF, NodeType::RUB, NodeType::RDF, NodeType::RDB, NodeType::LUF, NodeType::LUB, NodeType::LDF, NodeType::LDB, },
    { NodeType::RUF, NodeType::RUB, NodeType::RDF, NodeType::RDB, NodeType::LUF, NodeType::LUB, NodeType::LDF, NodeType::LDB, },
    { NodeType::RUF, NodeType::RUB, NodeType::RDF, NodeType::RDB, NodeType::LUF, NodeType::LUB, NodeType::LDF, NodeType::LDB, },
    { NodeType::RUF, NodeType::RUB, NodeType::RDF, NodeType::RDB, NodeType::LUF, NodeType::LUB, NodeType::LDF, NodeType::LDB, },
    { NodeType::RUF, NodeType::RUB, NodeType::RDF, NodeType::RDB, NodeType::LUF, NodeType::LUB, NodeType::LDF, NodeType::LDB, },
    { NodeType::RUF, NodeType::RUB, NodeType::RDF, NodeType::RDB, NodeType::LUF, NodeType::LUB, NodeType::LDF, NodeType::LDB, },
    { NodeType::RUF, NodeType::RUB, NodeType::RDF, NodeType::RDB, NodeType::LUF, NodeType::LUB, NodeType::LDF, NodeType::LDB, },
    { NodeType::RUF, NodeType::RUB, NodeType::RDF, NodeType::RDB, NodeType::LUF, NodeType::LUB, NodeType::LDF, NodeType::LDB  }
};

NodeDir OctreeNode::CommonFaceTable[26][8] = {
    { NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL },
    { NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL },
    { NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL },
    { NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL },
    { NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL },
    { NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL },
    { NodeDir::NIL, NodeDir::NIL, NodeDir::L,   NodeDir::L,   NodeDir::D,   NodeDir::D,   NodeDir::NIL, NodeDir::NIL },
    { NodeDir::L,   NodeDir::L,   NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::U,   NodeDir::NIL },
    { NodeDir::NIL, NodeDir::L,   NodeDir::NIL, NodeDir::L,   NodeDir::B,   NodeDir::NIL, NodeDir::B,   NodeDir::NIL },
    { NodeDir::L,   NodeDir::NIL, NodeDir::L,   NodeDir::NIL, NodeDir::NIL, NodeDir::F,   NodeDir::NIL, NodeDir::F   },
    { NodeDir::D,   NodeDir::D,   NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::R,   NodeDir::R   },
    { NodeDir::NIL, NodeDir::NIL, NodeDir::U,   NodeDir::U,   NodeDir::R,   NodeDir::R,   NodeDir::NIL, NodeDir::NIL },
    { NodeDir::B,   NodeDir::NIL, NodeDir::B,   NodeDir::NIL, NodeDir::NIL, NodeDir::R,   NodeDir::NIL, NodeDir::R   },
    { NodeDir::NIL, NodeDir::F,   NodeDir::NIL, NodeDir::F,   NodeDir::R,   NodeDir::NIL, NodeDir::R,   NodeDir::NIL },
    { NodeDir::NIL, NodeDir::D,   NodeDir::B,   NodeDir::NIL, NodeDir::NIL, NodeDir::D,   NodeDir::B,   NodeDir::NIL },
    { NodeDir::D,   NodeDir::NIL, NodeDir::NIL, NodeDir::F,   NodeDir::D,   NodeDir::NIL, NodeDir::NIL, NodeDir::F   },
    { NodeDir::B,   NodeDir::NIL, NodeDir::NIL, NodeDir::U,   NodeDir::B,   NodeDir::NIL, NodeDir::NIL, NodeDir::U   },
    { NodeDir::NIL, NodeDir::F,   NodeDir::U,   NodeDir::NIL, NodeDir::NIL, NodeDir::F,   NodeDir::U,   NodeDir::NIL },
    { NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::L,   NodeDir::NIL, NodeDir::D,   NodeDir::B,   NodeDir::NIL },
    { NodeDir::NIL, NodeDir::NIL, NodeDir::L,   NodeDir::NIL, NodeDir::D,   NodeDir::NIL, NodeDir::NIL, NodeDir::F   },
    { NodeDir::NIL, NodeDir::L,   NodeDir::NIL, NodeDir::NIL, NodeDir::B,   NodeDir::NIL, NodeDir::NIL, NodeDir::U   },
    { NodeDir::L,   NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::F,   NodeDir::U,   NodeDir::NIL },
    { NodeDir::NIL, NodeDir::D,   NodeDir::B,   NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::R   },
    { NodeDir::D,   NodeDir::NIL, NodeDir::NIL, NodeDir::F,   NodeDir::NIL, NodeDir::NIL, NodeDir::R,   NodeDir::NIL },
    { NodeDir::B,   NodeDir::NIL, NodeDir::NIL, NodeDir::U,   NodeDir::NIL, NodeDir::R,   NodeDir::NIL, NodeDir::NIL },
    { NodeDir::NIL, NodeDir::F,   NodeDir::U,   NodeDir::NIL, NodeDir::R,   NodeDir::NIL, NodeDir::NIL, NodeDir::NIL }
};

NodeDir OctreeNode::CommonEdgeTable[8][8] = {
    { NodeDir::NIL, NodeDir::LD,  NodeDir::LB,  NodeDir::NIL, NodeDir::DB,  NodeDir::NIL, NodeDir::NIL, NodeDir::NIL },
    { NodeDir::LD,  NodeDir::NIL, NodeDir::NIL, NodeDir::LF,  NodeDir::NIL, NodeDir::DF,  NodeDir::NIL, NodeDir::NIL },
    { NodeDir::LB,  NodeDir::NIL, NodeDir::NIL, NodeDir::LU,  NodeDir::NIL, NodeDir::NIL, NodeDir::UB,  NodeDir::NIL },
    { NodeDir::NIL, NodeDir::LF,  NodeDir::LU,  NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::UF  },
    { NodeDir::DB,  NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::RD,  NodeDir::RB,  NodeDir::NIL },
    { NodeDir::NIL, NodeDir::DF,  NodeDir::NIL, NodeDir::NIL, NodeDir::RD,  NodeDir::NIL, NodeDir::NIL, NodeDir::RF  },
    { NodeDir::NIL, NodeDir::NIL, NodeDir::UB,  NodeDir::NIL, NodeDir::RB,  NodeDir::NIL, NodeDir::NIL, NodeDir::RU  },
    { NodeDir::NIL, NodeDir::NIL, NodeDir::NIL, NodeDir::UF,  NodeDir::NIL, NodeDir::RF,  NodeDir::RU,  NodeDir::NIL }
};

#pragma endregion

/***** STATIC FIELD GETTER/SETTERS *****/
#pragma region STATIC_FIELD_GETTERS
bool OctreeNode::IsShowGreen()
{
    return OctreeNode::s_showGreen;
}

void OctreeNode::SetShowGreen(bool showGreen)
{
    OctreeNode::s_showGreen = showGreen;
}

int OctreeNode::GetShowSize()
{
    return OctreeNode::s_showSize;
}

void OctreeNode::SetShowSize(int showSize)
{
    OctreeNode::s_showSize = showSize;
    if (s_showSize > MAX_TREE_DEPTH)
    {
        s_showSize = MAX_TREE_DEPTH;
    }
    else if (s_showSize < -1)
    {
        s_showSize = -1;
    }
}
#pragma endregion

/***** CTOR/DTOR *****/
#pragma region CTOR_DTOR
OctreeNode::OctreeNode(
    OctreeNode* parent,
    AABB bounds,
    const std::vector<MeshEntity*>& meshes,
    u8 division,
    NodeType nodeType
) :
    m_parent(parent),
    m_bounds(bounds),
    m_division(division),
    m_nodeType(nodeType),
    m_isLeaf(false),
    m_isSolid(false),
    m_allContained(false)
{
    memset(m_children, 0, sizeof(m_children));

#if _DEBUG
    Mesh::createBbox(m_bounds, nullptr, m_debugMesh);
    m_debugShader = Graphics.getShader(L"Octree");
#endif

    /* If last child or empty, make leaf */
    if (division == 0 || meshes.size() == 0)
    {
        makeLeaf(meshes);
        return;
    }

    // init octants
    AABB octants[8];
    initializeOctants(octants);

    //divide meshes into 8 buckets
    std::vector<MeshEntity*> addToChildren[8];
    bool allContained = divideInsertMeshes(meshes, octants, addToChildren);

    //early exit check
    if (division == 1 && allContained)
    {
        m_allContained = true;
        makeLeaf(meshes);
        return;
    }

    //insert into 8 children
    int childrenAllContained = 0;
    for (int i = 0; i < 8; i++)
    {
        m_children[i] = new OctreeNode(this, octants[i], addToChildren[i], division - 1, (NodeType)i);
        if (m_children[i]->m_allContained)
        {
            childrenAllContained++;
        }
    }

    if (childrenAllContained == 8)
    {
        for (int i = 0; i < 8; i++)
        {
            delete m_children[i];
            m_children[i] = nullptr;
        }
        makeLeaf(meshes);
    }
}

OctreeNode::~OctreeNode()
{
    for (int i = 0; i < 8; i++)
    {
        if (m_children[i])
        {
            delete m_children[i];
            m_children[i] = nullptr;
        }
    }
}
#pragma endregion

/***** GETTERS/SETTERS *****/
#pragma region GETTERS_SETTERS
bool OctreeNode::isLeaf()
{
    return m_isLeaf;
}

bool OctreeNode::isSolid()
{
    return m_isSolid;
}

int OctreeNode::getDivision()
{
    return m_division;
}

OctreeNode* OctreeNode::getParent()
{
    return m_parent;
}

OctreeNode** OctreeNode::getChildren()
{
    return m_children;
}

sz OctreeNode::getSize()
{
    if (isLeaf())
    {
        return 8;
    }
    else
    {
        sz size = 0;
        for (int i = 0; i < 8; i++)
        {
            size += m_children[i]->getSize();
        }
        return size;
    }
}

std::vector<OctreeNode*>& OctreeNode::getNeighbors()
{
    return m_neighbors;
}

NodeType OctreeNode::getNodeType()
{
    return m_nodeType;
}

AABB OctreeNode::getBounds()
{
    return m_bounds;
}
#pragma endregion

/***** TREE QUERIES *****/
#pragma region TREE_QUERIES
void OctreeNode::query(
    AABB* aabb,
    std::vector<MeshEntity*>& meshes,
    std::set<MeshEntity*>& unique,
    u64 type
)
{
    if (m_meshes.size() > 0 || m_children[0] == NULL) // leaf
    {
        for (auto mesh : m_meshes)
        {
            if (std::find(unique.begin(), unique.end(), mesh) != unique.end())
            {
                continue;
            }
            auto other = mesh->getMesh()->getBox();
            // type == 0 means all types
            if ((type == 0 || mesh->getType() == type) && (other.collides(aabb) || aabb->collides(&other)))
            {
                meshes.push_back(mesh);
                unique.emplace(mesh);
            }
        }
        return;
    }
    for (int i = 0; i < 8; i++)
    {
        if (m_children[i]->getBounds().collides(aabb))
        {
            m_children[i]->query(aabb, meshes, unique, type);
        }
    }
}

void OctreeNode::query(
    XMFLOAT3 point,
    std::vector<MeshEntity*>& meshes,
    std::set<MeshEntity*>& unique,
    u64 type
)
{
    if (m_meshes.size() > 0 || m_children[0] == NULL) // leaf
    {
        for (auto mesh : m_meshes)
        {
            if (std::find(unique.begin(), unique.end(), mesh) != unique.end())
            {
                continue;
            }
            auto other = mesh->getMesh()->getBox();
            // type == 0 means all types
            if ((type == 0 || mesh->getType() == type) && other.contains(point))
            {
                meshes.push_back(mesh);
                unique.emplace(mesh);
            }
        }
        return;
    }
    for (int i = 0; i < 8; i++)
    {
        if (m_children[i]->getBounds().contains(point))
        {
            m_children[i]->query(point, meshes, unique, type);
        }
    }
}

OctreeNode* OctreeNode::query(XMFLOAT3 point)
{
    if (m_isLeaf) // leaf
    {
        if (m_bounds.contains(point))
        {
            return this;
        }
    }
    for (int i = 0; i < 8; i++) // this can be reduced to 3 planar checks
    {
        if (m_children[i]->getBounds().contains(point))
        {
            return m_children[i]->query(point);
        }
    }
    return nullptr;
}
#pragma endregion

/***** DEBUG DRAWING *****/
#pragma region DEBUG_DRAWING
void OctreeNode::draw()
{
   /*if (m_isLeaf)
    {
        if ((((!IsShowGreen() && m_meshes.size() != 0)) ^ (IsShowGreen() && (m_meshes.size() == 0))) && (GetShowSize() == -1 || GetShowSize() == m_division))
        {
            draw(IsShowGreen()
                ? XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)
                : XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)
            );
        }
    }
    else
    {
        for (int i = 0; i < 8; i++)
        {
            m_children[i]->draw();
        }
    }*/
}

void OctreeNode::draw(XMFLOAT4 color)
{
    m_debugShader->use();
    m_debugShader->bindCBuffer(&color);
    m_debugMesh->draw();
}
#pragma endregion

/***** NAVIGATION *****/
#pragma region NAVIGATION

void OctreeNode::generateNeighbors()
{
    if (isLeaf())
    {
        queryNeighbors(m_neighbors);
    }
    else
    {
        for (int i = 0; i < 8; i++)
        {
            m_children[i]->generateNeighbors();
        }
    }
}

bool OctreeNode::findPath(OctreeNode* destination, std::unordered_map<OctreeNode*, OctreeNode*>& path)
{
    path.clear();
    std::vector<std::tuple<f32, OctreeNode*>> frontier;
    frontier.reserve(3000);

    std::make_heap(frontier.begin(), frontier.end());
    
    frontier.push_back(std::make_tuple(0.0f, this));
    std::push_heap(frontier.begin(), frontier.end()); // doesn't matter what cost is for first node
    path[this] = nullptr;
    std::unordered_map<OctreeNode*, f32> costSoFar;
    costSoFar[this] = 0;

    while (frontier.size() > 0)
    {
        std::pop_heap(frontier.begin(), frontier.end());
        auto& current = frontier.back();
        frontier.pop_back();
        OctreeNode* node = std::get<1>(current);

        if (node == destination)
        {
            return true;
        }

        for (const auto& next : node->getNeighbors())
        {
            f32 newCost = costSoFar[node] + 1.0f;
            
            auto lastCost = costSoFar.find(next);
            bool notInCostSoFar = (lastCost == costSoFar.end());
            if (notInCostSoFar || newCost < lastCost->second)
            {
                costSoFar[next] = newCost;
                f32 priority = newCost + next->nodeHeuristic(destination->getBounds().getMinV());
                frontier.push_back(std::make_tuple(priority, (OctreeNode*)next));
                std::push_heap(frontier.begin(), frontier.end());
                path[next] = node;
            }
        }
    }
    return false;
}


#pragma endregion

/***** INITIALIZATION METHODS *****/
#pragma region INITIALIZATION_METHODS
void OctreeNode::initializeOctants(
    AABB(&octants)[8]
)
{
    XMVECTOR minV = XMLoadFloat3(&(m_bounds.getMin()));
    XMVECTOR maxV = XMLoadFloat3(&(m_bounds.getMax()));
    XMVECTOR diffV = XMVectorSubtract(maxV, minV);
    XMVECTOR halfSizeV = XMVectorScale(diffV, 0.5f);
    XMFLOAT3 halfSize;
    XMFLOAT3 half;
    XMVECTOR halfV = XMVectorAdd(minV, halfSizeV);

    XMStoreFloat3(&half, halfV);
    XMStoreFloat3(&halfSize, halfSizeV);

    XMFLOAT3 min = m_bounds.getMin();
    XMFLOAT3 max = m_bounds.getMax();

    f32 L1 = min.x,
        L2 = half.x,

        R1 = half.x,
        R2 = max.x,

        D1 = min.y,
        D2 = half.y,

        U1 = half.y,
        U2 = max.y,

        B1 = min.z,
        B2 = half.z,

        F1 = half.z,
        F2 = max.z;

    octants[0] = AABB({ L1, D1, B1 }, { L2, D2, B2 }); // LDB
    octants[1] = AABB({ L1, D1, F1 }, { L2, D2, F2 }); // LDF
    octants[2] = AABB({ L1, U1, B1 }, { L2, U2, B2 }); // LUB
    octants[3] = AABB({ L1, U1, F1 }, { L2, U2, F2 }); // LUF
    octants[4] = AABB({ R1, D1, B1 }, { R2, D2, B2 }); // RDB
    octants[5] = AABB({ R1, D1, F1 }, { R2, D2, F2 }); // RDF
    octants[6] = AABB({ R1, U1, B1 }, { R2, U2, B2 }); // RUB
    octants[7] = AABB({ R1, U1, F1 }, { R2, U2, F2 });  // RUF
}

bool OctreeNode::divideInsertMeshes(
    const std::vector<MeshEntity*>& meshes,
     AABB(& octants)[8],
    std::vector<MeshEntity*>(&addToChildren)[8]
)
{
    int fullOctants = 0;
    for (auto mesh : meshes)
    {
        for (int i = 0; i < 8; i++)
        {
            if (mesh->getMesh()->getBox().collides(&octants[i]))
            {
                fullOctants |= 1 << i;
                addToChildren[i].push_back(mesh);
            }
        }
    }

    return fullOctants == 0xff;
}

void OctreeNode::makeLeaf(
    const std::vector<MeshEntity*>& meshes
)
{
    m_isLeaf = true;
    m_meshes = meshes;
    m_isSolid = meshes.size() > 0;
}
#pragma endregion

/***** NEIGHBOR HELPER METHODS *****/
#pragma region NEIGHBOR_HELPERS
OctreeNode* OctreeNode::GetEqualFaceNeighbor(
    OctreeNode* p,
    _NodeDir::NodeDir face
)
{
    if (p == nullptr)
    {
        return nullptr;
    }
    OctreeNode* next = nullptr;
    if (ADJ(face, p->getNodeType()))
    {
        next = GetEqualFaceNeighbor(p->getParent(), face);
    }
    else
    {
        next = p->getParent();
    }
    if (!next)
    {
        return nullptr;
    }
    return GetSon(next, REFLECT(face, p->getNodeType()));
}

OctreeNode* OctreeNode::GetGtEqualFaceNeighbor(
    OctreeNode* p,
    NodeDir face
)
{
    OctreeNode* q;
    if (p->getParent() != nullptr && ADJ(face, p->getNodeType()))
    {
        q = GetGtEqualFaceNeighbor(p->getParent(), face);
    }
    else
    {
        q = p->getParent();
    }
    if (q != nullptr && !q->isLeaf())
    {
        return GetSon(q, REFLECT(face, p->getNodeType()));
    }
    return q;
}

sz OctreeNode::GetAllFaceNeighbors(
    OctreeNode* p,
    _NodeDir::NodeDir face,
    std::vector<OctreeNode*>& neighbors
)
{
    auto faceNeighbor = GetGtEqualFaceNeighbor(
        p, face
    );
    if (faceNeighbor)
    {
        GetAllChildrenOppositeFace(faceNeighbor, face, neighbors);
    }
    return neighbors.size();
}

OctreeNode* OctreeNode::GetSon(
    OctreeNode* p,
    _NodeType::NodeType octant
)
{
    return p->m_children[octant];
}

sz OctreeNode::GetAllChildrenOppositeFace(
    OctreeNode* parent,
    NodeDir face,
    std::vector<OctreeNode*>& children
)
{
    if (parent->isLeaf())
    {
        if (!parent->isSolid())
        {
            children.push_back(parent);
        }
        return 0;
    }
    else
    {
        NodeType* childTypes = CommonFaceChildren[face];
        for (int i = 0; i < 4; i++)
        {
            GetAllChildrenOppositeFace(
                GetSon(parent, childTypes[i]),
                face,
                children
            );
        }
    }
    return children.size();
}

sz OctreeNode::queryNeighbors(std::vector<OctreeNode*>& neighbors)
{
    for (int i = 0; i < 6; i++)
    {
        GetAllFaceNeighbors(this, (NodeDir)i, neighbors);
    }
    return neighbors.size();
}
#pragma endregion

#pragma region NAVIGATION_HELPERS

f32 OctreeNode::nodeHeuristic(XMVECTOR end)
{
    XMVECTOR vec = m_bounds.getMinV();
    vec = XMVectorSubtract(vec, end);
    vec = XMVector3Length(vec);
    return XMVectorGetX(vec) + (getDivision() - MAX_TREE_DEPTH) * 5;
}

#pragma endregion
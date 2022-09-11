#pragma once

#include "AABB.h"
#include "GTypes.h"

#include <vector>
#include <set>
#include <unordered_map>

class MeshEntity;
class Mesh;
class Shader;

#define ADJ(i, o) (OctreeNode::AdjTable[(i)][(o)])
#define REFLECT(i, o) (OctreeNode::ReflTable[(i)][(o)])
#define COMMON_FACE(i, o) (OctreeNode::CommonFaceTable[(i)][(o)])
#define COMMON_EDGE(i, o) (OctreeNode::CommonEdgeTable[(i - _NodeDir::NodeDir::LDB)][(o)])

constexpr i32 MAX_TREE_DEPTH = 5;

#pragma warning(disable : 26812)
namespace _NodeDir // hackaroni and cheese pizza
{
    enum NodeDir
    {
        L = 0, R, D, U, B, F,
        LD, LU, LB, LF, RD, RU, RB, RF, DB, DF, UB, UF,
        LDB, LDF, LUB, LUF, RDB, RDF, RUB, RUF, NIL
    };
}

namespace _NodeType
{
    enum NodeType
    {
        LDB = 0,
        LDF,
        LUB,
        LUF,
        RDB,
        RDF,
        RUB,
        RUF,
        ROOT
    };
}
class OctreeNode
{
public:
    /***** STATIC FIELD GETTER/SETTERS *****/
    static bool    IsShowGreen();
    static void    SetShowGreen(bool showGreen);

    static int     GetShowSize();
    static void    SetShowSize(int showSize);

    /**** CTOR, DTOR *****/
    OctreeNode(
        OctreeNode* parent,                     // parent node
        AABB bounds,                            // bounds of this octant
        const std::vector<MeshEntity*>& meshes, // meshes to insert into this node and its children
        u8 division,                            // the depth of this node (0 being deepest)
        _NodeType::NodeType nodeType            // an ID representing which octant this node lies in
    );
    ~OctreeNode();  // deletes all children recursively

    /***** GETTERS/SETTERS *****/
    bool                isLeaf();
    bool                isSolid();
    int                 getDivision();
    OctreeNode*         getParent();
    OctreeNode**        getChildren();
    sz                  getSize();

    std::vector<OctreeNode*>& getNeighbors();
    _NodeType::NodeType getNodeType();
    AABB                getBounds();

    /***** TREE QUERIES *****/
    void query(
        AABB* aabb,                         // input region to search
        std::vector<MeshEntity*>& meshes,   // meshes vector to be filled with results
        std::set<MeshEntity*>& unique,      // keeps track of unique mesh entities for de-duping
        u64 type                            // ID of meshentity type (worldspawn, ambient light, etc.)
    );
    void query(
        XMFLOAT3 point,                     // input point
        std::vector<MeshEntity*>& meshes,   // meshes vector to be filled with results
        std::set<MeshEntity*>& unique,      // keeps track of unique mesh entities for de-duping
        u64 type                            // ID of meshentity type (worldspawn, ambient light, etc.)
    );
    OctreeNode* query(XMFLOAT3 point);      // gets the smallest octant that contains the point

    /***** DEBUG DRAWING *****/
    void draw();
    void draw(XMFLOAT4 color);

    /***** NAVIGATION *****/
    void generateNeighbors();
    bool findPath(OctreeNode* destination, std::unordered_map<OctreeNode*, OctreeNode*>& path);

private:
    /***** STATIC FIELDS *****/
    static bool s_showGreen;
    static int s_showSize;
    /***** ADJACENCY, FACE, REFL, ETC. TABLES *****/
    static u8 AdjTable[26][8];
    static _NodeType::NodeType CommonFaceChildren[6][4];
    static _NodeType::NodeType ReflTable[26][8];
    static _NodeDir::NodeDir CommonFaceTable[26][8];
    static _NodeDir::NodeDir CommonEdgeTable[8][8];

    /***** FIELDS *****/
    /***** LOGICAL FIELDS *****/
    bool                m_isLeaf;
    bool                m_isSolid;
    u8                  m_division;
    OctreeNode*         m_parent;
    _NodeType::NodeType m_nodeType;
    AABB                m_bounds;
    bool                m_allContained;
    OctreeNode*         m_children[8];
    std::vector<OctreeNode*> m_neighbors;

    /***** DRAW FIELDS *****/
    std::vector<MeshEntity*>    m_meshes;
    Shader*                     m_debugShader;
    Mesh*                       m_debugMesh;

    /***** INITIALIZATION METHODS *****/
    void initializeOctants(
        AABB(&octants)[8] // reference to array of 8 AABBs
    );

    bool divideInsertMeshes(
        const std::vector<MeshEntity*>& meshes,
        AABB(& octants)[8], // reference to array of 8 AABBs
        std::vector<MeshEntity*>(&addToChildren)[8] // reference to array of 8 vectors of MeshEntity*
    );
    
    void makeLeaf(
        const std::vector<MeshEntity*>& meshes
    );

    /***** NEIGHBOR HELPER METHODS *****/
    static OctreeNode* GetGtEqualFaceNeighbor(OctreeNode* p, _NodeDir::NodeDir face);
    static sz GetAllFaceNeighbors(OctreeNode* p, _NodeDir::NodeDir face, std::vector<OctreeNode*>& neighbors);
    static OctreeNode* GetEqualFaceNeighbor(OctreeNode* p, _NodeDir::NodeDir face);
    static OctreeNode* GetSon(OctreeNode* parent, _NodeType::NodeType octant);
    static sz GetAllChildrenOppositeFace(OctreeNode* parent, _NodeDir::NodeDir, std::vector<OctreeNode*>& children);
    sz queryNeighbors(std::vector<OctreeNode*>& neighbors);

    /***** PRIVATE NAVIGATION HELPERS *****/
    f32 nodeHeuristic(XMVECTOR end);
};


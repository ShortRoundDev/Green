#pragma once

#include "GameManager.h"

#include "DirectXMath.h"

#include <vector>

using namespace DirectX;

class NavMesh;
class Mesh;

class NavNode
{
public:
    static NavNode* Create(std::vector<XMFLOAT3> triangle, GameManager* game)
    {
        return nullptr;
    }

    NavNode(XMFLOAT3 spot):
        m_spot(spot)
    {

    }
    ~NavNode()
    {

    }

    void findNeighbors(NavMesh* mesh)
    {

    }
    void link(NavNode* node)
    {

    }

    XMFLOAT3 getSpot()
    {
        return m_spot;
    }

private:
    XMFLOAT3 m_spot;
    //std::vector<NavNode*> neighbors;
};


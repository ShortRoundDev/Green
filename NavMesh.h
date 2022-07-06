#pragma once

#include "GameManager.h"
#include "NavNode.h"

#include <vector>

class Scene;

class NavMesh
{
public:
    NavMesh()
    {

    }
    //NavMesh(Scene* scene, GameManager* game);
    ~NavMesh()
    {

    }

    void addNode(XMFLOAT3 pos)
    {
        m_root.push_back(new NavNode(pos));
    }
    
    const std::vector<NavNode*>& getRoot()
    {
        return m_root;
    }

private:
    std::vector<NavNode*> m_root;

};


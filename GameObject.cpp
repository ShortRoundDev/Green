#include "GameObject.h"

#include "GameManager.h"

static u32 ObjIdCount;

GameObject::GameObject(XMFLOAT3 pos, u64 typeId) :
    GameObject(pos, typeId, &Game)
{
}

GameObject::GameObject(XMFLOAT3 pos, u64 typeId, GameManager* gameManager) :
    m_pos(pos),
    m_id(ObjIdCount++),
    m_typeId(typeId)
{

}

GameObject::~GameObject()
{

}

void GameObject::update()
{
    
}

void GameObject::draw()
{

}

u32 GameObject::getId()
{
    return m_id;
}

u64 GameObject::getType()
{
    return m_typeId;
}

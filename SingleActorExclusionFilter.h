#pragma once
#include "PxPhysicsAPI.h"

using namespace physx;

class SingleActorExclusionFilter : public PxQueryFilterCallback
{
public:
    SingleActorExclusionFilter(PxRigidActor* exclude):
        m_exclude(exclude)
    {        
    }
    virtual PxQueryHitType::Enum preFilter(
        const PxFilterData& filterData,
        const PxShape* shape,
        const PxRigidActor* actor,
        PxHitFlags& queryFlags
    )
    {
        if (actor == m_exclude)
        {
            return PxQueryHitType::eNONE;
        }
        return PxQueryHitType::eTOUCH;
    }

    virtual PxQueryHitType::Enum postFilter(const PxFilterData& filterData, const PxQueryHit& hit)
    {
        return PxQueryHitType::eTOUCH;
    }

private:
    PxRigidActor* m_exclude;
};


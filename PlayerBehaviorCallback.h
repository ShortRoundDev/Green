#pragma once

#include <PxPhysicsAPI.h>

using namespace physx;

class PlayerBehaviorCallback : public PxControllerBehaviorCallback
{
public:
    PlayerBehaviorCallback();
    virtual ~PlayerBehaviorCallback();

    virtual PxControllerBehaviorFlags getBehaviorFlags(const PxShape& shape, const PxActor& actor);
    virtual PxControllerBehaviorFlags getBehaviorFlags(const PxController& controller);
    virtual PxControllerBehaviorFlags getBehaviorFlags(const PxObstacle& obstacle);
private:
    
};


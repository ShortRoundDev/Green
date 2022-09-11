#include "PlayerBehaviorCallback.h"

#include "Logger.h"

static ::Logger logger = CreateLogger("PlayerBehaviorCallback");

PlayerBehaviorCallback::PlayerBehaviorCallback()
{

}

PlayerBehaviorCallback::~PlayerBehaviorCallback()
{

}

PxControllerBehaviorFlags PlayerBehaviorCallback::getBehaviorFlags(const PxShape& shape, const PxActor& actor)
{
//    logger.info("Shape, Actor");
    PxControllerBehaviorFlags flags;
    flags.set(PxControllerBehaviorFlag::eCCT_SLIDE);
    return flags;
}

PxControllerBehaviorFlags PlayerBehaviorCallback::getBehaviorFlags(const PxController& controller)
{
    logger.info("Controller");
    PxControllerBehaviorFlags flags;
    flags.set(PxControllerBehaviorFlag::eCCT_SLIDE);
    return flags;
}

PxControllerBehaviorFlags PlayerBehaviorCallback::getBehaviorFlags(const PxObstacle& obstacle)
{
    logger.info("obstacle");
    PxControllerBehaviorFlags flags;
    flags.set(PxControllerBehaviorFlag::eCCT_SLIDE);
    return flags;
}
#include "GVertex.h"


bool operator == (const GVertex& a, const GVertex& b)
{
    return
            a.pos.x     == b.pos.x      &&
            a.pos.y     == b.pos.y      &&
            a.pos.z     == b.pos.z      &&
            a.normal.x  == b.normal.x   &&
            a.normal.y  == b.normal.y   &&
            a.normal.z  == b.normal.z   &&
            a.tex.x     == b.tex.x      &&
            a.tex.y     == b.tex.y      &&
            a.bones.x   == b.bones.x    &&
            a.bones.y   == b.bones.y    &&
            a.bones.z   == b.bones.z    &&
            a.bones.w   == b.bones.w    &&
            a.weights.x == b.weights.x  &&
            a.weights.y == b.weights.y  &&
            a.weights.z == b.weights.z  &&
            a.weights.w == b.weights.w;
}
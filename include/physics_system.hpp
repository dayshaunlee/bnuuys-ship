#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem {
   public:
    void step(float elapsed_ms);
    static bool collidesPolyVec(Entity island_entity, ivec2 node_pos);

    PhysicsSystem() {}
};

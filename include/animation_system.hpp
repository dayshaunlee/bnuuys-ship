#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

// A simple animation system.
class AnimationSystem {
   public:
    void step(float elapsed_ms);
    AnimationSystem() {}
};

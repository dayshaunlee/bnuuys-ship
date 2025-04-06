#pragma once

#include <random>
#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

class ParticleSystem {
public:
    void step(float elapsed_ms);
    void Emit(ParticleEmitter& emitter, float dt);

    ParticleSystem();

private:
    std::random_device rd;
    std::mt19937 generator;
    std::uniform_real_distribution<float> float_distribution;
};

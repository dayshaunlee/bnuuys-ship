#include "particle_system.hpp"
#include <cmath>
#include "tinyECS/registry.hpp"

void ParticleSystem::Emit(ParticleEmitter& emitter, float dt) {
    Particle& particle = emitter.particles[emitter.poolIndex];
    particle.Active = true;
    particle.Position = emitter.props.Position;
	particle.Rotation = M_PI * 2 * float_distribution(generator);

	// Velocity
	particle.Velocity = emitter.props.Velocity;
	particle.Velocity.x += emitter.props.VelocityVariation.x * (float_distribution(generator) - 0.5f);
	particle.Velocity.y += emitter.props.VelocityVariation.y * (float_distribution(generator) - 0.5f);

	// Color
	particle.ColorBegin = emitter.props.ColorBegin;
	particle.ColorEnd = emitter.props.ColorEnd;

	particle.LifeTime = emitter.props.LifeTime;
	particle.LifeRemaining = emitter.props.LifeTime;
	particle.SizeBegin = emitter.props.SizeBegin + emitter.props.SizeVariation * (float_distribution(generator) - 0.5f);
	particle.SizeEnd = emitter.props.SizeEnd;

	emitter.poolIndex = --emitter.poolIndex % emitter.particles.size();
}

void ParticleSystem::step(float dt) {
    for (Entity particle_emitter_entity : registry.particleEmitters.entities) {
        ParticleEmitter& particle_emitter = registry.particleEmitters.get(particle_emitter_entity);
        if (registry.motions.has(particle_emitter_entity)) {
            particle_emitter.props.Position = registry.motions.get(particle_emitter_entity).position;
        }

        for (Particle& particle : particle_emitter.particles) {
            if (!particle.Active || particle.LifeRemaining <= 0.0f) {
                particle.Active = false;
                continue;
            }
            particle.LifeRemaining -= dt;
            particle.Position += particle.Velocity * dt / 1000.0f;
            particle.Rotation += 0.01 * dt / 1000.0f;
        }

        // For now we can just always emit a particle.
        if (time <= 0) {
            Emit(particle_emitter, dt);
            time = 20.0f;
        } else {
            time -= dt;
        }
    }
}

ParticleSystem::ParticleSystem(): generator(rd()), float_distribution(0, 1.0f) {}

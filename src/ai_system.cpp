#include "ai_system.hpp"

#include <iostream>

#include "world_init.hpp"

void AISystem::step(float elapsed_ms) {
    (void) elapsed_ms;
    for (const Entity& player : registry.players.entities) {
        Motion& player_motion = registry.motions.get(player);
        vec2& player_position = player_motion.position;

        for (const Entity& enemy : registry.enemies.entities) {
            Enemy& enemy_comp = registry.enemies.get(enemy);
            Motion& enemy_motion = registry.motions.get(enemy);
            vec2& enemy_position = enemy_motion.position;

            if (enemy_position != player_position) {
                vec2 direction = player_position - enemy_position;

                // unit vector for direction
                float length = sqrt(direction.x * direction.x + direction.y * direction.y);
                direction.x /= length;
                direction.y /= length;

                enemy_motion.velocity = direction * ENEMY_BASE_SPEED;

                if (enemy_motion.velocity.x < 0) {
                    vec2 flip = {-1 ,1};
                    enemy_motion.scale *= flip;
                }
            }
        };
    }

}

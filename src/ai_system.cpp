#include "ai_system.hpp"
#include "camera_system.hpp"
#include <iostream>

#include "world_init.hpp"

void AISystem::step(float elapsed_ms) {
    (void) elapsed_ms;
    for (const Entity& ship : registry.ships.entities) {
        Motion& ship_motion = registry.motions.get(ship);
        vec2& ship_position = ship_motion.position;

        // Simple enemy path finding (no algorithm implemented yet)
        for (const Entity& enemy : registry.enemies.entities) {
            Motion& enemy_motion = registry.motions.get(enemy);
            vec2 enemy_position = enemy_motion.position + CameraSystem::GetInstance()->position;   // Clare's note: camera offset calculation
            if (enemy_position != ship_position) {
                vec2 direction = ship_position - enemy_position;

                // unit vector for direction
                float length = sqrt(direction.x * direction.x + direction.y * direction.y);
                if (length > 0) {
                    direction.x /= length;
                    direction.y /= length;
                }

                enemy_motion.velocity = direction * (float) registry.enemies.get(enemy).speed;

                if (enemy_motion.velocity.x < 0) {
                    vec2 flip = {-1, 1};
                    enemy_motion.scale *= flip;
                }
            }
        };
    }
}

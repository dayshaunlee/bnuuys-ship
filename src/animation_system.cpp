// internal
#include "animation_system.hpp"
#include <ios>
#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "iostream"
#include "camera_system.hpp"

bool isIdleAnim(TEXTURE_ASSET_ID currAnim) {
    return (currAnim == TEXTURE_ASSET_ID::BUNNY_IDLE_UP0) || (currAnim == TEXTURE_ASSET_ID::BUNNY_IDLE_UP1) ||
           (currAnim == TEXTURE_ASSET_ID::BUNNY_IDLE_DOWN0) || (currAnim == TEXTURE_ASSET_ID::BUNNY_IDLE_DOWN1) ||
           (currAnim == TEXTURE_ASSET_ID::BUNNY_IDLE_LEFT0) || (currAnim == TEXTURE_ASSET_ID::BUNNY_IDLE_LEFT1) ||
           (currAnim == TEXTURE_ASSET_ID::BUNNY_IDLE_RIGHT0) || (currAnim == TEXTURE_ASSET_ID::BUNNY_IDLE_RIGHT1);
}

bool isWalkAnim(TEXTURE_ASSET_ID currAnim) {
    return (currAnim == TEXTURE_ASSET_ID::BUNNY_UP_WALK0) || (currAnim == TEXTURE_ASSET_ID::BUNNY_UP_WALK1) ||
           (currAnim == TEXTURE_ASSET_ID::BUNNY_RIGHT_WALK0) || (currAnim == TEXTURE_ASSET_ID::BUNNY_RIGHT_WALK1) ||
           (currAnim == TEXTURE_ASSET_ID::BUNNY_DOWN_WALK0) || (currAnim == TEXTURE_ASSET_ID::BUNNY_DOWN_WALK1) ||
           (currAnim == TEXTURE_ASSET_ID::BUNNY_LEFT_WALK0) || (currAnim == TEXTURE_ASSET_ID::BUNNY_LEFT_WALK1);
}

DIRECTION getAnimDirection(TEXTURE_ASSET_ID currAnim) {
    switch (currAnim) {
        case TEXTURE_ASSET_ID::BUNNY_IDLE_UP0:
        case TEXTURE_ASSET_ID::BUNNY_IDLE_UP1:
        case TEXTURE_ASSET_ID::BUNNY_UP_WALK0:
        case TEXTURE_ASSET_ID::BUNNY_UP_WALK1:
            return UP;

        case TEXTURE_ASSET_ID::BUNNY_IDLE_RIGHT0:
        case TEXTURE_ASSET_ID::BUNNY_IDLE_RIGHT1:
        case TEXTURE_ASSET_ID::BUNNY_RIGHT_WALK0:
        case TEXTURE_ASSET_ID::BUNNY_RIGHT_WALK1:
            return RIGHT;

        case TEXTURE_ASSET_ID::BUNNY_IDLE_DOWN0:
        case TEXTURE_ASSET_ID::BUNNY_IDLE_DOWN1:
        case TEXTURE_ASSET_ID::BUNNY_DOWN_WALK0:
        case TEXTURE_ASSET_ID::BUNNY_DOWN_WALK1:
            return DOWN;

        case TEXTURE_ASSET_ID::BUNNY_IDLE_LEFT0:
        case TEXTURE_ASSET_ID::BUNNY_IDLE_LEFT1:
        case TEXTURE_ASSET_ID::BUNNY_LEFT_WALK0:
        case TEXTURE_ASSET_ID::BUNNY_LEFT_WALK1:
            return LEFT;
        default:
            return DOWN;
    }
}

TEXTURE_ASSET_ID GetFirstPlayerAnimation(Player player) {
    if (player.player_state == IDLE) {
        switch (player.direction) {
            case UP:
                return TEXTURE_ASSET_ID::BUNNY_IDLE_UP0;
            case RIGHT:
                return TEXTURE_ASSET_ID::BUNNY_IDLE_RIGHT0;
            case DOWN:
                return TEXTURE_ASSET_ID::BUNNY_IDLE_DOWN0;
            case LEFT:
                return TEXTURE_ASSET_ID::BUNNY_IDLE_LEFT0;
        }
    } else if (player.player_state == WALKING) {
        switch (player.direction) {
            case UP:
                return TEXTURE_ASSET_ID::BUNNY_UP_WALK0;
            case RIGHT:
                return TEXTURE_ASSET_ID::BUNNY_RIGHT_WALK0;
            case DOWN:
                return TEXTURE_ASSET_ID::BUNNY_DOWN_WALK0;
            case LEFT:
                return TEXTURE_ASSET_ID::BUNNY_LEFT_WALK0;
        }
    }
    return TEXTURE_ASSET_ID::BUNNY_IDLE_DOWN0;
}

// Get Next Player Animation.
TEXTURE_ASSET_ID GetNextPlayerAnimation(TEXTURE_ASSET_ID currAnim, Player player) {
    // Handle Idle Animation.
    if (player.player_state == PLAYERSTATE::IDLE) {
        // For Facing Down.
        if (currAnim == TEXTURE_ASSET_ID::BUNNY_IDLE_DOWN0)
            return TEXTURE_ASSET_ID::BUNNY_IDLE_DOWN1;
        else if (currAnim == TEXTURE_ASSET_ID::BUNNY_IDLE_DOWN1)
            return TEXTURE_ASSET_ID::BUNNY_IDLE_DOWN0;

        // For Facing Up.
        else if (currAnim == TEXTURE_ASSET_ID::BUNNY_IDLE_UP0)
            return TEXTURE_ASSET_ID::BUNNY_IDLE_UP1;
        else if (currAnim == TEXTURE_ASSET_ID::BUNNY_IDLE_UP1)
            return TEXTURE_ASSET_ID::BUNNY_IDLE_UP0;

        // For Facing Right.
        else if (currAnim == TEXTURE_ASSET_ID::BUNNY_IDLE_RIGHT0)
            return TEXTURE_ASSET_ID::BUNNY_IDLE_RIGHT1;
        else if (currAnim == TEXTURE_ASSET_ID::BUNNY_IDLE_RIGHT1)
            return TEXTURE_ASSET_ID::BUNNY_IDLE_RIGHT0;

        // For Facing Left.
        else if (currAnim == TEXTURE_ASSET_ID::BUNNY_IDLE_LEFT0)
            return TEXTURE_ASSET_ID::BUNNY_IDLE_LEFT1;
        else if (currAnim == TEXTURE_ASSET_ID::BUNNY_IDLE_LEFT1)
            return TEXTURE_ASSET_ID::BUNNY_IDLE_LEFT0;
    }

    // Handle Walking Animation.
    if (player.player_state == PLAYERSTATE::WALKING) {
        // For Facing Down.
        if (currAnim == TEXTURE_ASSET_ID::BUNNY_DOWN_WALK0)
            return TEXTURE_ASSET_ID::BUNNY_DOWN_WALK1;
        else if (currAnim == TEXTURE_ASSET_ID::BUNNY_DOWN_WALK1)
            return TEXTURE_ASSET_ID::BUNNY_DOWN_WALK0;

        // For Facing Up.
        else if (currAnim == TEXTURE_ASSET_ID::BUNNY_UP_WALK0)
            return TEXTURE_ASSET_ID::BUNNY_UP_WALK1;
        else if (currAnim == TEXTURE_ASSET_ID::BUNNY_UP_WALK1)
            return TEXTURE_ASSET_ID::BUNNY_UP_WALK0;

        // For Facing Right.
        else if (currAnim == TEXTURE_ASSET_ID::BUNNY_RIGHT_WALK0)
            return TEXTURE_ASSET_ID::BUNNY_RIGHT_WALK1;
        else if (currAnim == TEXTURE_ASSET_ID::BUNNY_RIGHT_WALK1)
            return TEXTURE_ASSET_ID::BUNNY_RIGHT_WALK0;

        // For Facing Left.
        else if (currAnim == TEXTURE_ASSET_ID::BUNNY_LEFT_WALK0)
            return TEXTURE_ASSET_ID::BUNNY_LEFT_WALK1;
        else if (currAnim == TEXTURE_ASSET_ID::BUNNY_LEFT_WALK1)
            return TEXTURE_ASSET_ID::BUNNY_LEFT_WALK0;
    }

    return TEXTURE_ASSET_ID::BUNNY_IDLE_DOWN0;
}

/*
*   M1 interpolation implementation.
*   Very basic linear interpolation to switch player keyframes/textures for animation.
*/
void HandlePlayerAnimation(float elapsed_ms) {
    for (Entity player : registry.playerAnimations.entities) {
        Player player_comp = registry.players.get(player);
        PlayerAnimation& player_anim = registry.playerAnimations.get(player);

        auto currAnim = player_anim.curr_anim;
        if ((player_comp.player_state == IDLE && !isIdleAnim(currAnim)) ||
            (player_comp.player_state == WALKING && !isWalkAnim(currAnim)) ||
            (player_comp.direction != getAnimDirection(currAnim))) {
            // Update their animation.
            TEXTURE_ASSET_ID newAnim = GetFirstPlayerAnimation(player_comp);
            RenderRequest& rr = registry.renderRequests.get(player);
            rr.used_texture = newAnim;
            player_anim.curr_anim = newAnim;
            player_anim.timer_ms = ANIMATION_TIME;
            return;
        }

        // Change animation for idle.
        if (player_anim.timer_ms <= 0) {
            // Update their animation.
            TEXTURE_ASSET_ID newAnim = GetNextPlayerAnimation(currAnim, player_comp);
            RenderRequest& rr = registry.renderRequests.get(player);
            rr.used_texture = newAnim;
            player_anim.curr_anim = newAnim;
            player_anim.timer_ms = ANIMATION_TIME;
        } else {
            player_anim.timer_ms -= elapsed_ms;
        }
    }
}

void HandleBunnyAnimation(float elapsed_ms) {
    Base& base = registry.base.components[0];
    for (Entity entity : registry.bunnies.entities) {
        Bunny& bunny = registry.bunnies.get(entity);
        TEXTURE_ASSET_ID& texture = registry.renderRequests.get(entity).used_texture;
        if (bunny.is_jailed) {
            if (bunny.timer_ms <= 0) {
                bunny.timer_ms = 1 * ANIMATION_TIME;
                texture = TEXTURE_ASSET_ID::BUNNY_NPC_JAILED0;
            } else if (bunny.timer_ms <= 0.5 * ANIMATION_TIME) {
                texture = TEXTURE_ASSET_ID::BUNNY_NPC_JAILED1;
            }
        } else {
            if (bunny.timer_ms <= 0) {
                bunny.timer_ms = 2 * ANIMATION_TIME;
                texture = TEXTURE_ASSET_ID::BUNNY_NPC_IDLE_UP0;
            } else if (bunny.timer_ms <= 1 * ANIMATION_TIME) {
                texture = TEXTURE_ASSET_ID::BUNNY_NPC_IDLE_UP1;
            }
        }
        bunny.timer_ms -= elapsed_ms;

        // Bunny saving animation.
        Motion& bunny_motion = registry.motions.get(entity);
        
        // bunny -> ship
        if (!bunny.is_jailed && !bunny.on_ship && !bunny.on_base && !bunny.moving_to_base && !bunny.on_module) {
            vec2 bunny_position = bunny_motion.position + CameraSystem::GetInstance()->position;
            vec2 empty_ship_location = {364, 252};  // save bunny to top right ship tile
            if (round(bunny_position) != empty_ship_location) {
                vec2 direction = empty_ship_location - bunny_position;

                // unit vector for direction
                float length = sqrt(direction.x * direction.x + direction.y * direction.y);
                direction.x /= length;
                direction.y /= length;

                bunny_motion.velocity = direction * 150.f;
            } else {
                bunny.on_island = false;
                bunny.on_ship = true;
                bunny_motion.position += CameraSystem::GetInstance()->position;
                bunny_motion.velocity = {0, 0};
                registry.backgroundObjects.remove(entity);
            }
        }
        
        // bunny -> base
        vec2& bunny_position = bunny_motion.position;
        Entity& base_entity = registry.base.entities[0];
        vec2 empty_base_location = registry.motions.get(base_entity).position - (registry.motions.get(base_entity).scale / vec2(2, 2));
        // the bunny house is (2.5, 3) tiles from the top left corner of the base
        empty_base_location += vec2(GRID_CELL_WIDTH_PX * 2.5, GRID_CELL_HEIGHT_PX * 3);

        // set bunny flags if it should start moving to the base from the ship
        if (!bunny.is_jailed && bunny.on_ship && !bunny.moving_to_base &&
            base.drop_off_timer >= BUNNY_BASE_DROPOFF_TIME && !base.locked) {
            // the bunny should not be a background object while its on the ship before it started moving
            assert(!registry.backgroundObjects.has(entity));
            registry.backgroundObjects.emplace(entity);
            bunny.on_ship = false;
            bunny.moving_to_base = true;
            bunny_position -= CameraSystem::GetInstance()->position;
        }

        if (bunny.moving_to_base) {
            if (round(bunny_position) != round(empty_base_location)) {
                vec2 direction = empty_base_location - bunny_position;

                // unit vector for direction
                float length = sqrt(direction.x * direction.x + direction.y * direction.y);
                if (length > 0) {
                    direction.x /= length;
                    direction.y /= length;
                }

                bunny_motion.velocity = direction * 100.f;
            } else {
                std::cout << "bunny docked on base" << std::endl;
                bunny.moving_to_base = false;
                bunny.on_base = true;
                base.bunny_count += 1;
                bunny_motion.velocity = {0, 0};
                std::cout << "bunny: " << bunny_position.x << ", " << bunny_position.y << std::endl;
                // remove bunny to make it look like it disappeared into the house
                // also saves computing resources
                registry.remove_all_components_of(entity);
            }
        }
    }
}

void HandleEnemyAnimation(float elapsed_ms) {
    for (Entity entity : registry.enemies.entities) {
        Enemy& enemy = registry.enemies.get(entity);
        TEXTURE_ASSET_ID& texture = registry.renderRequests.get(entity).used_texture;
        if (enemy.type == ENEMY_TYPE::BASIC_GUNNER) {
            if (enemy.timer_ms <= 0) {
                enemy.timer_ms = 1 * ANIMATION_TIME;
                texture = TEXTURE_ASSET_ID::CHICKEN_BOAT0;
            } else if (enemy.timer_ms <= 0.5 * ANIMATION_TIME) {
                texture = TEXTURE_ASSET_ID::CHICKEN_BOAT1;
            }
        } else if (enemy.type == ENEMY_TYPE::FLYER) {
            if (enemy.timer_ms <= 0) {
                enemy.timer_ms = 1 * ANIMATION_TIME;
                texture = TEXTURE_ASSET_ID::BALLOON0;
            } else if (enemy.timer_ms <= 0.5 * ANIMATION_TIME) {
                texture = TEXTURE_ASSET_ID::BALLOON1;
            }
        } else if (enemy.type == ENEMY_TYPE::TANK) {    // TODO: find new tank sprites
            if (enemy.timer_ms <= 0) {
                enemy.timer_ms = 1 * ANIMATION_TIME;
                texture = TEXTURE_ASSET_ID::BALLOON0;
            } else if (enemy.timer_ms <= 0.5 * ANIMATION_TIME) {
                texture = TEXTURE_ASSET_ID::BALLOON1;
            }
        } else if (enemy.type == ENEMY_TYPE::SHOOTER) {
            enemy.cooldown_ms -= elapsed_ms;
            if (enemy.timer_ms <= 0) {
                enemy.timer_ms = 1.5 * ANIMATION_TIME;
                texture = TEXTURE_ASSET_ID::COW0;
            } else if (enemy.timer_ms <= 0.5 * ANIMATION_TIME) {
                texture = TEXTURE_ASSET_ID::COW1;
            } else if (enemy.timer_ms <= 1 * ANIMATION_TIME) {
                texture = TEXTURE_ASSET_ID::COW2;
            }
        } else if (enemy.type == ENEMY_TYPE::DUMMY) {
            continue;
        }

        enemy.timer_ms -= elapsed_ms;
    }
}

void HandleDisasterAnimation(float elapsed_ms) {
    for (Entity entity : registry.disasters.entities) {
        Disaster& disaster = registry.disasters.get(entity);
        TEXTURE_ASSET_ID& texture = registry.renderRequests.get(entity).used_texture;
        if (disaster.type == DISASTER_TYPE::TORNADO) {
            if (disaster.timer_ms <= 0) {
                disaster.timer_ms = 2 * ANIMATION_TIME;
                texture = TEXTURE_ASSET_ID::TORNADO0;
            } else if (disaster.timer_ms <= 0.5 * ANIMATION_TIME) {
                texture = TEXTURE_ASSET_ID::TORNADO1;
            } else if (disaster.timer_ms <= 1 * ANIMATION_TIME) {
                texture = TEXTURE_ASSET_ID::TORNADO2;
            } else if (disaster.timer_ms <= 1.5 * ANIMATION_TIME) {
                texture = TEXTURE_ASSET_ID::TORNADO1;
            }
        } else if (disaster.type == DISASTER_TYPE::WHIRLPOOL) {
            if (disaster.timer_ms <= 0) {
                disaster.timer_ms = 2 * ANIMATION_TIME;
                texture = TEXTURE_ASSET_ID::WHIRLPOOL0;
            } else if (disaster.timer_ms <= 0.5 * ANIMATION_TIME) {
                texture = TEXTURE_ASSET_ID::WHIRLPOOL1;
            } else if (disaster.timer_ms <= 1 * ANIMATION_TIME) {
                texture = TEXTURE_ASSET_ID::WHIRLPOOL2;
            } else if (disaster.timer_ms <= 1.5 * ANIMATION_TIME) {
                texture = TEXTURE_ASSET_ID::WHIRLPOOL1;
            }
        }

        disaster.timer_ms -= elapsed_ms;
    }
}

void AnimationSystem::step(float elapsed_ms) {
    // Move each entity that has motion.
    HandlePlayerAnimation(elapsed_ms);
    HandleBunnyAnimation(elapsed_ms);
    HandleEnemyAnimation(elapsed_ms);
    HandleDisasterAnimation(elapsed_ms);
}

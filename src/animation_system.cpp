// internal
#include "animation_system.hpp"
#include <ios>
#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "iostream"

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
    for (Entity entity : registry.bunnies.entities) {
        Bunny& bunny = registry.bunnies.get(entity);
        // Bunny saving animation.
        Motion& bunny_motion = registry.motions.get(entity);
        if (!bunny.is_jailed && !bunny.on_ship) {
            vec2& bunny_position = bunny_motion.position;
            vec2 empty_ship_location = {364, 252};      // hard coded to one specific tile

            if (round(bunny_position) != empty_ship_location) {
                vec2 direction = empty_ship_location - bunny_position;

                // unit vector for direction
                float length = sqrt(direction.x * direction.x + direction.y * direction.y);
                direction.x /= length;
                direction.y /= length;

                bunny_motion.velocity = direction * 100.f;
            } else {
                bunny.on_island = false;
                bunny.on_ship = true;
                bunny_motion.velocity = {0, 0};
            }
        } 
    }
}

void AnimationSystem::step(float elapsed_ms) {
    // Move each entity that has motion.
    HandlePlayerAnimation(elapsed_ms);
    HandleBunnyAnimation(elapsed_ms);
}

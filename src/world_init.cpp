#include "world_init.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

Entity createPlayer(vec2 position) {
    Entity player;

    Player comp_player;
    comp_player.name = "Player 1";
    comp_player.direction = DIRECTION::DOWN;
    comp_player.player_state = PLAYERSTATE::IDLE;
    registry.players.emplace(player, comp_player);

    Motion comp_motion;
    comp_motion.angle = 0.0f;
    comp_motion.scale = {120, 120};
    comp_motion.position = position;
    registry.motions.emplace(player, comp_motion);

    RenderRequest comp_render_request = {
        TEXTURE_ASSET_ID::BUNNY_IDLE_DOWN0, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE};
    registry.renderRequests.emplace(player, comp_render_request);

    PlayerAnimation comp_anim;
    comp_anim.timer_ms = 250;
    comp_anim.curr_anim = TEXTURE_ASSET_ID::BUNNY_IDLE_DOWN0;
    registry.playerAnimations.emplace(player, comp_anim);

    return player;
}

Entity createPlayer(RenderSystem* renderer, vec2 position) {
    Entity player;

    Player comp_player;
    comp_player.name = "Player 1";
    comp_player.direction = DIRECTION::DOWN;
    comp_player.player_state = PLAYERSTATE::IDLE;
    registry.players.emplace(player, comp_player);

    Motion comp_motion;
    comp_motion.angle = 0.0f;
    comp_motion.scale = {120, 120};
    comp_motion.position = position;
    registry.motions.emplace(player, comp_motion);

    RenderRequest comp_render_request = {
        TEXTURE_ASSET_ID::BUNNY_IDLE_DOWN0, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE};
    registry.renderRequests.emplace(player, comp_render_request);

    PlayerAnimation comp_anim;
    comp_anim.timer_ms = 250;
    comp_anim.curr_anim = TEXTURE_ASSET_ID::BUNNY_IDLE_DOWN0;
    registry.playerAnimations.emplace(player, comp_anim);

    return player;
}

#include "world_init.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

Entity createPlayer(RenderSystem* renderer, vec2 position) {
    Entity player;

    Player comp_player;
    comp_player.is_sailing_ship = false;
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

Entity setupCamera(RenderSystem* renderer, vec2 ship_position) {
    // loop through each backgroundObjects
    Entity game_camera = Entity();
    for (Entity i : registry.backgroundObjects.entities) {
    }

    return game_camera;
}

Entity createWaterBackground() {
    // create the water entity
    Entity waterbg = Entity();
    registry.backgroundObjects.emplace(waterbg);
    Motion& waterMotion = registry.motions.emplace(waterbg);

    waterMotion.position.x = WINDOW_WIDTH_PX / 2;
    waterMotion.position.y = WINDOW_HEIGHT_PX / 2;
    waterMotion.scale.x = WINDOW_WIDTH_PX * 1.5f;
    waterMotion.scale.y = WINDOW_HEIGHT_PX * 1.5f;

    registry.renderRequests.insert(
        waterbg, {TEXTURE_ASSET_ID::WATER_BACKGROUND, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});
    return waterbg;
}

Entity createShip() {
    Entity ship = Entity();
    Motion& shipMotion = registry.motions.emplace(ship);
    // registry.collisions.emplace(ship);

    // need to add a componet for ship like dieable or something
    shipMotion.position.x = WINDOW_WIDTH_PX / 2;
    shipMotion.position.y = WINDOW_HEIGHT_PX / 2;
    shipMotion.scale.x = 56 * 3;  // the temporary grid height and width is 56
    shipMotion.scale.y = 56 * 3;

    registry.renderRequests.insert(
        ship, {TEXTURE_ASSET_ID::TEXTURE_COUNT, EFFECT_ASSET_ID::EGG, GEOMETRY_BUFFER_ID::SHIP_SQUARE});

    return ship;
}

Entity createGridLine(vec2 start_pos, vec2 end_pos) {
    Entity entity = Entity();

    GridLine& gridLine = registry.gridLines.emplace(entity);
    gridLine.start_pos = start_pos;
    gridLine.end_pos = end_pos;
    registry.renderRequests.insert(
        entity, {TEXTURE_ASSET_ID::TEXTURE_COUNT, EFFECT_ASSET_ID::EGG, GEOMETRY_BUFFER_ID::DEBUG_LINE});
    registry.colors.insert(entity, vec3(0.8f, 0.8f, 0.8f));
    return entity;
}
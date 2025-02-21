#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "render_system.hpp"

// Player
Entity createPlayer(RenderSystem* renderer, vec2 position);
Entity createPlayer(vec2 position);
Entity createCannonProjectile(vec2 orig, vec2 dest);
Entity createSteeringWheel(vec2 tile_pos);
Entity createCannon(vec2 tile_pos);

Entity createWaterBackground();

Entity createShip();
Entity createCamera();
Entity createGridLine(vec2 start_pos, vec2 end_pos);

#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "render_system.hpp"

// Player
Entity createPlayer(RenderSystem* renderer, vec2 position);

Entity setupCamera(RenderSystem* renderer, vec2 shipPosiiton);

Entity createWaterBackground();

Entity createShip();

Entity createGridLine(vec2 start_pos, vec2 end_pos);
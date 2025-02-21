#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "render_system.hpp"

// Player
Entity createPlayer(RenderSystem* renderer, vec2 position);

Entity setupCamera(RenderSystem* renderer, vec2 shipPosiiton);

Entity createWaterBackground();

Entity createIslandBackground(int width, int height);

Entity createShip();

Entity createGridLine(vec2 start_pos, vec2 end_pos);

// Enemy
Entity createEnemy(RenderSystem* renderer, vec2 position);
// Obstacle for enemy path finding testing
Entity createObstacle(RenderSystem* renderer, vec2 position);
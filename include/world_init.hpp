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

Entity createIslandBackground(int width, int height);

Entity createShip();
Entity createCamera();
Entity createGridLine(vec2 start_pos, vec2 end_pos);

// Enemy
Entity createEnemy(RenderSystem* renderer, vec2 position);
Entity createEnemy(vec2 position);

// Obstacle for enemy path finding testing
Entity createObstacle(RenderSystem* renderer, vec2 position);

//Bunny
Entity createBunny(RenderSystem* renderer, vec2 position);
Entity createBunny(vec2 position);
// Obstacle for enemy path finding testing
Entity createObstacle(RenderSystem* renderer, vec2 position);

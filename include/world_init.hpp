#pragma once

#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "render_system.hpp"
#include "vector"
#include <SDL_mixer.h>

// Enemy helpers
int getEnemyHealth(ENEMY_TYPE type);
float getEnemySpeed(ENEMY_TYPE type);
int getEnemyRange(ENEMY_TYPE type);

// Player
Entity createPlayer(RenderSystem* renderer, vec2 position);
Entity createPlayer(vec2 position);
Entity renderPlayer(Entity player);
Entity createCannonProjectile(vec2 orig, vec2 dest);
Entity createModifiedCannonProjectile(vec2 orig, vec2 dest, CannonModifier cm);
Entity createEnemyProjectile(vec2 orig, vec2 dest);
Entity createSteeringWheel(vec2 tile_pos);
Entity createCannon(vec2 tile_pos);
Entity modifyCannon(Entity entity, MODIFIER_TYPE type);
Entity createLaserWeapon(vec2 tile_pos);
std::vector<Entity> createLaserBeam(vec2 orig, vec2 dest);
Entity createHealModule(vec2 tile_pos);


Entity createWaterBackground();
Entity createIslandBackground(int width, int height, int offset_x, int offset_y, TEXTURE_ASSET_ID texture);

Entity createShip();
Entity expandShip();
Entity createCamera();
Entity createGridLine(vec2 start_pos, vec2 end_pos);
Entity createOverlay(float alpha, vec3 color = vec3(0));

std::vector<tson::Vector2i> get_poly_from_motion(const Motion& motion);
std::vector<Entity> createBaseProgressLines(Entity base_entity);

// Enemy
Entity createEnemy(vec2 position);
Entity createEnemy(Entity entity);

// Bunny
Entity createBunny(RenderSystem* renderer, vec2 position);
Entity createBunny(Entity bunny);

Entity createBunnyIcon(vec2 tile_pos);

// filled tile for enemy path 
Entity createFilledTile(vec2 position, vec2 size);

// Tornado
Entity createDisaster(Entity entity);

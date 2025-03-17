#include "world_init.hpp"
#include <cmath>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <ostream>
#include <vector>
#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/entity.hpp"
#include "tinyECS/registry.hpp"
#include <random>

ENEMY_TYPE getRandEnemyType() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 3);
    return static_cast<ENEMY_TYPE>(distrib(gen));
}

int getEnemyHealth(ENEMY_TYPE type) {
    switch (type) {
        case BASIC_GUNNER:
            return ENEMY_BASE_HEALTH;
        case FLYER:
            return ENEMY_FLYER_HEALTH;
        case TANK:
            return ENEMY_TANK_HEALTH;
        case SHOOTER:
            return ENEMY_SHOOTER_HEALTH;
    }
    return 0;
}

float getEnemySpeed(ENEMY_TYPE type) {
    switch (type) {
        case BASIC_GUNNER:
            return ENEMY_BASE_SPEED;
        case FLYER:
            return ENEMY_FLYER_SPEED;
        case TANK:
            return ENEMY_TANK_SPEED;
        case SHOOTER:
            return ENEMY_SHOOTER_SPEED;
    }
    return 0;
}

int getEnemyRange(ENEMY_TYPE type) {
    switch (type) {
        case BASIC_GUNNER:
            return ENEMY_BASE_RANGE;
        case FLYER:
            return ENEMY_FLYER_RANGE;
        case TANK:
            return ENEMY_TANK_RANGE;
        case SHOOTER:
            return ENEMY_SHOOTER_RANGE;
    }
    return 0;
}

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>
#include <camera_system.hpp>

Mix_Chunk* projectile_shoot;

Entity createPlayer(vec2 position) {
    Entity player;

    Player comp_player;
    comp_player.name = "Player 1";
    comp_player.direction = DIRECTION::DOWN;
    comp_player.player_state = PLAYERSTATE::IDLE;
    registry.players.emplace(player, comp_player);

    Motion comp_motion;
    comp_motion.angle = 0.0f;
    comp_motion.scale = {100, 100};
    comp_motion.position = position;
    registry.motions.emplace(player, comp_motion);

    return player;
}

// Seems like it's broken.
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

    return player;
}

Entity renderPlayer(Entity player) {
    RenderRequest comp_render_request = {
        TEXTURE_ASSET_ID::BUNNY_IDLE_DOWN0, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE};
    registry.renderRequests.emplace(player, comp_render_request);

    PlayerAnimation comp_anim;
    comp_anim.timer_ms = 250;
    comp_anim.curr_anim = TEXTURE_ASSET_ID::BUNNY_IDLE_DOWN0;
    registry.playerAnimations.emplace(player, comp_anim);

    return player;
}

// create enemy from spawner entity
Entity createEnemy(Entity entity) {
    EnemySpawner& spawner = registry.enemySpawners.get(entity);
    spawner.range = getEnemyHealth(spawner.type);

    Entity enemy;
    Enemy& comp_enemy = registry.enemies.emplace(enemy);
    comp_enemy.type = spawner.type;
    comp_enemy.health = getEnemyHealth(comp_enemy.type);
    comp_enemy.range = spawner.range;
    comp_enemy.timer_ms = 0;

    Motion& motion = registry.motions.emplace(enemy);
    motion = registry.motions.get(entity);
    motion.angle = 0.f;
    motion.velocity = {0.f, 0.f};

    registry.backgroundObjects.emplace(enemy);

    switch (comp_enemy.type) {
        case BASIC_GUNNER:
            motion.scale = {112, 56};
            registry.renderRequests.insert(
                enemy,
                {TEXTURE_ASSET_ID::CHICKEN_BOAT0, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});
            break;
        case FLYER:
            motion.scale = {56, 112};
            registry.renderRequests.insert(
                enemy,
                {TEXTURE_ASSET_ID::BALLOON0, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});
            break;
        case TANK:      // TODO!!
            motion.scale = {56, 112};
            registry.renderRequests.insert(
                enemy,
                {TEXTURE_ASSET_ID::BALLOON0, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});
            break;
        case SHOOTER:
            motion.scale = {112, 56};
            registry.renderRequests.insert(
                enemy,
                {TEXTURE_ASSET_ID::COW0, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});
            break;
    };


    std::cout << "Enemy id: " << enemy.id() << std::endl;
    return enemy;
}

Entity createBunny(Entity entity) {
    registry.backgroundObjects.emplace(entity);

    Bunny& bunny = registry.bunnies.get(entity);
    bunny.on_island = true;
    bunny.is_jailed = true;
    bunny.on_ship = false;
    bunny.on_base = false;
    bunny.moving_to_base = false;

    Motion& motion = registry.motions.get(entity);
    motion.angle = 0.f;
    motion.velocity = {0.f, 0.f};
    motion.scale = {40, 40};

    registry.renderRequests.insert(
        entity, {TEXTURE_ASSET_ID::BUNNY_NPC_JAILED0, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});

    return entity;
}

Entity createEnemy(vec2 position) {
    auto entity = Entity();
    registry.backgroundObjects.emplace(entity);

    Enemy& enemy = registry.enemies.emplace(entity);
    //enemy.type = getRandEnemyType();
    enemy.health = getEnemyHealth(enemy.type);
    enemy.speed = getEnemySpeed(enemy.type);
    enemy.timer_ms = 0;

    Motion& motion = registry.motions.emplace(entity);
    motion.angle = 0.f;
    motion.velocity = {0.f, 0.f};
    motion.position = position;
    motion.scale = {112, 56};

    registry.renderRequests.insert(entity,
                                   {TEXTURE_ASSET_ID::CHICKEN_BOAT0, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});

    std::cout << "Enemy id: " << entity.id() << std::endl;
    return entity;
}

Entity createBunny(RenderSystem* renderer, vec2 position) {
    auto entity = Entity();
    registry.backgroundObjects.emplace(entity);

    Bunny& bunny = registry.bunnies.emplace(entity);
    bunny.on_island = true;
    bunny.is_jailed = true;
    bunny.on_ship = false;

    bunny.jail_health = 10;
    bunny.timer_ms = 10000;

    Motion& motion = registry.motions.emplace(entity);
    motion.angle = 0.f;
    motion.velocity = {0.f, 0.f};
    motion.position = position;
    motion.scale = {56, 56};

    registry.renderRequests.insert(
        entity, {TEXTURE_ASSET_ID::BUNNY_NPC_JAILED0, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});

    return entity;
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

Entity createCannonProjectile(vec2 orig, vec2 dest) {
    Entity e;
    Motion& m = registry.motions.emplace(e);
    m.position = orig - CameraSystem::GetInstance()->position;
    m.scale = {GRID_CELL_WIDTH_PX / 2, GRID_CELL_HEIGHT_PX / 2};
    m.angle = degrees(atan2(dest.y - dest.x, dest.x - orig.x));
    vec2 velVec = dest - orig;
    m.velocity = normalize(velVec) * 350.0f;
    registry.backgroundObjects.emplace(e);
    registry.renderRequests.insert(
        e, {TEXTURE_ASSET_ID::BUNNY_FACE_ANGRY05, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});

    PlayerProjectile& proj = registry.playerProjectiles.emplace(e);
    proj.damage = SIMPLE_CANNON_DAMAGE;
    proj.alive_time_ms = PROJECTILE_LIFETIME;

    //Play sound
    if (projectile_shoot == nullptr) {
        projectile_shoot = Mix_LoadWAV(audio_path("projectile_shoot.wav").c_str());
    }
    Mix_PlayChannel(-1, projectile_shoot, 0);

    return e;
}

// TODO: Change the stats and sprite
Entity createEnemyProjectile(vec2 orig, vec2 dest) {
    Entity e;
    Motion& m = registry.motions.emplace(e);
    m.position = orig;
    m.scale = {GRID_CELL_WIDTH_PX / 2, GRID_CELL_HEIGHT_PX / 2};
    m.angle = degrees(atan2(dest.y - dest.x, dest.x - orig.x));
    vec2 velVec = dest - orig;
    m.velocity = normalize(velVec) * 50.0f;

    registry.renderRequests.insert(
        e, {TEXTURE_ASSET_ID::BUNNY_FACE_ANGRY05, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});
    
        EnemyProjectile& proj = registry.enemyProjectiles.emplace(e);
    proj.damage = SIMPLE_CANNON_DAMAGE;
    proj.alive_time_ms = PROJECTILE_LIFETIME;

    return e;
}

// Creates a steering wheel at specified
Entity createSteeringWheel(vec2 tile_pos) {
    Entity entity;
    SteeringWheel& steering_wheel = registry.steeringWheels.emplace(entity);
    steering_wheel.is_automated = false;

    Motion& motion = registry.motions.emplace(entity);
    vec2 world_pos = TileToVector2(tile_pos.x, tile_pos.y);
    motion.position.x = world_pos.x;
    motion.position.y = world_pos.y;

    motion.scale = {GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX};

    registry.renderRequests.insert(
        entity, {TEXTURE_ASSET_ID::SQUARE_3_CLICKED, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});

    return entity;
}

Entity createCannon(vec2 tile_pos) {
    Entity cannon;
    SimpleCannon& simple_cannon = registry.simpleCannons.emplace(cannon);
    simple_cannon.is_automated = false;
    simple_cannon.timer_ms = 0;

    Motion& motion = registry.motions.emplace(cannon);
    vec2 world_pos = TileToVector2(tile_pos.x, tile_pos.y);
    motion.position.x = world_pos.x;
    motion.position.y = world_pos.y;

    motion.scale = {GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX};

    registry.renderRequests.insert(
        cannon, {TEXTURE_ASSET_ID::SIMPLE_CANNON01, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});

    return cannon;
}

Entity createLaserWeapon(vec2 tile_pos){
    Entity laser;
    LaserWeapon& laser_weapon = registry.laserWeapons.emplace(laser);
    laser_weapon.is_automated = false;
    laser_weapon.timer_ms = 0; 

    Motion& motion = registry.motions.emplace(laser);
    vec2 world_pos = TileToVector2(tile_pos.x, tile_pos.y);
    motion.position.x = world_pos.x;
    motion.position.y = world_pos.y;

    motion.scale = {GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX};

    registry.renderRequests.insert(
        laser, {TEXTURE_ASSET_ID::LASER_WEAPON0, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});

    return laser;
}

Entity createLaserBeam(vec2 orig, vec2 dest) {
    Entity e;

    LaserBeam& beam = registry.LaserBeams.emplace(e);
    beam.damage = 0;
    beam.currWidth = 1;

    Motion& m = registry.motions.emplace(e);
    m.position = orig - CameraSystem::GetInstance()->position;

    float length = distance(orig, dest);
    m.scale = {length, beam.fixLength}; // Adjust height as needed

    m.angle = degrees(atan2(dest.y - orig.y, dest.x - orig.x));

    m.velocity = {0, 0};

    registry.backgroundObjects.emplace(e);
    registry.renderRequests.insert(
        e, {TEXTURE_ASSET_ID::TEXTURE_COUNT, EFFECT_ASSET_ID::EGG, GEOMETRY_BUFFER_ID::LASER_SQUARE});

    // TODO: play sound for the laser beam
    if (projectile_shoot == nullptr) {
        // projectile_shoot = Mix_LoadWAV(audio_path("projectile_shoot.wav").c_str());
    }
    // Mix_PlayChannel(-1, projectile_shoot, 0);

    return e;
}


void initializeShipModules(Ship& ship) {
    auto tmp_modules = std::vector<std::vector<MODULE_TYPES>>(ROW_COUNT, std::vector<MODULE_TYPES>(COL_COUNT, EMPTY));

    // This will make a ROW_COUNT * COL_COUNT new entities, which is fine i guess as they are just numbers.
    auto tmp_entities = std::vector<std::vector<Entity>>(ROW_COUNT, std::vector<Entity>(COL_COUNT));

    // Make a 3x3 platform from the middle.
    for (int i = MIDDLE_GRID_Y - 1; i <= MIDDLE_GRID_Y + 1; i++) {
        for (int j = MIDDLE_GRID_X - 1; j <= MIDDLE_GRID_X + 1; j++) {
            tmp_modules[i][j] = PLATFORM;
        }
    }

    vec2 SteeringWheelGridPos = {MIDDLE_GRID_X, MIDDLE_GRID_Y};
    tmp_modules[SteeringWheelGridPos.y][SteeringWheelGridPos.x] = STEERING_WHEEL;

    Entity wheel_entity = createSteeringWheel(SteeringWheelGridPos);
    tmp_entities[SteeringWheelGridPos.y][SteeringWheelGridPos.x] = wheel_entity;

    vec2 SimpleCannonGridPos = {MIDDLE_GRID_X, MIDDLE_GRID_Y - 1};
    tmp_modules[SimpleCannonGridPos.y][SimpleCannonGridPos.x] = SIMPLE_CANNON;

    Entity cannon_entity = createCannon(SimpleCannonGridPos);
    tmp_entities[SimpleCannonGridPos.y][SimpleCannonGridPos.x] = cannon_entity;

    ship.ship_modules = tmp_modules;
    ship.ship_modules_entity = tmp_entities;
}

Entity createIslandBackground(int width, int height, int offset_x, int offset_y, TEXTURE_ASSET_ID island_texture) {
    // create the island background entity
    Entity islandbg = Entity();
    registry.backgroundObjects.emplace(islandbg);
    Motion& islMotion = registry.motions.emplace(islandbg);
    offset_x -= WINDOW_WIDTH_PX / 2 - width / 2;
    offset_y -= WINDOW_HEIGHT_PX / 2 - height / 2;
    islMotion.position.x = WINDOW_WIDTH_PX / 2 + offset_x;
    islMotion.position.y = WINDOW_HEIGHT_PX / 2 + offset_y;
    std::cout << "offset: " << offset_x << ", " << offset_y << std::endl;
    islMotion.scale.x = width;
    islMotion.scale.y = height;

    registry.renderRequests.insert(
        islandbg, {island_texture, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});
    return islandbg;
}

Entity expandShip() {
    Entity ship = registry.ships.entities[0];
    
    Motion& shipMotion = registry.motions.get(ship);
    shipMotion.scale.x = GRID_CELL_WIDTH_PX * 5;
    shipMotion.scale.y = GRID_CELL_HEIGHT_PX * 5;

    Ship& shipStat = registry.ships.get(ship);
    shipStat.health = EXPANDED_SHIP_HEALTH;
    shipStat.maxHealth = EXPANDED_SHIP_HEALTH;

    shipStat.ship_modules[MIDDLE_GRID_Y - 2][MIDDLE_GRID_X - 2] = PLATFORM;
    shipStat.ship_modules[MIDDLE_GRID_Y - 1][MIDDLE_GRID_X - 2] = PLATFORM;
    shipStat.ship_modules[MIDDLE_GRID_Y - 0][MIDDLE_GRID_X - 2] = PLATFORM;
    shipStat.ship_modules[MIDDLE_GRID_Y + 1][MIDDLE_GRID_X - 2] = PLATFORM;
    shipStat.ship_modules[MIDDLE_GRID_Y + 2][MIDDLE_GRID_X - 2] = PLATFORM;
    shipStat.ship_modules[MIDDLE_GRID_Y + 2][MIDDLE_GRID_X - 1] = PLATFORM;
    shipStat.ship_modules[MIDDLE_GRID_Y + 2][MIDDLE_GRID_X - 0] = PLATFORM;
    shipStat.ship_modules[MIDDLE_GRID_Y + 2][MIDDLE_GRID_X + 1] = PLATFORM;
    shipStat.ship_modules[MIDDLE_GRID_Y + 2][MIDDLE_GRID_X + 2] = PLATFORM;
    shipStat.ship_modules[MIDDLE_GRID_Y + 1][MIDDLE_GRID_X + 2] = PLATFORM;
    shipStat.ship_modules[MIDDLE_GRID_Y - 0][MIDDLE_GRID_X + 2] = PLATFORM;
    shipStat.ship_modules[MIDDLE_GRID_Y - 1][MIDDLE_GRID_X + 2] = PLATFORM;
    shipStat.ship_modules[MIDDLE_GRID_Y - 2][MIDDLE_GRID_X + 2] = PLATFORM;
    shipStat.ship_modules[MIDDLE_GRID_Y - 2][MIDDLE_GRID_X + 1] = PLATFORM;
    shipStat.ship_modules[MIDDLE_GRID_Y - 2][MIDDLE_GRID_X + 0] = PLATFORM;
    shipStat.ship_modules[MIDDLE_GRID_Y - 2][MIDDLE_GRID_X - 1] = PLATFORM;

    return ship;
}

Entity createShip() {
    Entity entity = Entity();

    // registry.collisions.emplace(ship);

    Motion& shipMotion = registry.motions.emplace(entity);
    // need to add a component for ship like dieable or something
    shipMotion.position.x = WINDOW_WIDTH_PX / 2;
    shipMotion.position.y = WINDOW_HEIGHT_PX / 2;
    shipMotion.scale.x = GRID_CELL_WIDTH_PX * 3;  // the temporary grid height and width is 56
    shipMotion.scale.y = GRID_CELL_HEIGHT_PX * 3;

    // registry.renderRequests.insert(
    //     entity, {TEXTURE_ASSET_ID::TEXTURE_COUNT, EFFECT_ASSET_ID::EGG, GEOMETRY_BUFFER_ID::SHIP_SQUARE});

    registry.renderRequests.insert(
        entity, {TEXTURE_ASSET_ID::RAFT, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});

    Ship& ship = registry.ships.emplace(entity);
    ship.health = SHIP_BASE_HEALTH;
    ship.maxHealth = SHIP_BASE_HEALTH;

    initializeShipModules(ship);
    return entity;
}

Entity createCamera() {
    Entity entity = Entity();

    registry.cameras.emplace(entity);
    return entity;
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

Entity createFilledTile(vec2 position, vec2 size)
{
	// reserve an entity
	auto entity = Entity();
    registry.backgroundObjects.emplace(entity);

	// add a FilledTile component
	FilledTile& filledTile = registry.filledTiles.emplace(entity);
	filledTile.pos = position;
	filledTile.size = size;

    Motion& motion = registry.motions.emplace(entity);
    motion.angle = 0.f;
    motion.velocity = {0.f, 0.f};
    motion.position = position;
    motion.scale = size;

	// re-use the "DEBUG_LINE"
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::FILLED_TILE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	return entity;
}

// TODO: refactor this after map include disasters
Entity createDisaster(vec2 position, DISASTER_TYPE type) {
    auto entity = Entity();
    registry.backgroundObjects.emplace(entity);

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, 1); // int distribution in range [0, 1]

    int random_signX = dist(rng) == 0 ? 1 : -1;
    int random_signY = dist(rng) == 0 ? 1 : -1;

    Motion& motion = registry.motions.emplace(entity);
    Disaster& disaster = registry.disasters.emplace(entity);
    disaster.type = type;
    disaster.alive_time_ms = DISASTER_LIFETIME;

    switch (disaster.type) {
        case TORNADO:
            disaster.speed = 100;
            disaster.damage = 0.1;

            motion.velocity = {disaster.speed * random_signX, disaster.speed * random_signY};
            motion.scale = {168, 168};

            registry.renderRequests.insert(entity,
                {TEXTURE_ASSET_ID::TORNADO0, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});
            break;
        case WHIRLPOOL:
            disaster.speed = 0;
            disaster.damage = 0.1;

            motion.scale = {168, 112};

            registry.renderRequests.insert(entity,
                {TEXTURE_ASSET_ID::WHIRLPOOL0, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});
            break;
    };

    motion.angle = 0.f;
    motion.position = position;

    return entity;
}
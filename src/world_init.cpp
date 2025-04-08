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
#include "saveload_system.hpp"

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
        case DUMMY:
            return ENEMY_BASE_HEALTH;
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
        case DUMMY:
            return ENEMY_BASE_SPEED;
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
        case DUMMY:
            return ENEMY_BASE_RANGE;

    }
    return 0;
}

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>
#include <camera_system.hpp>

//Mix_Chunk* projectile_shoot;
//Mix_Chunk* laser_shoot;
//Mix_Chunk* cow_shoot;

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

    ParticleEmitter pe;
    pe.particles.resize(1000);
    ParticleProps props;
    props.ColorBegin = { 1, 1, 1, 1 };
    props.ColorEnd = { 53 / 255.0f, 55 / 255.0f, 59 / 255.0f, 0.0f };
	props.SizeBegin = 0.75f, props.SizeVariation = 0.3f, props.SizeEnd = 0.0f;
	props.LifeTime = 1.0f * 500.0f;
	props.Velocity = { 0.0f, 0.0f };
	props.VelocityVariation = { 3.0f, 10.0f };
	props.Position = position;
    props.Offset = { 0, 10.0f };
    pe.props = props;
    registry.particleEmitters.emplace(player, pe);

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
    comp_enemy.range = getEnemyRange(comp_enemy.type);
    comp_enemy.timer_ms = 0;
    comp_enemy.is_mod_affected = false;
    comp_enemy.mod_effect_duration = 0;
    comp_enemy.speed = getEnemySpeed(comp_enemy.type);

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
            comp_enemy.cooldown_ms = ENEMY_PROJECTILE_COOLDOWN;
            motion.scale = {112, 56};
            registry.renderRequests.insert(
                enemy,
                {TEXTURE_ASSET_ID::COW0, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});
            break;
        case DUMMY:
            motion.scale = {56, 56};
            registry.renderRequests.insert(
                enemy, {TEXTURE_ASSET_ID::BUNNY_FACE_ANGRY05, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});
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
    proj.mod_type = NONE;
    proj.damage = SIMPLE_CANNON_DAMAGE;
    proj.alive_time_ms = PROJECTILE_LIFETIME;

    ParticleEmitter pe;
    pe.particles.resize(1000);
    ParticleProps props;
    props.ColorBegin = { 254 / 255.0f, 212 / 255.0f, 123 / 255.0f, 1.0f };
	props.ColorEnd = { 254 / 255.0f, 109 / 255.0f, 41 / 255.0f, 0.5f };
	props.SizeBegin = 1.25f, props.SizeVariation = 0.3f, props.SizeEnd = 0.0f;
	props.LifeTime = 1.0f * 1000.0f;
	props.Velocity = {0,0};
	props.VelocityVariation = { 5.0f, 1.0f };

    pe.props = props;
    registry.particleEmitters.emplace(e, pe);

    //Play sound
    Entity sound_entity = Entity();
    Sound& sound = registry.sounds.emplace(sound_entity);
    sound.sound_type = SOUND_ASSET_ID::PROJECTILE_SHOOT;
    sound.volume = 50;
    /*if (projectile_shoot == nullptr) {
        projectile_shoot = Mix_LoadWAV(audio_path("projectile_shoot.wav").c_str());
    }
    Mix_PlayChannel(-1, projectile_shoot, 0);*/

    return e;
}

Entity createModifiedCannonProjectile(vec2 orig, vec2 dest, CannonModifier cm) {
    Entity e;
    Motion& m = registry.motions.emplace(e);
    m.position = orig - CameraSystem::GetInstance()->position;
    m.scale = {GRID_CELL_WIDTH_PX / 2, GRID_CELL_HEIGHT_PX / 2};
    m.angle = degrees(atan2(dest.y - dest.x, dest.x - orig.x));
    vec2 velVec = dest - orig;
    m.velocity = normalize(velVec) * 350.0f;
    registry.backgroundObjects.emplace(e);
    switch (cm.type) {
        case BUBBLE:
            registry.renderRequests.insert(
                e, {TEXTURE_ASSET_ID::BUBBLE_BULLET, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});
        break;
    }

    PlayerProjectile& proj = registry.playerProjectiles.emplace(e);
    proj.mod_type = cm.type;
    proj.damage = SIMPLE_CANNON_DAMAGE;
    proj.alive_time_ms = PROJECTILE_LIFETIME;

    ParticleEmitter pe;
    pe.particles.resize(1000);
    ParticleProps props;
    props.ColorEnd = { 148 / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 1.0f };
	props.ColorBegin = { 1, 1, 1, 0 };
	props.SizeBegin = 1.25f, props.SizeVariation = 0.5f, props.SizeEnd = 0.0f;
	props.LifeTime = 1.0f * 750.0f;
	props.Velocity = { 0.0f, 0.0f };
	props.VelocityVariation = { 3.0f, 1.0f };
	props.Position = orig;
    pe.props = props;
    registry.particleEmitters.emplace(e, pe);

    //Play sound
    Entity sound_entity = Entity();
    Sound& sound = registry.sounds.emplace(sound_entity);
    sound.sound_type = SOUND_ASSET_ID::PROJECTILE_SHOOT;
    sound.volume = 50;
    /*if (projectile_shoot == nullptr) {
        projectile_shoot = Mix_LoadWAV(audio_path("projectile_shoot.wav").c_str());
    }
    Mix_PlayChannel(-1, projectile_shoot, 0);*/

    return e;
}

// TODO: Change the stats and sprite
Entity createEnemyProjectile(vec2 orig, vec2 dest) {
    Entity e;
    registry.backgroundObjects.emplace(e);
    Motion& m = registry.motions.emplace(e);
    m.position = orig - CameraSystem::GetInstance()->position;
    m.scale = {GRID_CELL_WIDTH_PX / 2, GRID_CELL_HEIGHT_PX / 2};
    m.angle = degrees(atan2(dest.y - dest.x, dest.x - orig.x));
    vec2 velVec = dest - orig;
    m.velocity = normalize(velVec) * ENEMY_PROJECTILE_SPEED;

    registry.renderRequests.insert(
        e, {TEXTURE_ASSET_ID::BULLET_GREEN, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});
    
    EnemyProjectile& proj = registry.enemyProjectiles.emplace(e);
    proj.damage = 5;
    proj.alive_time_ms = ENEMY_PROJECTILE_LIFETIME;

    /*if (cow_shoot == nullptr) {
        cow_shoot = Mix_LoadWAV(audio_path("cow_bullet.wav").c_str());
    }
    Mix_PlayChannel(-1, cow_shoot, 0);*/

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
        entity, {TEXTURE_ASSET_ID::STEERING_WHEEL, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});

    return entity;
}

Entity createCannon(vec2 tile_pos) {
    Entity cannon;
    SimpleCannon& simple_cannon = registry.simpleCannons.emplace(cannon);
    simple_cannon.is_automated = false;
    simple_cannon.is_modified= false;
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

Entity modifyCannon(Entity entity, MODIFIER_TYPE type) {
    CannonModifier& cannon_modifier = registry.cannonModifiers.emplace(entity);
    cannon_modifier.type = type;

    SimpleCannon& simple_cannon = registry.simpleCannons.get(entity);
    simple_cannon.is_modified = true;

    TEXTURE_ASSET_ID& texture = registry.renderRequests.get(entity).used_texture;
    texture = TEXTURE_ASSET_ID::BUBBLE_CANNON;


    return entity;
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


std::vector<Entity> createLaserBeam(vec2 orig, vec2 dest) {
    std::vector<Entity> beams = {};
    vec2 positionToRender = orig + normalize(dest - orig)*15.f;

    for (int i = 0; i< 33; i++){
        Entity e;
        beams.push_back(e);
        LaserBeam& beam = registry.laserBeams.emplace(e);
        beam.damage = 20;
        beam.alive_time_ms = LASER_LIFETIME;
        beam.prevCamPos = CameraSystem::GetInstance()->position;

        Motion& m = registry.motions.emplace(e);
        positionToRender += normalize(dest - orig)*20.f;
        m.position = positionToRender - CameraSystem::GetInstance()->position;

        m.scale = vec2(20, 20);
        // m.angle = degrees(atan2(dest.y - m.position.y, dest.x - m.position.x)) + 90.0f;
        m.angle = degrees(atan2(dest.y - orig.y, dest.x - orig.x)) + 90.0f;
        m.velocity = {0, 0};

        registry.backgroundObjects.emplace(e);
        registry.renderRequests.insert(
            e, {TEXTURE_ASSET_ID::LASER_BEAM, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});

        Entity sound_entity = Entity();
        Sound& sound = registry.sounds.emplace(sound_entity);
        sound.sound_type = SOUND_ASSET_ID::LASER;
        sound.volume = 20;

        /*if (laser_shoot == nullptr) {
            laser_shoot = Mix_LoadWAV(audio_path("laser.wav").c_str());
            if (laser_shoot == nullptr) {
                laser_shoot = Mix_LoadWAV(audio_path("laser.wav").c_str());
                if (laser_shoot != nullptr) {
                    laser_shoot->volume = 50;
                }
            } else {
                laser_shoot->volume = 50;
            }
        }
        Mix_PlayChannel(-1, laser_shoot, 0);*/
    }
    return beams;
}


Entity createHealModule(vec2 tile_pos) {
    Entity heal;
    Heal& heal_module = registry.healModules.emplace(heal);
    heal_module.is_automated = false;
    heal_module.cooldown_ms = 0;

    Motion& motion = registry.motions.emplace(heal);
    vec2 world_pos = TileToVector2(tile_pos.x, tile_pos.y);
    motion.position.x = world_pos.x;
    motion.position.y = world_pos.y;

    motion.scale = vec2(GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX);

    registry.renderRequests.insert(
        heal, {TEXTURE_ASSET_ID::HEAL_MODULE, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});

    return heal;
}


void initializeShipModules(Ship& ship) {
    if (SaveLoadSystem::getInstance().hasLoadedData) {
        auto& ship_modules = SaveLoadSystem::getInstance().loadedGameData.used_modules;
        auto tmp_entities = std::vector<std::vector<Entity>>(ROW_COUNT, std::vector<Entity>(COL_COUNT));

        for (size_t i = 0; i < ship_modules.size(); ++i) {
            for (size_t j = 0; j < ship_modules[i].size(); ++j) {
                MODULE_TYPES& module = ship_modules[i][j];

                switch (module) {
                    case MODULE_TYPES::EMPTY: continue;
                    case MODULE_TYPES::PLATFORM: continue;
                    case MODULE_TYPES::STEERING_WHEEL: {
                        vec2 SteeringWheelGridPos = {j, i};
                        Entity wheel_entity = createSteeringWheel(SteeringWheelGridPos);
                        tmp_entities[i][j] = wheel_entity;
                        break;
                    };
                    case MODULE_TYPES::SIMPLE_CANNON: {
                        vec2 SimpleCannonGridPos = {j, i};
                        Entity cannon_entity = createCannon(SimpleCannonGridPos);
                        tmp_entities[i][j] = cannon_entity;
                        break;
                    }
                    case MODULE_TYPES::LASER_WEAPON: {
                        vec2 LaserWeaponGridPos = {j, i};
                        Entity laser_entity = createLaserWeapon(LaserWeaponGridPos);
                        tmp_entities[i][j] = laser_entity;
                        break;
                    }
                    case MODULE_TYPES::HELPER_BUNNY: {
                        // right now no chance of getting helper bunny in gacha so this will do for now
                        break;
                    }
                    case MODULE_TYPES::BUBBLE_MOD: {
                        vec2 bubbleGridPos = {j, i};
                        Entity bubble_cannon_entity = createCannon(bubbleGridPos);
                        tmp_entities[i][j] = bubble_cannon_entity;
                        modifyCannon(bubble_cannon_entity, MODIFIER_TYPE::BUBBLE);
                        break;
                    }
                    case MODULE_TYPES::HEAL: {
                        vec2 HealGridPos = {j, i};
                        Entity heal_entity = createHealModule(HealGridPos);
                        tmp_entities[i][j] = heal_entity;
                        break;
                    }
                
                    default: break;
                  }
            }
        }

        ship.ship_modules_entity = tmp_entities;

    } else {
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
}

Entity createWaterBackground() {
    // water background is fixed at 192 x 144 tiles
    int width = 192 * GRID_CELL_WIDTH_PX;
    int height = 144 * GRID_CELL_HEIGHT_PX;

    // create the water entity
    Entity waterbg = Entity();
    registry.backgroundObjects.emplace(waterbg);
    Motion& waterMotion = registry.motions.emplace(waterbg);

    waterMotion.position.x = WINDOW_WIDTH_PX / 2;
    waterMotion.position.y = WINDOW_HEIGHT_PX / 2;
    waterMotion.scale.x = width;
    waterMotion.scale.y = height;

    registry.renderRequests.insert(
        waterbg, {TEXTURE_ASSET_ID::WATER_BACKGROUND, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});
    return waterbg;
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
    islMotion.scale.x = width;
    islMotion.scale.y = height;

    registry.renderRequests.insert(
        islandbg, {island_texture, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});
    return islandbg;
}

Entity expandShip() {
    CameraSystem::GetInstance()->position = vec2(0,0);
    
    Entity ship = registry.ships.entities[0];
    
    Motion& shipMotion = registry.motions.get(ship);
    shipMotion.scale.x = GRID_CELL_WIDTH_PX * 5;
    shipMotion.scale.y = GRID_CELL_HEIGHT_PX * 5;

    Ship& shipStat = registry.ships.get(ship);
    shipStat.health = EXPANDED_SHIP_HEALTH;
    shipStat.maxHealth = EXPANDED_SHIP_HEALTH;
    shipStat.is_expanded = true;

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

    RenderLayer& render_layer = registry.renderLayers.emplace(entity);
    render_layer.layer = 2;
    registry.renderRequests.insert(
        entity, {TEXTURE_ASSET_ID::RAFT, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});

    Ship& ship = registry.ships.emplace(entity);
    ship.health = SHIP_BASE_HEALTH;
    ship.maxHealth = SHIP_BASE_HEALTH;
    ship.is_expanded = false;

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

Entity createOverlay(float alpha, vec3 color) {
    Entity entity = Entity();

    Overlay& overlay = registry.overlays.emplace(entity);
    overlay.alpha = alpha;
    registry.renderRequests.insert(
        entity, {TEXTURE_ASSET_ID::TEXTURE_COUNT, EFFECT_ASSET_ID::ALPHA, GEOMETRY_BUFFER_ID::OVERLAY_SQUARE});
    registry.colors.insert(entity, color);
    return entity;
}


std::vector<tson::Vector2i> get_poly_from_motion(const Motion& motion) {
    std::vector<tson::Vector2i> polygon;
    int posX = motion.position.x;
    int posY = motion.position.y;
    int rot = motion.angle;  // in degrees
    int halfWidth = motion.scale.x / 2;
    int halfHeight = motion.scale.y / 2;

    double rad = rot * M_PI / 180.0;
    double cosA = std::cos(rad);
    double sinA = std::sin(rad);

    std::vector<tson::Vector2i> corners = {
        {-halfWidth, -halfHeight},  // top left
        {halfWidth, -halfHeight},   // top right
        {halfWidth, halfHeight},    // bottom right
        {-halfWidth, halfHeight}    // bottom left
    };

    // rotate and translate back because it rotates around origin
    for (const auto& corner : corners) {
        int xNew = static_cast<int>(corner.x * cosA + corner.y * sinA) + posX;
        int yNew = static_cast<int>(-corner.x * sinA + corner.y * cosA) + posY;
        polygon.push_back(tson::Vector2i(xNew, yNew));
    }

    return polygon;
}

std::vector<Entity> createBaseProgressLines(Entity base_entity) {
    assert(registry.base.entities.size() > 0);
    std::vector<Entity> out = {};
    std::vector<tson::Vector2i> corners = get_poly_from_motion(registry.motions.get(base_entity));
    for (tson::Vector2i pos : corners) {
        Entity entity = Entity();
        GridLine& gridLine = registry.gridLines.emplace(entity);
        gridLine.start_pos = vec2(pos.x, pos.y) + CameraSystem::GetInstance()->position;
        gridLine.end_pos = vec2(0, 0);
        registry.backgroundObjects.emplace(entity);
        registry.renderRequests.insert(
            entity, {TEXTURE_ASSET_ID::TEXTURE_COUNT, EFFECT_ASSET_ID::EGG, GEOMETRY_BUFFER_ID::DEBUG_LINE});
        registry.colors.insert(entity, vec3(87 / 255.f, 114 / 255.f, 151 / 255.f));  // purple
        out.push_back(entity);
    }
    return out;
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
Entity createDisaster(Entity entity) {
    // added to background object in map_init
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, 1); // int distribution in range [0, 1]

    int random_signX = dist(rng) == 0 ? 1 : -1;
    int random_signY = dist(rng) == 0 ? 1 : -1;

    Motion& motion = registry.motions.get(entity);
    Disaster& disaster = registry.disasters.get(entity);
    disaster.alive_time_ms = DISASTER_LIFETIME;
    
    switch (disaster.type) {
        case TORNADO: {
            disaster.speed = 100;
            disaster.damage = 0.1;

            motion.velocity = {disaster.speed * random_signX, disaster.speed * random_signY};
            motion.scale = {168, 168};

            RenderLayer& render_layer_tornado = registry.renderLayers.emplace(entity);
            render_layer_tornado.layer = 3;
            registry.renderRequests.insert(
                entity, {TEXTURE_ASSET_ID::TORNADO0, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});

            break;
        }
        case WHIRLPOOL: {
            disaster.speed = 0;
            disaster.damage = 0.1;

            motion.scale = {168, 112};

            RenderLayer& render_layer_wp = registry.renderLayers.emplace(entity);
            render_layer_wp.layer = 1;
            registry.renderRequests.insert(
                entity, {TEXTURE_ASSET_ID::WHIRLPOOL0, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});
        }
            break;
    };

    motion.angle = 0.f;

    return entity;
}

Entity createBunnyIcon(vec2 tile_pos) {
    Entity e;
    HelperBunnyIcon& helper_bunny_icon = registry.helperBunnyIcons.emplace(e);
    helper_bunny_icon.tile_pos = tile_pos;

    Motion& motion = registry.motions.emplace(e);
    motion.position = TileToVector2(tile_pos.x, tile_pos.y) + vec2(15.0f, -15.0f);
    motion.scale = {20, 20};

    registry.renderRequests.insert
        (e, {TEXTURE_ASSET_ID::BUNNY_NPC_FACE, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});
    return e;
}

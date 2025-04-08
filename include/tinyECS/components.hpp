#pragma once
#include <unordered_map>
#include <vector>

#include "../ext/stb_image/stb_image.h"
#include "../ext/tileson/tileson.hpp"
#include "common.hpp"

// All data relevant to the shape and motion of entities
struct Motion {
    vec2 position = {0, 0};
    float angle = 0;
    vec2 velocity = {0, 0};
    vec2 scale = {10, 10};
};

// Stucture to store collision information
struct Collision {
    // Note, the first object is stored in the ECS container.entities
    Entity other;  // the second object involved in the collision
    vec2 normal;
    Collision(Entity& other, vec2 normal = {0.0, 0.0}) {
        this->other = other;
        this->normal = normal;
    }
};

// Sets the brightness of the screen
struct ScreenState {
    float darken_screen_factor = -1;
    float vignette_screen_factor = -1;
};

struct Island {
    std::vector<tson::Vector2i> polygon;
};

// Player base, in the future may add more attributes for upgrades functionality
struct Base {
    std::vector<tson::Vector2i> polygon;
    float drop_off_timer = 0.0; // keep track of how long the ship is inside the base
    bool ship_in_base = false;
    int bunny_count = 0;
    bool locked = false; // for tutorial: when true, dropoff will not work
};

// Data structure for toggling debug mode
struct Debug {
    bool in_debug_mode = 0;
    bool in_freeze_mode = 0;
};
extern Debug debugging;

// A struct to refer to debugging graphics in the ECS
struct DebugComponent {
    // Note, an empty struct has size 1
};

// used to hold grid line start and end positions
// Dayshaun: start_pos is not actually the start of the line, and end_pos is not actually the end position of the line.
// start_pos = {coords of the middle of the line}
// end_pos   = {width of the line, height of the line} (basically SCALE)
struct GridLine {
    vec2 start_pos = {0, 0};
    vec2 end_pos = {10, 10};  // default to diagonal line
};

struct Overlay {
    float alpha = 0.7;
    vec2 pos = {WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2};
    vec2 size = {WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX};
    bool visible = false;
};

struct Spotlight {
    vec2 position = {0.0, 0.0};
    float radius = 0.0;
};

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & chicken.vs.glsl)
struct ColoredVertex {
    vec3 position;
    vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex {
    vec3 position;
    vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh {
    static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices,
                                std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
    vec2 original_size = {1, 1};
    std::vector<ColoredVertex> vertices;
    std::vector<uint16_t> vertex_indices;
};

enum class TEXTURE_ASSET_ID {
    // Bunny player
    BUNNY_IDLE_UP0 = 0,
    BUNNY_IDLE_UP1 = BUNNY_IDLE_UP0 + 1,

    BUNNY_IDLE_RIGHT0 = BUNNY_IDLE_UP1 + 1,
    BUNNY_IDLE_RIGHT1 = BUNNY_IDLE_RIGHT0 + 1,

    BUNNY_IDLE_DOWN0 = BUNNY_IDLE_RIGHT1 + 1,
    BUNNY_IDLE_DOWN1 = BUNNY_IDLE_DOWN0 + 1,

    BUNNY_IDLE_LEFT0 = BUNNY_IDLE_DOWN1 + 1,
    BUNNY_IDLE_LEFT1 = BUNNY_IDLE_LEFT0 + 1,

    BUNNY_UP_WALK0 = BUNNY_IDLE_LEFT1 + 1,
    BUNNY_UP_WALK1 = BUNNY_UP_WALK0 + 1,

    BUNNY_RIGHT_WALK0 = BUNNY_UP_WALK1 + 1,
    BUNNY_RIGHT_WALK1 = BUNNY_RIGHT_WALK0 + 1,

    BUNNY_DOWN_WALK0 = BUNNY_RIGHT_WALK1 + 1,
    BUNNY_DOWN_WALK1 = BUNNY_DOWN_WALK0 + 1,

    BUNNY_LEFT_WALK0 = BUNNY_DOWN_WALK1 + 1,
    BUNNY_LEFT_WALK1 = BUNNY_LEFT_WALK0 + 1,

    WATER_BACKGROUND = BUNNY_LEFT_WALK1 + 1,
    
    // Game Maps
    TUTORIAL_BACKGROUND = WATER_BACKGROUND + 1,
    LEVEL01_BACKGROUND = TUTORIAL_BACKGROUND + 1,
    LEVEL02_BACKGROUND = LEVEL01_BACKGROUND + 1,
    LEVEL03_BACKGROUND = LEVEL02_BACKGROUND + 1,
    LEVEL04_BACKGROUND = LEVEL03_BACKGROUND + 1,

    // Enemies
    BALLOON0 = LEVEL04_BACKGROUND + 1,
    BALLOON1 = BALLOON0 + 1,
    BALLOON2 = BALLOON1 + 1,

    CHICKEN_BOAT0 = BALLOON2 + 1,
    CHICKEN_BOAT1 = CHICKEN_BOAT0 + 1,

    COW0 = CHICKEN_BOAT1 + 1,
    COW1 = COW0 + 1,
    COW2 = COW1 + 1,

    // Tornado
    TORNADO0 = COW2 + 1,
    TORNADO1 = TORNADO0 + 1,
    TORNADO2 = TORNADO1 + 1,

    // Whirlpool
    WHIRLPOOL0 = TORNADO2 + 1,
    WHIRLPOOL1 = WHIRLPOOL0 + 1,
    WHIRLPOOL2 = WHIRLPOOL1 + 1,
    
    // UI assets.
    SQUARE_3_NORMAL = WHIRLPOOL2 + 1,
    SQUARE_3_HOVER = SQUARE_3_NORMAL + 1,
    SQUARE_3_CLICKED = SQUARE_3_HOVER + 1,

    PLAY_BUTTON_NORMAL = SQUARE_3_CLICKED + 1,
    PLAY_BUTTON_CLICKED = PLAY_BUTTON_NORMAL + 1,

    LONG_BOX = PLAY_BUTTON_CLICKED + 1,
    LONG_BOX_CLICKED = LONG_BOX + 1,

    // Bunny UI Neutral Face.
    BUNNY_FACE_NEUTRAL01 = LONG_BOX_CLICKED + 1,
    BUNNY_FACE_NEUTRAL02 = BUNNY_FACE_NEUTRAL01 + 1,
    BUNNY_FACE_NEUTRAL03 = BUNNY_FACE_NEUTRAL02 + 1,
    BUNNY_FACE_NEUTRAL04 = BUNNY_FACE_NEUTRAL03 + 1,
    BUNNY_FACE_NEUTRAL05 = BUNNY_FACE_NEUTRAL04 + 1,
    BUNNY_FACE_NEUTRAL06 = BUNNY_FACE_NEUTRAL05 + 1,
    BUNNY_FACE_NEUTRAL07 = BUNNY_FACE_NEUTRAL06 + 1,
    BUNNY_FACE_NEUTRAL08 = BUNNY_FACE_NEUTRAL07 + 1,
    BUNNY_FACE_NEUTRAL09 = BUNNY_FACE_NEUTRAL08 + 1,

    // Bunny UI Angry Face.
    BUNNY_FACE_ANGRY01 = BUNNY_FACE_NEUTRAL09 + 1,
    BUNNY_FACE_ANGRY02 = BUNNY_FACE_ANGRY01 + 1,
    BUNNY_FACE_ANGRY03 = BUNNY_FACE_ANGRY02 + 1,
    BUNNY_FACE_ANGRY04 = BUNNY_FACE_ANGRY03 + 1,
    BUNNY_FACE_ANGRY05 = BUNNY_FACE_ANGRY04 + 1,

    // Tile Cursor
    TILE_CURSOR = BUNNY_FACE_ANGRY05 + 1,

    RAFT = TILE_CURSOR + 1,

    // Simple Cannon
    SIMPLE_CANNON01 = RAFT + 1,
    SIMPLE_CANNON01_SHADED = SIMPLE_CANNON01 + 1,

    // Bubble Cannon
    BUBBLE_CANNON = SIMPLE_CANNON01_SHADED + 1,
    BUBBLE_CANNON_SHADED = BUBBLE_CANNON + 1,

    // Projectiles
    BULLET_GREEN = BUBBLE_CANNON_SHADED + 1,
    BULLET_BLUE = BULLET_GREEN + 1,
    BULLET_RED = BULLET_BLUE + 1,

    BUBBLE_BULLET = BULLET_RED + 1,

    // Bunny npc
    BUNNY_NPC_JAILED0 = BUBBLE_BULLET + 1,
    BUNNY_NPC_JAILED1 = BUNNY_NPC_JAILED0 + 1,

    BUNNY_NPC_IDLE_UP0 = BUNNY_NPC_JAILED1 + 1,
    BUNNY_NPC_IDLE_UP1 = BUNNY_NPC_IDLE_UP0 + 1,
    
    MAIN_MENU_BG = BUNNY_NPC_IDLE_UP1 + 1,

    FILLED_TILE = MAIN_MENU_BG + 1,

    RESTART_BUTTON_NORMAL = FILLED_TILE + 1,
    RESTART_BUTTON_CLICKED = RESTART_BUTTON_NORMAL + 1,

    GAME_OVER_BG = RESTART_BUTTON_CLICKED + 1,

    TUTORIAL_DIALOGUE_BOX_UI = GAME_OVER_BG + 1,

    TUTORIAL_WASD_UI = TUTORIAL_DIALOGUE_BOX_UI + 1,
    TUTORIAL_SPACE_UI = TUTORIAL_WASD_UI + 1,
    TUTORIAL_MOUSE_UI = TUTORIAL_SPACE_UI + 1,
    TUTORIAL_HOME_UI = TUTORIAL_MOUSE_UI + 1,

    NEXT_LEVEL_BG = TUTORIAL_HOME_UI + 1,

    UPGRADE_TITLE = NEXT_LEVEL_BG + 1,

    TUTORIAL_BUTTON_NORMAL = UPGRADE_TITLE + 1,
    TUTORIAL_BUTTON_CLICKED = TUTORIAL_BUTTON_NORMAL + 1, 

    CONTINUE_BUTTON_NORMAL = TUTORIAL_BUTTON_CLICKED + 1,
    CONTINUE_BUTTON_CLICKED = CONTINUE_BUTTON_NORMAL + 1,

    LASER_WEAPON0 = CONTINUE_BUTTON_CLICKED + 1,
    LASER_WEAPON0_SHADED = LASER_WEAPON0 + 1,
    LASER_BEAM = LASER_WEAPON0_SHADED + 1,

    BUNNY_NPC_FACE = LASER_BEAM + 1,

    BUNNY_INDICATOR = BUNNY_NPC_FACE + 1,

    CUTSCENE_1 = BUNNY_INDICATOR + 1,
    CUTSCENE_2 = CUTSCENE_1 + 1,
    CUTSCENE_3 = CUTSCENE_2 + 1,
    CUTSCENE_4 = CUTSCENE_3 + 1,
    CUTSCENE_BUNNY_VILLAGE = CUTSCENE_4 + 1,
    CUTSCENE_COW_CAPTURE = CUTSCENE_BUNNY_VILLAGE + 1,
    CUTSCENE_CRYING_BUNNY = CUTSCENE_COW_CAPTURE + 1,
    CUTSCENE_MANY_BUNNY_CAGED = CUTSCENE_CRYING_BUNNY + 1,
    CUTSCENE_BUNNY_ESCAPE = CUTSCENE_MANY_BUNNY_CAGED + 1,
    CUTSCENE_BUNNY_BUILD = CUTSCENE_BUNNY_ESCAPE + 1,

    END_CUTSCENE_HUG = CUTSCENE_BUNNY_BUILD + 1,
    END_CUTSCENE_SAIL = END_CUTSCENE_HUG + 1,
    END_CUTSCENE_VILLAGE = END_CUTSCENE_SAIL + 1,

    TEXT_M1 = END_CUTSCENE_VILLAGE + 1,
    TEXT_M2 = TEXT_M1 + 1,
    TEXT_M3 = TEXT_M2 + 1,
    TEXT_M4 = TEXT_M3 + 1,
    TEXT_ASSETS = TEXT_M4 + 1,
    TEXT_THANKS = TEXT_ASSETS + 1,
    TEXT_TITLE = TEXT_THANKS + 1,
    
    HOME_INDICATOR  = TEXT_TITLE + 1,

    VICTORY_BG = HOME_INDICATOR + 1,
    
    BOOK_ICON = VICTORY_BG + 1,
    BOOK = BOOK_ICON + 1,

    //Curved texts
    MODULETYPE_TEXT = BOOK + 1,

    MODULEDESCRIPTION_TEXT = MODULETYPE_TEXT + 1,
    SIMPLE_CANNON_TEXT = MODULEDESCRIPTION_TEXT + 1,
    DESC_INTRO_TEXT = SIMPLE_CANNON_TEXT + 1,
    BUBBLE_BUFF_TEXT = DESC_INTRO_TEXT + 1,
    BUBBLE_CANNON_TEXT = BUBBLE_BUFF_TEXT + 1,
    HEALING_MODULE_TEXT = BUBBLE_CANNON_TEXT + 1,
    LASER_MODULE_TEXT = HEALING_MODULE_TEXT + 1,

    HEAL_MODULE = LASER_MODULE_TEXT + 1,
    HEAL_MODULE_SHADED = HEAL_MODULE + 1,

    STEERING_WHEEL = HEAL_MODULE_SHADED + 1,
    STEERING_WHEEL_TEXT = STEERING_WHEEL + 1,
    BUNNY_INDICATOR_TEXT = STEERING_WHEEL_TEXT + 1,
    HOME_INDICATOR_TEXT = BUNNY_INDICATOR_TEXT + 1,
    SHIP_TEXT = HOME_INDICATOR_TEXT + 1, 

    EXIT_BUTTON_NORMAL = SHIP_TEXT + 1,
    EXIT_BUTTON_CLICKED = EXIT_BUTTON_NORMAL + 1,
    
    TEXTURE_COUNT = EXIT_BUTTON_CLICKED + 1
};

const int texture_count = (int) TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
    COLOURED = 0,
    EGG = COLOURED + 1,
    CHICKEN = EGG + 1,
    TEXTURED = CHICKEN + 1,
    VIGNETTE = TEXTURED + 1,
    FONT = VIGNETTE + 1,
    PARTICLE = FONT + 1,
    ALPHA = PARTICLE + 1,
    EFFECT_COUNT = ALPHA + 1
};
const int effect_count = (int) EFFECT_ASSET_ID::EFFECT_COUNT;

enum class SOUND_ASSET_ID {
    BACKGROUND_MUSIC,
    ENEMY_INCOMING,
    ISLAND_SHIP_COLLISION,
    ENEMY_SHIP_COLLISION,
    PROJECTILE_ENEMY_COLLISION,
    PROJECTILE_JAIL_COLLISION,
    GAME_OVER,
    CUTSCENE_MUSIC,
    BOOK,
    BUBBLE,
    CLICK,
    LASER,
    MODULE,
    RAFT,
    COW_BULLET,
    PROJECTILE_SHOOT,
    END_MUSIC,
    SOUND_COUNT
};
const int sound_count = (int) SOUND_ASSET_ID::SOUND_COUNT;

enum class GEOMETRY_BUFFER_ID {
    CHICKEN = 0,
    SPRITE = CHICKEN + 1,
    EGG = SPRITE + 1,
    DEBUG_LINE = EGG + 1,
    SCREEN_TRIANGLE = DEBUG_LINE + 1,
    LASER_SQUARE = SCREEN_TRIANGLE + 1,
    UI_SQUARE = LASER_SQUARE + 1,
    HIGHLIGHT_SQUARE = UI_SQUARE + 1,
    OVERLAY_SQUARE = HIGHLIGHT_SQUARE + 1,
    GEOMETRY_COUNT = OVERLAY_SQUARE + 1
};
const int geometry_count = (int) GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
    TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
    EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
    GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
    float highlight_radius = 0.0;
};

struct Sound {
    SOUND_ASSET_ID sound_type = SOUND_ASSET_ID::ENEMY_INCOMING;
    bool is_repeating = false;
    int volume = 0;
};


/* for ordering of rendering. so far (add to comment to keep track):
1 = whirlpool
2 = ship
3 = tornado
*/
struct RenderLayer {
    int layer = 0;
};

enum DIRECTION { UP, RIGHT, DOWN, LEFT };

// ========== PLAYER DETAILS ==========

/*
 *  A PlayerAnimation will read the Player component
 *  So be sure to add Player Component when adding PlayerAnimation
 */

enum PLAYERSTATE {
    IDLE,
    WALKING,
    STATIONING,
    BUILDING
};

// Player component
struct Player {
    float health;
    std::string name;
    DIRECTION direction;
    PLAYERSTATE player_state;
    bool is_sailing_ship;
};

struct PlayerAnimation {
    TEXTURE_ASSET_ID curr_anim;
    int timer_ms;  // How many ms before switching to the next frame.
};

// ========= Camera related components ======================
// used for updating the objects in the background as camera moves with ship
// backgroundObject is anything that doesn't move with the ship

struct BackgroundObject {};
struct Camera{
    vec2 acceleration = {0,0};
    bool applyFrictionX = true;
    bool applyFrictionY = true;
};


// ========== SHIP DETAILS ==========

enum MODULE_TYPES {
    EMPTY,
    PLATFORM,
    STEERING_WHEEL,
    SIMPLE_CANNON,
    LASER_WEAPON,
    HEAL,
    HELPER_BUNNY,
    BUBBLE_MOD
};

inline TEXTURE_ASSET_ID getTextureFromModuleType(MODULE_TYPES module) {
    switch (module)
    {
    case MODULE_TYPES::SIMPLE_CANNON :
        return TEXTURE_ASSET_ID::SIMPLE_CANNON01;
    case MODULE_TYPES::PLATFORM :
        return TEXTURE_ASSET_ID::RAFT;
    case MODULE_TYPES::HELPER_BUNNY :
        return TEXTURE_ASSET_ID::BUNNY_NPC_IDLE_UP0;
    case MODULE_TYPES::STEERING_WHEEL :
        return TEXTURE_ASSET_ID::STEERING_WHEEL;
    case MODULE_TYPES::LASER_WEAPON :
        return TEXTURE_ASSET_ID::LASER_WEAPON0;
    case MODULE_TYPES::BUBBLE_MOD :
        return TEXTURE_ASSET_ID::BUBBLE_BULLET;
    case MODULE_TYPES::HEAL:
        return TEXTURE_ASSET_ID::HEAL_MODULE;
    default:
        std::cout << "This is not a valid module" << std::endl;
        return TEXTURE_ASSET_ID::WATER_BACKGROUND; 
    }
}

inline std::string getModuleName(MODULE_TYPES module) {
    switch (module) {
    case MODULE_TYPES::SIMPLE_CANNON :
        return "Simple Cannon";
    case MODULE_TYPES::PLATFORM :
        return "Expand Ship";
    case MODULE_TYPES::LASER_WEAPON :
        return "Laser Module";
    case MODULE_TYPES::BUBBLE_MOD :
        return "Bubble Buff";
    case MODULE_TYPES::HEAL:
        return "Heal Module";
    default:
        return "This shouldn't be a module bruh.";
    }
}

struct SteeringWheel {
    bool is_automated;
};

struct PlayerProjectile {
    MODIFIER_TYPE mod_type;
    float damage;
    float alive_time_ms; // How long before we remove this projectile.
};

struct EnemyProjectile {
    float damage;
    float alive_time_ms;  // How long before we remove this projectile.
};

struct SimpleCannon {
    bool is_automated;
    bool is_modified;
    float timer_ms; // The cooldown period before another shot.
};

struct CannonModifier {
    MODIFIER_TYPE type;
};

struct LaserWeapon {
    bool is_automated;
    float timer_ms;
    float maxLoadTime_ms = 1500;
};

struct LaserBeam {
    float damage;
    float alive_time_ms;
    vec2 prevCamPos;
};

struct Heal {
    bool is_automated;
    float cooldown_ms;  // The cooldown period before another heal.
};

struct Ship {
    float health;
    float maxHealth;
    bool is_expanded;
    // This defines what the module is AND the corresponding entity.
    std::vector<std::vector<MODULE_TYPES>> ship_modules;
    std::vector<std::vector<Entity>> ship_modules_entity;

    // This defines how many of each modules is in our inventory. 
    std::unordered_map<MODULE_TYPES, uint> available_modules;
};

struct HelperBunnyIcon {
    vec2 tile_pos;
};

// ========== ENEMY DETAILS ==========
// the enemy type stores information about enemy HP, damage, speed, etc..
struct Enemy {
    ENEMY_TYPE type;
    int health;
    bool is_mod_affected;
    int mod_effect_duration;
	int timer_ms; // for sprite animation
    int range = 10;
    float speed;
    int cooldown_ms; // cooldown for enemy projectile
};

struct EnemySpawner {
    ENEMY_TYPE type;
    int range = 10;
    int home_island;
    int cooldown_ms; // cooldown for spawning enemies
};

// walking path for enemy
struct WalkingPath {
	std::vector<ivec2> path;
};

// filled tile for enemy path
struct FilledTile {
	vec2 pos;
	vec2 size;
};


// ========== BUNNY DETAILS ==========
struct Bunny {
    bool on_island;
    bool is_jailed;
    bool on_ship;
    bool on_base;
    bool moving_to_base;

    bool on_module;

    float jail_health;  // 0 if is_jailed is false
    int timer_ms;   // field reserved for animation 
};

// ========= OBSTACLE DETAILS =========
struct Disaster {
    DISASTER_TYPE type;
	float timer_ms;
    int speed;
    float damage;
    float alive_time_ms;
};

// Particle System following (https://www.youtube.com/watch?v=GK0jHlv3e3w)
struct ParticleProps {
	vec2 Position;
    vec2 Offset = {0,0};
	vec2 Velocity, VelocityVariation;
	vec4 ColorBegin, ColorEnd;
	float SizeBegin, SizeEnd, SizeVariation;
	float LifeTime = 1.0f;
};

struct Particle {
    glm::vec2 Position;
    glm::vec2 Velocity;
    glm::vec4 ColorBegin, ColorEnd;
    float Rotation = 0.0f;
    float SizeBegin, SizeEnd;

    float LifeTime = 1.0f * 500.0f;
    float LifeRemaining = 0.0f;

    bool Active = false;
};

struct ParticleEmitter {
    ParticleProps props;
    std::vector<Particle> particles;
    uint32_t poolIndex = 999;

    float delay_ms = DEFAULT_PARTICLE_TIME;
};

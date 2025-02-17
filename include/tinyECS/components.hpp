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
    Collision(Entity& other) { this->other = other; };
};

// Sets the brightness of the screen
struct ScreenState
{
	float darken_screen_factor = -1;
	float vignette_screen_factor = -1;
};

struct Island {
    tson::Vector2<int> polygon;
};

// Player base, in the future may add more attributes for upgrades functionality
struct Base {
    tson::Vector2<int> polygon;
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
struct GridLine {
    vec2 start_pos = {0, 0};
    vec2 end_pos = {10, 10};  // default to diagonal line
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

/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID {
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

    TEXTURE_COUNT = WATER_BACKGROUND + 1
};

const int texture_count = (int) TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
    COLOURED = 0,
    EGG = COLOURED + 1,
    CHICKEN = EGG + 1,
    TEXTURED = CHICKEN + 1,
    VIGNETTE = TEXTURED + 1,
    EFFECT_COUNT = VIGNETTE + 1
};
const int effect_count = (int) EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
    CHICKEN = 0,
    SPRITE = CHICKEN + 1,
    EGG = SPRITE + 1,
    DEBUG_LINE = EGG + 1,
    SCREEN_TRIANGLE = DEBUG_LINE + 1,
    SHIP_SQUARE = SCREEN_TRIANGLE + 1,
    GEOMETRY_COUNT = SHIP_SQUARE + 1
};
const int geometry_count = (int) GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
    TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
    EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
    GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
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
};

// Player component
struct Player {
    std::string name;
    DIRECTION direction;
    PLAYERSTATE player_state;
    bool is_sailing_ship;
};

struct PlayerAnimation {
    TEXTURE_ASSET_ID curr_anim;
    int timer_ms;  // How many ms before switching to the next frame.
};

// Camera related componenet
// used for updating the objects in the background as camera moves with ship
// bachgroundObject is anything that doesn't move with the ship

struct BackgroundObject {};

// ========== SHIP DETAILS ==========
struct Ship {
    float health;
    int num_weapon;
};

// ========== ENEMY DETAILS ==========
// the enemy type stores information about enemy HP, damage, speed, etc..
// TODO: ADD ENEMY TYPE INFORMATION IN COMMON
enum ENEMY_TYPE { BASIC_GUNNER, FLYER };

struct Enemy {
    ENEMY_TYPE type;
};

// ========== ENEMY DETAILS ==========
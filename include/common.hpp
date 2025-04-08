#pragma once

// stlib
#include <fstream>  // stdout, stderr..
#include <string>
#include <tuple>
#include <vector>

#define NOMINMAX
#include <gl3w.h>
#include <GLFW/glfw3.h>

// The glm library provides vector and matrix operations as in GLSL
#include <glm/vec2.hpp>             // vec2
#include <glm/ext/vector_int2.hpp>  // ivec2
#include <glm/vec3.hpp>             // vec3
#include <glm/mat3x3.hpp>           // mat3
using namespace glm;

#include "tinyECS/tiny_ecs.hpp"

// Simple utility functions to avoid mistyping directory name
// audio_path("audio.ogg") -> data/audio/audio.ogg
// Get defintion of PROJECT_SOURCE_DIR from:
#include "../ext/project_path.hpp"
inline std::string data_path() {
    return std::string(PROJECT_SOURCE_DIR) + "data";
};
inline std::string shader_path(const std::string& name) {
    return std::string(PROJECT_SOURCE_DIR) + "/shaders/" + name;
};
inline std::string textures_path(const std::string& name) {
    return data_path() + "/textures/" + std::string(name);
};
inline std::string audio_path(const std::string& name) {
    return data_path() + "/audio/" + std::string(name);
};
inline std::string mesh_path(const std::string& name) {
    return data_path() + "/meshes/" + std::string(name);
};
inline std::string map_path(const std::string& name) {
    return data_path() + "/maps/" + std::string(name);
};
inline std::string font_path(const std::string& name) {
    return data_path() + "/fonts/" + std::string(name);
};

enum ENEMY_TYPE { BASIC_GUNNER = 0, FLYER = 1, TANK = 2, SHOOTER = 3, DUMMY = 4 };
enum DISASTER_TYPE { TORNADO = 0, WHIRLPOOL = 1};
enum MODIFIER_TYPE { NONE = 0, BUBBLE = 1 };


const float DISASTER_LIFETIME = 30000.0f;   // 30 seconds life time 
const float BUBBLE_MOD_EFFECT_FACTOR = 0;

//
// game constants
//

const int BLOCK = 80;

const int WINDOW_WIDTH_PX = 840;
const int WINDOW_HEIGHT_PX = 616;

const int GRID_CELL_WIDTH_PX = 56;
const int GRID_CELL_HEIGHT_PX = 56;
const int GRID_LINE_WIDTH_PX = 2;
const int PROGRESS_BAR_LINE_WIDTH_PX = 4;

// middle grid for the center of the ship
const int MIDDLE_GRID_X = 7;
const int MIDDLE_GRID_Y = 5;

const int COL_COUNT = WINDOW_WIDTH_PX/GRID_CELL_WIDTH_PX;
const int ROW_COUNT = WINDOW_HEIGHT_PX/GRID_CELL_HEIGHT_PX;

const float SHIP_BASE_HEALTH = 100.f;
const float EXPANDED_SHIP_HEALTH = 150.f;

const int ENEMY_BASE_HEALTH = 20;
const float ENEMY_BASE_SPEED = 50;
const float ENEMY_BASE_RANGE = 20;
const float ENEMY_BASE_SPAWN_CD_MS = 35 * 1000; // 35 seconds

const int ENEMY_FLYER_HEALTH = 15;
const float ENEMY_FLYER_SPEED = 75;
const float ENEMY_FLYER_RANGE = 30;

const int ENEMY_TANK_HEALTH = 30;
const float ENEMY_TANK_SPEED = 25;
const float ENEMY_TANK_RANGE = 10;

const int ENEMY_SHOOTER_HEALTH = 25;
const float ENEMY_SHOOTER_SPEED = 0;
const float ENEMY_SHOOTER_RANGE = 5.0f;
const float ENEMY_PROJECTILE_SPEED = 100.0f;

const float ENEMY_PROJECTILE_COOLDOWN = 2000.0f;

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// The 'Transform' component handles transformations passed to the Vertex shader
// (similar to the gl Immediate mode equivalent, e.g., glTranslate()...)
// We recommend making all components non-copyable by derving from ComponentNonCopyable
struct Transform {
    mat3 mat = {{1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}};  // start with the identity
    void scale(vec2 scale);
    void rotate(float radians);
    void translate(vec2 offset);
};

bool gl_has_errors();

inline vec2 TileToVector2(int tile_x, int tile_y) {
  return vec2((int)tile_x * GRID_CELL_WIDTH_PX + 30,
              (int)tile_y * GRID_CELL_HEIGHT_PX + 30);
}

// ==== PLAYER CONSTANTS ====

// Character controls.
const int MOVE_UP_BUTTON = GLFW_KEY_W;
const int MOVE_RIGHT_BUTTON = GLFW_KEY_D;
const int MOVE_DOWN_BUTTON = GLFW_KEY_S;
const int MOVE_LEFT_BUTTON = GLFW_KEY_A;

const float WALK_SPEED = 150.0f;
const float ANIMATION_TIME = 250.0f;
const float SHIP_CAMERA_SPEED = 100.0f;

const float SIMPLE_CANNON_COOLDOWN = 1000.0f;
const float SIMPLE_CANNON_DAMAGE = 10.0f;
const float PROJECTILE_LIFETIME = 2000.0f;
const float ENEMY_PROJECTILE_LIFETIME = 3000.0f;

const float MODIFIER_EFFECT_DURATION = SIMPLE_CANNON_COOLDOWN;

const float LASER_LIFETIME = 200.0f;
const int   LASER_LENGTH_IN_NUM = 3; // This is how many 50 pixles laser beam we render to stack
const float LASER_COOLDOWN = 5000.0f;   // 5 second

const float HEAL_COOLDOWN = 10000.0f;  // 10 second
const float HEAL_AMOUNT = 25.0f;

const float BUNNY_BASE_DROPOFF_TIME = 1000.0f;  // ship must be in base for 1 second before the bunnies get dropped off

const float DEFAULT_PARTICLE_TIME = 50.0f;

const float SIMPLE_CANNON_AUTO_RANGE = 350.0f;
const float LASER_AUTO_RANGE = 250.0f;

#include "scenes/level_01.hpp"
#include <glm/ext/vector_float2.hpp>
#include <string>
#include "scenes/game_level.hpp"
#include "world_init.hpp"
#include "world_system.hpp"

Level01::Level01(WorldSystem* world_system, std::string map_filename) : GameLevel(world_system) {
    this->name = "Level 1";
    this->level_path = map_filename;
}

Level01::~Level01() {}

void Level01::LevelInit() {
    createBunny(vec2(100, 200));
}

void Level01::LevelUpdate() {}

// GameLevel should already handle clearing all components. So don't have to worry here.
void Level01::LevelExit() {}

void Level01::LevelHandleInput(int key, int action, int mod) {}

void Level01::LevelHandleMouseMove(glm::vec2 mousePos) {}

void Level01::LevelHandleMouseClick(int button, int action, int mods) {}

void Level01::LevelUpdate(float dt) {}

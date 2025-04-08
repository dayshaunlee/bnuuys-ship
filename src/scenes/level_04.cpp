#include "scenes/level_04.hpp"
#include <glm/ext/vector_float2.hpp>
#include <string>
#include "sceneManager/scene_manager.hpp"
#include "scenes/game_level.hpp"
#include "tinyECS/components.hpp"
#include "world_init.hpp"
#include "world_system.hpp"
#include "gacha_system.hpp"

Level04::Level04(WorldSystem* world_system, std::string map_filename, TEXTURE_ASSET_ID texture) : GameLevel(world_system) {
    this->name = "Level 4";
    this->level_path = map_filename; 
    this->bunnies_to_win = 0;
    this->texture = texture;
    GachaSystem::getInstance().setLevelPool(
        4, {MODULE_TYPES::SIMPLE_CANNON, MODULE_TYPES::PLATFORM, MODULE_TYPES::LASER_WEAPON, MODULE_TYPES::BUBBLE_MOD, MODULE_TYPES::HEAL});
}

Level04::~Level04() {
}

void Level04::LevelInit() {
    // unlock the base
    registry.base.components[0].locked = false;
}

void Level04::LevelUpdate() {}

// GameLevel should already handle clearing all components. So don't have to worry here.
void Level04::LevelExit() {}

void Level04::LevelHandleInput(int key, int action, int mod) {}

void Level04::LevelHandleMouseMove(glm::vec2 mousePos) {}

void Level04::LevelHandleMouseClick(int button, int action, int mods) {}

void Level04::LevelUpdate(float dt) {
    // update window title with points
    int points = registry.base.components[0].bunny_count;
    std::string title_points = std::to_string(points);
    world_system->add_to_title("Total bunny saved: " + title_points + "/" + std::to_string(bunnies_to_win));
    if (registry.base.components[0].bunny_count == bunnies_to_win) {
        std::cout << "BEAT LEVEL -- SAVED ALL [" << bunnies_to_win << "] BUNNIES" << std::endl; 
    }
    
    if(upgradesReceived == bunnies_to_win){
        SceneManager& sceneManager = SceneManager::getInstance();
        sceneManager.switchScene("EndCutscene");
        SaveLoadSystem& saveLoadSystem = SaveLoadSystem::getInstance();
        GameData gameData = saveLoadSystem.createGameData("Player1", "Level 4", registry.ships.components[0]);
        saveLoadSystem.saveGame(gameData, "level_save.json");
        std::cout << "Level 4 saved" << std::endl;
        return;
    }

    if(this->upgradesReceived < points){
        if(!this->gacha_called){
            std::cout << "Gacha popup in level 4.." << std::endl; 
            this->gacha_called = true;
            GachaSystem::getInstance().displayGacha(4, this->scene_ui, *this);
        }
    }
}

#include "scenes/level_03.hpp"
#include <glm/ext/vector_float2.hpp>
#include <string>
#include "sceneManager/scene_manager.hpp"
#include "scenes/game_level.hpp"
#include "tinyECS/components.hpp"
#include "world_init.hpp"
#include "world_system.hpp"
#include "gacha_system.hpp"

Level03::Level03(WorldSystem* world_system, std::string map_filename, TEXTURE_ASSET_ID texture) : GameLevel(world_system) {
    this->name = "Level 3";
    this->level_path = map_filename; 
    this->bunnies_to_win = 0;
    this->texture = texture;
    GachaSystem::getInstance().setLevelPool(
        3, {MODULE_TYPES::SIMPLE_CANNON, MODULE_TYPES::PLATFORM, MODULE_TYPES::LASER_WEAPON, MODULE_TYPES::BUBBLE_MOD, MODULE_TYPES::HEAL});
}

Level03::~Level03() {}

void Level03::LevelInit() {
    // unlock the base
    registry.base.components[0].locked = false;
}

void Level03::LevelUpdate() {}

// GameLevel should already handle clearing all components. So don't have to worry here.
void Level03::LevelExit() {}

void Level03::LevelHandleInput(int key, int action, int mod) {}

void Level03::LevelHandleMouseMove(glm::vec2 mousePos) {}

void Level03::LevelHandleMouseClick(int button, int action, int mods) {}

void Level03::LevelUpdate(float dt) {
    // update window title with points
    int points = registry.base.components[0].bunny_count;
    std::string title_points = std::to_string(points);
    world_system->add_to_title("Total bunny saved: " + title_points + "/" + std::to_string(bunnies_to_win));
    if (registry.base.components[0].bunny_count == bunnies_to_win) {
        std::cout << "BEAT LEVEL -- SAVED ALL [" << bunnies_to_win << "] BUNNIES" << std::endl; 
    }
    
    if(upgradesReceived == bunnies_to_win){
        SceneManager& sceneManager = SceneManager::getInstance();
        sceneManager.setNextLevelScence("Level 4");
        std::cout << "Switching to next level scene.." << std::endl;
        sceneManager.switchScene("Next Level Scene");

        SaveLoadSystem& saveLoadSystem = SaveLoadSystem::getInstance();
        GameData gameData = saveLoadSystem.createGameData("Player1", "Level 4", registry.ships.components[0]);
        saveLoadSystem.saveGame(gameData, "level_save.json");
        std::cout << "Level 3 saved" << std::endl;
        
        return;
    }

    if(this->upgradesReceived < points){
        if(!this->gacha_called){
            std::cout << "Gacha popup in level 3.." << std::endl; 
            this->gacha_called = true;
            GachaSystem::getInstance().displayGacha(3, this->scene_ui, *this);
        }
    }
}

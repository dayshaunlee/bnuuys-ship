#include "scenes/level_01.hpp"
#include <glm/ext/vector_float2.hpp>
#include <string>
#include "sceneManager/scene_manager.hpp"
#include "scenes/game_level.hpp"
#include "tinyECS/components.hpp"
#include "world_init.hpp"
#include "world_system.hpp"
#include "gacha_system.hpp"
#include "saveload_system.hpp"

Level01::Level01(WorldSystem* world_system, std::string map_filename, TEXTURE_ASSET_ID texture) : GameLevel(world_system) {
    this->name = "Level 1";
    this->level_path = map_filename; 
    this->bunnies_to_win = 0;
    this->texture = texture;
    //TODO: need to see if we should put platform and laser in level 1 in the actual game, ok for now
    GachaSystem::getInstance().setLevelPool(
        1, {MODULE_TYPES::SIMPLE_CANNON, MODULE_TYPES::LASER_WEAPON, MODULE_TYPES::BUBBLE_MOD,
            MODULE_TYPES::HEAL});
}

Level01::~Level01() {}

void Level01::LevelInit() {
    // unlock the base
    registry.base.components[0].locked = false;
}

void Level01::LevelUpdate() {}

// GameLevel should already handle clearing all components. So don't have to worry here.
void Level01::LevelExit() {}

void Level01::LevelHandleInput(int key, int action, int mod) {}

void Level01::LevelHandleMouseMove(glm::vec2 mousePos) {}

void Level01::LevelHandleMouseClick(int button, int action, int mods) {}

void Level01::LevelUpdate(float dt) {
    // update window title with points
    int points = registry.base.components[0].bunny_count;
    std::string title_points = std::to_string(points);
    world_system->add_to_title("Total bunny saved: " + title_points + "/" + std::to_string(bunnies_to_win));
    if (registry.base.components[0].bunny_count == bunnies_to_win) {
        std::cout << "BEAT LEVEL -- SAVED ALL [" << bunnies_to_win << "] BUNNIES" << std::endl; 
    }
    
    if(upgradesReceived == bunnies_to_win){
        SceneManager& sceneManager = SceneManager::getInstance();
        sceneManager.setNextLevelScence("Level 2");
        std::cout << "Switching to next level scene.." << std::endl;
        sceneManager.switchScene("Next Level Scene");

        SaveLoadSystem& saveLoadSystem = SaveLoadSystem::getInstance();
        GameData gameData = saveLoadSystem.createGameData("Player1", "Level 2", registry.ships.components[0]);
        saveLoadSystem.saveGame(gameData, "level_save.json");
        std::cout << "Level 1 saved" << std::endl;

        return;
    }

    // std::cout << "upgraded: " << this->upgradesReceived << " points " << points << std::endl;
    if(this->upgradesReceived < points){
        if(!this->gacha_called){
            std::cout << "Gacha popup in level 1.." << std::endl; 
            this->gacha_called = true;
            // todo M3: update level 1 pool
            GachaSystem::getInstance().displayGacha(1, this->scene_ui, *this);
            // std::cout << "Gacha poped.." << std::endl;
        }
    }
    // std::cout << "Gacha end.." << std::endl;
}

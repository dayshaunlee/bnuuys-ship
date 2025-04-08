#pragma once

#include <glm/ext/vector_float2.hpp>
#include <string>
#include "ai_system.hpp"
#include "animation_system.hpp"
#include "inventory_system.hpp"
#include "modules_system.hpp"
#include "physics_system.hpp"
#include "sceneManager/scene.hpp"
#include "tinyECS/components.hpp"
#include "world_system.hpp"
#include "render_system.hpp"
#include "sound_system.hpp"
#include "particle_system.hpp"

// This class describes a parent class for Gameplay Levels.
class GameLevel : public Scene {
private:
    void InitializeTrackingUI();
    void InitializeBunnySavingUI();
    void InitializeBookUI();
    void InitializeUI();
    void InitializePauseUI();

   protected:
    AISystem ai_system;
    PhysicsSystem physics_system;
    AnimationSystem animation_system;
    ModulesSystem module_system;
    InventorySystem inventory_system;
    WorldSystem* world_system;
    SoundSystem* sound_system;
    ParticleSystem particle_system;

    void RemoveStation(vec2 tile_pos, MODULE_TYPES module);
    std::string level_path; // TODO: Make a mapping for level_path and the background.
    // amount of bunnies player has to save to win
    TEXTURE_ASSET_ID texture;
    int bunnies_to_win = 0;
    std::vector<Entity> base_corners;
    
    // track attributes of these for tutorial
    std::shared_ptr<bnuui::Element> tracker_ui;
    std::shared_ptr<bnuui::Element> book;
    std::shared_ptr<bnuui::Element> book_icon;
    bool tracker_off_screen = true;
    
    virtual void LevelInit() = 0;
    virtual void LevelUpdate() = 0;
    virtual void LevelExit() = 0;
    virtual void LevelHandleInput(int key, int action, int mod) = 0;
    virtual void LevelHandleMouseMove(glm::vec2 mousePos) = 0;
    virtual void LevelHandleMouseClick(int button, int action, int mods) = 0;
    virtual void LevelUpdate(float dt) = 0;

public:
    int upgradesReceived = 0;
    bool gacha_called = false;
    GameLevel(WorldSystem* worldsystem);
    void Init() override;
    void Exit() override;
    void HandleInput(int key, int action, int mod) override;
    void HandleMouseMove(glm::vec2 mousePos) override;
    void HandleMouseClick(int button, int action, int mods) override;
    void Update(float dt) override; 

    void UpdateDropoffProgressBar();

    virtual ~GameLevel() = default;
}; 

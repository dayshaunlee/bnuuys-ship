#include "scenes/game_level.hpp"
#include <cmath>
#include <deque>
#include <glm/common.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include "GLFW/glfw3.h"
#include "bnuui/bnuui.hpp"
#include "camera_system.hpp"
#include "common.hpp"
#include "gacha_system.hpp"
#include "inventory_system.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "world_init.hpp"
#include "bnuui/buttons.hpp"
#include "map_init.hpp"
#include "saveload_system.hpp"
#include <sceneManager/scene_manager.hpp>

/*
 *   Place 'local' scene vars here just so it's easy to manage.
 */

// Just to make my life easier. (TODO: REFACTOR LATER)
std::shared_ptr<bnuui::Cursor> tile_cursor;
vec2 l1_mouse_pos;
MODULE_TYPES curr_selected = EMPTY;

std::set<int> activeKeys;
std::deque<int> keyOrder;

std::set<int> activeShipKeys;
std::deque<int> keyShipOrder;

GameLevel::GameLevel(WorldSystem* worldsystem) : inventory_system(scene_ui, curr_selected) {
    this->world_system = worldsystem;
}

MODULE_TYPES getModuleType(vec2 tile_pos) {
    Ship& ship = registry.ships.components[0];
    return ship.ship_modules[tile_pos.y][tile_pos.x];
}

vec2 getMouseTilePosition() {
    int tile_x = (int) (l1_mouse_pos.x / GRID_CELL_WIDTH_PX);
    int tile_y = (int) (l1_mouse_pos.y / GRID_CELL_HEIGHT_PX);
    return vec2(tile_x, tile_y);
}

void GameLevel::Init() {
    activeShipKeys.clear();
    scene_ui.clear();
    RenderSystem::isRenderingGacha = false;
    gacha_called = false;
    upgradesReceived = 0;
    // create player
    Entity player = createPlayer({WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2});
    // load map
    registry.list_all_components();
    std::cout << "loading map..." << std::endl;
    std::pair<tson::Vector2i, tson::Vector2i> map = loadMap(level_path);
    tson::Vector2i mapSize = map.first;
    tson::Vector2i map_offset = map.second;
    registry.list_all_components();
    std::cout << mapSize.x << ", " << mapSize.y << std::endl;

    // create the ocean background and then ship
    createWaterBackground();
    createIslandBackground(mapSize.x, mapSize.y, map_offset.x, map_offset.y, texture);
    
    createShip();
    assert(registry.base.entities.size() > 0);
    // stored from top left -> clockwise
    base_corners = createBaseProgressLines(registry.base.entities[0]);
    Ship& ship = registry.ships.components[0];
    ship.health = ship.maxHealth;

    if (SaveLoadSystem::getInstance().hasLoadedData) {
        GameData gd = SaveLoadSystem::getInstance().loadedGameData;
        ship.health = gd.ship_health;
        ship.maxHealth = gd.ship_maxHealth;
        ship.is_expanded = gd.ship_is_expanded;
        ship.ship_modules = gd.used_modules;
        ship.available_modules = gd.unused_modules;
        std::cout << "loaded saved ship" << std::endl;
    }

    if (ship.is_expanded) {
        Motion& shipMotion = registry.motions.get(registry.ships.entities[0]);
        shipMotion.scale.x = GRID_CELL_WIDTH_PX * 5;
        shipMotion.scale.y = GRID_CELL_HEIGHT_PX * 5;
        GachaSystem::getInstance().setDropRate(MODULE_TYPES::PLATFORM, 0);
    }

    // render player
    renderPlayer(player);

    createCamera();
    Entity overlay_entity = createOverlay(0.7);
    Overlay& overlay = registry.overlays.get(overlay_entity);

    bunnies_to_win = 0;
    // bunny creation
    for (Entity entity : registry.bunnies.entities) {
        createBunny(entity);
        bunnies_to_win += 1;
    };
    for (Entity entity : registry.disasters.entities) {
        createDisaster(entity);
    }
    /*createDisaster({300, 100}, DISASTER_TYPE::TORNADO);
    createDisaster({300, 100}, DISASTER_TYPE::WHIRLPOOL);*/

    registry.players.components[0].health = 100.0f;
    
    InitializeUI();
    LevelInit();

    std::cout << "Num of ships: " << registry.ships.components.size() << std::endl;

    RenderSystem::isInGame = true;
    RenderSystem::isPaused = false;
    RenderSystem::isRenderingBook = false;
    RenderSystem::isRenderingGacha = false;
}

bool isOffscreen(const glm::vec2& A, const glm::vec2& center) {
    // Compute screen bounds based on center B
    float halfWidth = WINDOW_WIDTH_PX / 2.0f;
    float halfHeight = WINDOW_HEIGHT_PX / 2.0f;

    float left = center.x - halfWidth;
    float right = center.x + halfWidth;
    float top = center.y - halfHeight;
    float bottom = center.y + halfHeight;

    // Check if A is outside the screen bounds
    return (A.x < left || A.x > right || A.y < top || A.y > bottom);
}

void GameLevel::InitializePauseUI() {
    scene_ui.pause_ui = std::make_shared<bnuui::Box>(
        vec2(WINDOW_WIDTH_PX / 4, WINDOW_HEIGHT_PX / 2), vec2(200, 200), 0.0f);

    auto continue_btn = std::make_shared<bnuui::LongBox>(
        vec2(WINDOW_WIDTH_PX / 4, 280), vec2(150, 50), 0.0f);
    continue_btn->setOnClick([](bnuui::Element& e) { RenderSystem::isPaused = false;
    });
    auto continue_text = std::make_shared<bnuui::TextLabel>(vec2(WINDOW_WIDTH_PX / 4 - 50.f, 284), 1, "Resume");
    continue_text->color = vec3(91.f/255.f, 58.f/255.f, 37.f/255.f);
    continue_btn->children.push_back(continue_text);

    auto mm_btn = std::make_shared<bnuui::LongBox>(
        vec2(WINDOW_WIDTH_PX / 4, 330), vec2(150, 50), 0.0f);
    mm_btn ->setOnClick([](bnuui::Element& e) { 
        RenderSystem::isPaused = false;
        SceneManager::getInstance().switchScene("Main Menu");
    });
    auto mm_text = std::make_shared<bnuui::TextLabel>(vec2(WINDOW_WIDTH_PX / 4 - 50.f, 334), 1, "Main Menu");
    mm_text->color = vec3(91.f/255.f, 58.f/255.f, 37.f/255.f);
    mm_btn->children.push_back(mm_text);

    scene_ui.pause_ui->children.push_back(continue_btn);
    scene_ui.pause_ui->children.push_back(mm_btn);
}

void GameLevel::InitializeTrackingUI() {
    auto tracking_ui = std::make_shared<bnuui::Box>(vec2(496, 96), vec2(45, 45), 0.0f);
    tracking_ui->texture = TEXTURE_ASSET_ID::BUNNY_INDICATOR;
    
    tracking_ui->setOnUpdate([this](bnuui::Element& e, float dt) {
        float smallest_dist = std::numeric_limits<float>::max();
        vec2 shortest_bunny_pos;

        Entity ship_entity = registry.ships.entities[0];
        vec2 ship_pos = registry.motions.get(ship_entity).position - CameraSystem::GetInstance()->position;

        // First get all the location of bunnies and find the nearest one.
        for (Entity bunny_entity : registry.bunnies.entities) {
            Bunny bunny = registry.bunnies.get(bunny_entity);
            if (bunny.is_jailed && registry.motions.has(bunny_entity)) { 
                vec2 bunny_pos = registry.motions.get(bunny_entity).position;
                float dist = glm::distance(bunny_pos, ship_pos);
                if (dist < smallest_dist) {
                    smallest_dist = dist;
                    shortest_bunny_pos = bunny_pos;
                }
            }
        }
        if (smallest_dist == std::numeric_limits<float>::max()) {
            // e.visible = false;
            e.texture = TEXTURE_ASSET_ID::HOME_INDICATOR;
            shortest_bunny_pos = vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2);
        } else {
            e.texture = TEXTURE_ASSET_ID::BUNNY_INDICATOR;
        }

        vec2 direction = glm::normalize(shortest_bunny_pos - ship_pos);

        if (isOffscreen(shortest_bunny_pos, ship_pos)) {
            tracker_off_screen = true;
            vec2 ui_padding = vec2(30, 30);

            // Find the edge of the screen where the UI should be positioned
            float halfWidth = WINDOW_WIDTH_PX / 2.0f - ui_padding.x;
            float halfHeight = WINDOW_HEIGHT_PX / 2.0f - ui_padding.y;

            vec2 ui_pos = ship_pos + CameraSystem::GetInstance()->position; // Start at the ship

            // Project position onto screen edge
            float slope = direction.y / direction.x;
            vec2 clamped_pos = ui_pos;

            if (direction.x > 0) {  // Right edge
                clamped_pos.x = ui_pos.x + halfWidth;
                clamped_pos.y = ui_pos.y + slope * halfWidth;
            } else {  // Left edge
                clamped_pos.x = ui_pos.x - halfWidth;
                clamped_pos.y = ui_pos.y - slope * halfWidth;
            }

            if (clamped_pos.y > ui_pos.y + halfHeight) {  // Clamp to top edge
                clamped_pos.y = ui_pos.y + halfHeight;
                clamped_pos.x = ui_pos.x + halfHeight / slope;
            } else if (clamped_pos.y < ui_pos.y - halfHeight) {  // Clamp to bottom edge
                clamped_pos.y = ui_pos.y - halfHeight;
                clamped_pos.x = ui_pos.x - halfHeight / slope;
            }

            // Apply final UI position
            e.position = clamped_pos;// Offset for better UI positioning
        } else {
            tracker_off_screen = false;
            e.position = shortest_bunny_pos + CameraSystem::GetInstance()->position + vec2(15, -15);
        }
    });
    scene_ui.insert(tracking_ui);
    tracker_ui = tracking_ui;
}



void GameLevel::InitializeBookUI(){
    auto book_icon = std::make_shared<bnuui::Box>(vec2(WINDOW_WIDTH_PX-180+20, 95-20), vec2(100, 100), 0.0f);
    book_icon->texture = TEXTURE_ASSET_ID::BOOK_ICON;

    auto book = std::make_shared<bnuui::Book>(vec2(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2 + 50), vec2(730, 730), 0.0f);
    book->visible = false;

    vec3 textColour = vec3(115.f/255.f, 75.f/255.f, 50.f/255.f);
    vec2 itemBasePos = vec2(WINDOW_WIDTH_PX/2 -210, WINDOW_HEIGHT_PX/2 - 55);
    vec2 itemBoxSize = vec2{60,60};
    vec2 itemIconSize = vec2{50,50};
    vec2 itemSpaceX = vec2{70,0};
    vec2 itemSpaceY = vec2{0,70};

    auto moduletype_text = std::make_shared<bnuui::Box>(vec2(WINDOW_WIDTH_PX/2-130,WINDOW_HEIGHT_PX/2 -65), vec2(450, 250), 0.0f);
    moduletype_text->texture = TEXTURE_ASSET_ID::MODULETYPE_TEXT;

    auto moduledesc_text = std::make_shared<bnuui::Box>(vec2(WINDOW_WIDTH_PX/2+150,WINDOW_HEIGHT_PX/2 -69), vec2(400, 250), 0.0f);
    moduledesc_text->texture = TEXTURE_ASSET_ID::MODULEDESCRIPTION_TEXT;
    

    auto module_iconBox1 = std::make_shared<bnuui::Box>(itemBasePos, itemBoxSize, 0.0f);
    auto module_icon1 = std::make_shared<bnuui::Box>(itemBasePos, itemIconSize, 0.0f); 
    module_iconBox1->children.push_back(module_icon1);
    module_icon1->texture = TEXTURE_ASSET_ID::SIMPLE_CANNON01;
    module_icon1->visible = false;

    auto module_iconBox2 = std::make_shared<bnuui::Box>(itemBasePos + itemSpaceX, itemBoxSize, 0.0f);
    auto module_icon2 = std::make_shared<bnuui::Box>(itemBasePos + itemSpaceX, itemIconSize, 0.0f); 
    module_iconBox2->children.push_back(module_icon2);
    module_icon2->texture = TEXTURE_ASSET_ID::BUBBLE_CANNON;
    module_icon2->visible = false;

    auto module_iconBox3 = std::make_shared<bnuui::Box>(itemBasePos + 2.f*itemSpaceX, itemBoxSize, 0.0f);
    auto module_icon3 = std::make_shared<bnuui::Box>(itemBasePos + 2.f*itemSpaceX, itemIconSize, 0.0f); 
    module_iconBox3->children.push_back(module_icon3);
    module_icon3->texture = TEXTURE_ASSET_ID::LASER_WEAPON0;
    module_icon3->visible = false;

    auto module_iconBox4 = std::make_shared<bnuui::Box>(itemBasePos + itemSpaceY, itemBoxSize, 0.0f);
    auto module_icon4 = std::make_shared<bnuui::Box>(itemBasePos + itemSpaceY, itemIconSize, 0.0f);
    module_iconBox4->children.push_back(module_icon4);
    module_icon4->texture = TEXTURE_ASSET_ID::HEAL_MODULE;
    module_icon4->visible = false;

    auto module_iconBox5 = std::make_shared<bnuui::Box>(itemBasePos + itemSpaceY + itemSpaceX, itemBoxSize, 0.0f);
    auto module_icon5 = std::make_shared<bnuui::Box>(itemBasePos + itemSpaceY + itemSpaceX, itemIconSize*0.8f, 0.0f);
    module_iconBox5->children.push_back(module_icon4);
    module_icon5->texture = TEXTURE_ASSET_ID::BUBBLE_BULLET;
    module_icon5->visible = false;

    auto module_iconBox6 = std::make_shared<bnuui::Box>(itemBasePos + itemSpaceY + 2.f*itemSpaceX, itemBoxSize, 0.0f);
    auto module_icon6 = std::make_shared<bnuui::Box>(itemBasePos + itemSpaceY + 2.f*itemSpaceX, itemIconSize*0.8f, 0.0f);
    module_iconBox6->children.push_back(module_icon6);
    module_icon6->texture = TEXTURE_ASSET_ID::STEERING_WHEEL;
    module_icon6->visible = false;

    auto module_iconBox7 = std::make_shared<bnuui::Box>(itemBasePos + 2.f*itemSpaceY, itemBoxSize, 0.0f);
    auto module_icon7 = std::make_shared<bnuui::Box>(itemBasePos + 2.f*itemSpaceY, itemIconSize*0.8f, 0.0f);
    module_iconBox7->children.push_back(module_icon7);
    module_icon7->texture = TEXTURE_ASSET_ID::BUNNY_INDICATOR;
    module_icon7->visible = false;

    auto module_iconBox8 = std::make_shared<bnuui::Box>(itemBasePos + 2.f*itemSpaceY + itemSpaceX, itemBoxSize, 0.0f);
    auto module_icon8 = std::make_shared<bnuui::Box>(itemBasePos + 2.f*itemSpaceY + itemSpaceX, itemIconSize*0.8f, 0.0f);
    module_iconBox8->children.push_back(module_icon8);
    module_icon8->texture = TEXTURE_ASSET_ID::HOME_INDICATOR;
    module_icon8->visible = false;

    auto module_iconBox9 = std::make_shared<bnuui::Box>(itemBasePos + 2.f*itemSpaceY + 2.f*itemSpaceX, itemBoxSize, 0.0f);
    auto module_icon9 = std::make_shared<bnuui::Box>(itemBasePos + 2.f*itemSpaceY + 2.f*itemSpaceX, itemIconSize*0.8f, 0.0f);
    module_iconBox9->children.push_back(module_icon9);
    module_icon9->texture = TEXTURE_ASSET_ID::RAFT;
    module_icon9->visible = false;

    auto itemDesc = std::make_shared<bnuui::Box>(vec2(WINDOW_WIDTH_PX/2 + 20, WINDOW_HEIGHT_PX/2 + 80), vec2(800, 500), 0.0f);
    itemDesc->texture = TEXTURE_ASSET_ID::DESC_INTRO_TEXT;

    book_icon->setOnClick([book, module_icon1, module_icon2, module_icon3, module_icon4, module_icon5,
        module_icon6, module_icon7, module_icon8, module_icon9](bnuui::Element& e) {
        if(!RenderSystem::isRenderingGacha){
            RenderSystem::isRenderingBook = !RenderSystem::isRenderingBook;
            book->visible = !book->visible;
            module_icon1->visible = !module_icon1->visible;
            module_icon2->visible = !module_icon2->visible;
            module_icon3->visible = !module_icon3->visible;
            module_icon4->visible = !module_icon4->visible;
            module_icon5->visible = !module_icon5->visible;
            module_icon6->visible = !module_icon6->visible;
            module_icon7->visible = !module_icon7->visible;
            module_icon8->visible = !module_icon8->visible;
            module_icon9->visible = !module_icon9->visible;

            // Play sound
            Entity sound_entity = Entity();
            Sound& sound = registry.sounds.emplace(sound_entity);
            sound.sound_type = SOUND_ASSET_ID::BOOK;
            sound.volume = 50;
        }
    });


    module_icon1->setOnClick([&, book, itemDesc](bnuui::Element& e) {
        if(book->visible){
            itemDesc->texture = TEXTURE_ASSET_ID::SIMPLE_CANNON_TEXT;
            itemDesc->visible = true;
        }
    });

    module_icon2->setOnClick([&, book, itemDesc](bnuui::Element& e) {
        if(book->visible){
            itemDesc->texture = TEXTURE_ASSET_ID::BUBBLE_CANNON_TEXT;
            itemDesc->visible = true;
        }
    });

    module_icon3->setOnClick([&, book, itemDesc](bnuui::Element& e) {
        if(book->visible){
            itemDesc->texture = TEXTURE_ASSET_ID::LASER_MODULE_TEXT;
            itemDesc->visible = true;
        }
    });

    module_icon4->setOnClick([&, book, itemDesc](bnuui::Element& e) {
        if(book->visible){
            itemDesc->texture = TEXTURE_ASSET_ID::HEALING_MODULE_TEXT;
            itemDesc->visible = true;
        }
    });

    module_icon5->setOnClick([&, book, itemDesc](bnuui::Element& e) {
        if(book->visible){
            itemDesc->texture = TEXTURE_ASSET_ID::BUBBLE_BUFF_TEXT;
            itemDesc->visible = true;
        }
    });

    module_icon6->setOnClick([&, book, itemDesc](bnuui::Element& e) {
        if(book->visible){
            itemDesc->texture = TEXTURE_ASSET_ID::STEERING_WHEEL_TEXT;
            itemDesc->visible = true;
        }
    });

    module_icon7->setOnClick([&, book, itemDesc](bnuui::Element& e) {
        if(book->visible){
            itemDesc->texture = TEXTURE_ASSET_ID::BUNNY_INDICATOR_TEXT;
            itemDesc->visible = true;
        }
    });

    module_icon8->setOnClick([&, book, itemDesc](bnuui::Element& e) {
        if(book->visible){
            itemDesc->texture = TEXTURE_ASSET_ID::HOME_INDICATOR_TEXT;
            itemDesc->visible = true;
        }
    });

    module_icon9->setOnClick([&, book, itemDesc](bnuui::Element& e) {
        if(book->visible){
            itemDesc->texture = TEXTURE_ASSET_ID::SHIP_TEXT;
            itemDesc->visible = true;
        }
    });


    book->children.push_back(moduletype_text);
    book->children.push_back(moduledesc_text);
    book->children.push_back(module_iconBox1);
    book->children.push_back(module_iconBox2);
    book->children.push_back(module_iconBox3);
    book->children.push_back(module_iconBox4);
    book->children.push_back(module_iconBox5);
    book->children.push_back(module_iconBox6);
    book->children.push_back(module_iconBox7);
    book->children.push_back(module_iconBox8);
    book->children.push_back(module_iconBox9);
    book->children.push_back(itemDesc);


    scene_ui.insert(book_icon);
    scene_ui.insert(book);
    scene_ui.insert(module_icon1);
    scene_ui.insert(module_icon2);
    scene_ui.insert(module_icon3);
    scene_ui.insert(module_icon4);
    scene_ui.insert(module_icon5);


    this->book = book;
    this->book_icon = book_icon;

    scene_ui.insert(module_icon6);
    scene_ui.insert(module_icon7);
    scene_ui.insert(module_icon8);
    scene_ui.insert(module_icon9);

}

void GameLevel::InitializeBunnySavingUI() {
    // Create the Remaining Bunny UI.
    auto bunny_ctr_box = std::make_shared<bnuui::Box>(vec2(WINDOW_WIDTH_PX-96+20, 96-20), vec2(60, 80), 0.0f);
    auto bunny_icon = std::make_shared<bnuui::Box>(vec2(WINDOW_WIDTH_PX-96+20, 82.5f-20), vec2(30, 30), 0.0f);
    bunny_icon->texture = TEXTURE_ASSET_ID::BUNNY_NPC_FACE;
    bunny_ctr_box->children.push_back(bunny_icon);

    auto ctr_text = std::make_shared<bnuui::TextLabel>(vec2(WINDOW_WIDTH_PX-108.0f+20,115-20), 1, "0/0");
    ctr_text->setOnUpdate([&](bnuui::Element& e, float dt) {  
        int free_bunny = registry.base.components[0].bunny_count;
        std::string s = std::to_string(free_bunny) + "/" + std::to_string(bunnies_to_win);
        static_cast<bnuui::TextLabel&>(e).setText(s);
    });

    auto info_box = std::make_shared<bnuui::LongBox>(vec2(WINDOW_WIDTH_PX-170+20, 96*2-20), vec2(240, 72), 0.0f);
    info_box->setOnUpdate([bunny_ctr_box](bnuui::Element& e, float dt) {
        if (bunny_ctr_box->hovering) {
            e.visible = true;
        } else {
            e.visible = false;
        }
    });

    auto bunnies_in_module = std::make_shared<bnuui::TextLabel>(vec2(WINDOW_WIDTH_PX-250.0f+20,185-20), 1, "OH NO");
    bunnies_in_module->setOnUpdate([](bnuui::Element& e, float dt) {
        int num = 0;
        for (Bunny b : registry.bunnies.components) {
            if (b.on_module)
                num++;
        }
        static_cast<bnuui::TextLabel&>(e).setText("Bunnies On module: " + std::to_string(num));
    });
    auto bunnies_in_idle = std::make_shared<bnuui::TextLabel>(vec2(WINDOW_WIDTH_PX-250.0f+20,205-20), 1, "OH NO");
    bunnies_in_idle->setOnUpdate([](bnuui::Element& e, float dt) {
        int num = 0;
        for (Bunny b : registry.bunnies.components) {
            if (b.on_ship)
                num++;
        }
        static_cast<bnuui::TextLabel&>(e).setText("Bunnies Idling: " + std::to_string(num));
    });
    info_box->children.push_back(bunnies_in_module);
    info_box->children.push_back(bunnies_in_idle);

    scene_ui.insert(bunny_ctr_box);
    scene_ui.insert(ctr_text);
    scene_ui.insert(info_box);
}

void GameLevel::InitializeUI() {
    // Create Healthbar.
    auto player_box = std::make_shared<bnuui::Box>(vec2(96-20, 96-20), vec2(96, 96), 0.0f);
    auto player_status = std::make_shared<bnuui::PlayerStatus>(
        vec2(96-20, 96-20), vec2(60, 60), 0.0f, registry.ships.components[0].health, registry.ships.components[0].maxHealth);
    auto slider_bg = std::make_shared<bnuui::LongBox>(vec2(256-20, 96-20), vec2(240, 72), 0.0f);
    auto progress_bar = std::make_shared<bnuui::ProgressBar>(
        vec2(256-20, 93-20), vec2(180, 24), 0.0f, registry.ships.components[0].health, registry.ships.components[0].maxHealth);
    player_box->children.push_back(slider_bg);    

    // Create the tile cursor effect.
    tile_cursor = std::make_shared<bnuui::Cursor>(vec2(0, 0), vec2(GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX), 0.0f);
    tile_cursor->visible = false;

    InitializeTrackingUI();
    InitializeBunnySavingUI();

    // Insert all the stuff.
    scene_ui.insert(player_box);
    scene_ui.insert(player_status);
    scene_ui.insert(progress_bar);
    scene_ui.insert(tile_cursor);

    InitializeBookUI();
    InitializePauseUI();
}

void GameLevel::Exit() {
    RenderSystem::isInGame = false;
    RenderSystem::isPaused = false;
    RenderSystem::isRenderingBook = false;
    RenderSystem::isRenderingGacha = false;

    scene_ui.clear();
    CameraSystem* cs = CameraSystem::GetInstance();
    cs->position = {0.0f, 0.0f};
    cs->prev_pos = {0.0f, 0.0f};
    cs->vel = {0.0f, 0.0f};
    // Delete all components and stuff from this scene.

    while (registry.cameras.entities.size() > 0){
	    registry.remove_all_components_of(registry.cameras.entities.back());
	}
    while (registry.renderRequests.entities.size() > 0){
	    registry.remove_all_components_of(registry.renderRequests.entities.back());
	}
    while (registry.renderLayers.entities.size() > 0) {
        registry.remove_all_components_of(registry.renderLayers.entities.back());
    }
    while (registry.colors.entities.size() > 0){
	    registry.remove_all_components_of(registry.colors.entities.back());
	}
    while (registry.players.entities.size() > 0){
	    registry.remove_all_components_of(registry.players.entities.back());
	}
    while (registry.motions.entities.size() > 0){
	    registry.remove_all_components_of(registry.motions.entities.back());
	}
    while (registry.playerAnimations.entities.size() > 0){
	    registry.remove_all_components_of(registry.playerAnimations.entities.back());
	}
    while (registry.ships.entities.size() > 0){
	    registry.remove_all_components_of(registry.ships.entities.back());
	}
    while (registry.backgroundObjects.entities.size() > 0){
	    registry.remove_all_components_of(registry.backgroundObjects.entities.back());
	}
    while (registry.enemies.entities.size() > 0){
	    registry.remove_all_components_of(registry.enemies.entities.back());
	}
    while (registry.enemySpawners.entities.size() > 0) {
        registry.remove_all_components_of(registry.enemySpawners.entities.back());
    }
    while (registry.islands.entities.size() > 0){
	    registry.remove_all_components_of(registry.islands.entities.back());
	}
    while (registry.base.entities.size() > 0){
	    registry.remove_all_components_of(registry.base.entities.back());
	}
    while (registry.bunnies.entities.size() > 0){
	    registry.remove_all_components_of(registry.bunnies.entities.back());
	}
    while (registry.walkingPaths.entities.size() > 0){
        registry.remove_all_components_of(registry.walkingPaths.entities.back());
    }
    while (registry.filledTiles.entities.size() > 0){
        registry.remove_all_components_of(registry.filledTiles.entities.back());
    }
    while (registry.steeringWheels.entities.size() > 0){
        registry.remove_all_components_of(registry.steeringWheels.entities.back());
    }
    while (registry.simpleCannons.entities.size() > 0){
        registry.remove_all_components_of(registry.simpleCannons.entities.back());
    }
    while (registry.cannonModifiers.entities.size() > 0){
        registry.remove_all_components_of(registry.cannonModifiers.entities.back());
    }
    while (registry.laserWeapons.entities.size() > 0){
        registry.remove_all_components_of(registry.laserWeapons.entities.back());
    }
    while (registry.healModules.entities.size() > 0) {
        registry.remove_all_components_of(registry.healModules.entities.back());
    }
    while (registry.laserBeams.entities.size() > 0){
        registry.remove_all_components_of(registry.laserBeams.entities.back());
    }
    while (registry.sounds.entities.size() > 0){
        registry.remove_all_components_of(registry.sounds.entities.back());
    }
    while (registry.disasters.entities.size() > 0){
        registry.remove_all_components_of(registry.disasters.entities.back());
    }
    while (registry.helperBunnyIcons.entities.size() > 0){
        registry.remove_all_components_of(registry.helperBunnyIcons.entities.back());
    }
    
    // while (registry.projectiles.entities.size() > 0){
    //     registry.remove_all_components_of(registry.projectiles.entities.back());
    // }
    LevelExit();
}

void HandlePlayerMovement(int key, int action, int mod) {
    assert(registry.players.size() == 1);
    Entity player = registry.players.entities[0];
    Player& player_comp = registry.players.get(player);
    Motion& mot = registry.motions.get(player);

    // Prevent player from moving when they're stationing.
    if (player_comp.player_state == PLAYERSTATE::STATIONING) return;

    if (action == GLFW_PRESS) {
        if (!activeKeys.count(key)) {
            keyOrder.push_back(key);
        }
        activeKeys.insert(key);
    } else if (action == GLFW_RELEASE) {
        activeKeys.erase(key);
        keyOrder.erase(std::remove(keyOrder.begin(), keyOrder.end(), key), keyOrder.end());
    }

    float velocityX = 0.0f;
    float velocityY = 0.0f;

    if (activeKeys.count(MOVE_UP_BUTTON)) velocityY -= WALK_SPEED;
    if (activeKeys.count(MOVE_DOWN_BUTTON)) velocityY += WALK_SPEED;
    if (activeKeys.count(MOVE_LEFT_BUTTON)) velocityX -= WALK_SPEED;
    if (activeKeys.count(MOVE_RIGHT_BUTTON)) velocityX += WALK_SPEED;

    velocityX = std::clamp(velocityX, -WALK_SPEED, WALK_SPEED);
    velocityY = std::clamp(velocityY, -WALK_SPEED, WALK_SPEED);

    mot.velocity = vec2(velocityX, velocityY);

    // Update the player state.
    if (activeKeys.empty() || ((!activeKeys.count(MOVE_UP_BUTTON)) && (!activeKeys.count(MOVE_DOWN_BUTTON)) &&
                               (!activeKeys.count(MOVE_LEFT_BUTTON)) && (!activeKeys.count(MOVE_RIGHT_BUTTON)))) {
        player_comp.player_state = PLAYERSTATE::IDLE;
    } else {
        player_comp.player_state = PLAYERSTATE::WALKING;

        // Determine direction based on last key pressed
        if (!keyOrder.empty()) {
            int lastKey = keyOrder.back();
            if (lastKey == MOVE_UP_BUTTON) {
                player_comp.direction = UP;
            } else if (lastKey == MOVE_DOWN_BUTTON) {
                player_comp.direction = DOWN;
            } else if (lastKey == MOVE_LEFT_BUTTON) {
                player_comp.direction = LEFT;
            } else if (lastKey == MOVE_RIGHT_BUTTON) {
                player_comp.direction = RIGHT;
            }
        }
    } 
}

void UpdateCameraMoveDirection(int key, int action, int mod){
    if (action == GLFW_PRESS) {
        if (!activeShipKeys.count(key)) {
            keyShipOrder.push_back(key);
        }
        activeShipKeys.insert(key);
        Camera& c  = registry.cameras.components[0];
        if (activeShipKeys.count(MOVE_UP_BUTTON) || activeShipKeys.count(MOVE_DOWN_BUTTON)) c.applyFrictionY = false;
        if (activeShipKeys.count(MOVE_LEFT_BUTTON) || activeShipKeys.count(MOVE_RIGHT_BUTTON)) c.applyFrictionX = false;
    } else if (action == GLFW_RELEASE) {
        activeShipKeys.erase(key);
        keyShipOrder.erase(std::remove(keyShipOrder.begin(), keyShipOrder.end(), key), keyShipOrder.end());
    }

    // If no keys are pressed, don't reset velocity
    if (activeShipKeys.empty() || (activeShipKeys.size() == 1 && activeShipKeys.count(GLFW_KEY_SPACE))) return;   
}

void HandleCameraMovement() {
    if (activeShipKeys.empty() || (activeShipKeys.size() == 1 && activeShipKeys.count(GLFW_KEY_SPACE))) return;
    CameraSystem* camera = CameraSystem::GetInstance();

    vec2 inputVel = {0.0f, 0.0f};

    if (activeShipKeys.count(MOVE_UP_BUTTON)) inputVel.y += 1;
    if (activeShipKeys.count(MOVE_DOWN_BUTTON)) inputVel.y -= 1;
    if (activeShipKeys.count(MOVE_LEFT_BUTTON)) inputVel.x += 1;
    if (activeShipKeys.count(MOVE_RIGHT_BUTTON)) inputVel.x -= 1;

    if(inputVel.x != 0 || inputVel.y != 0){
        float length = std::sqrt(inputVel.x* inputVel.x + inputVel.y * inputVel.y);
        if (length > 0){
            inputVel.x /= length;
            inputVel.y /= length;
        }
    }

    inputVel.x *= SHIP_CAMERA_SPEED;
    inputVel.y *= SHIP_CAMERA_SPEED;

    inputVel.x = std::clamp(inputVel.x, -WALK_SPEED, WALK_SPEED);
    inputVel.y = std::clamp(inputVel.y, -WALK_SPEED, WALK_SPEED);

    // Apply input to velocity directly instead of setting it
    camera->vel = inputVel;
}

// tile_pos is the player's tile position when pressing SPACE.
void HandlePlayerStationing(vec2 tile_pos) {
    activeShipKeys.clear();
    if (tile_pos.x < 0 || tile_pos.x >= COL_COUNT || tile_pos.y < 0 || tile_pos.y >= ROW_COUNT) return;

    Player& player_comp = registry.players.components[0];
    if (player_comp.player_state == STATIONING) {
        // Unstation and allow movement.
        player_comp.player_state = IDLE;
        return;
    }

    MODULE_TYPES types = getModuleType(tile_pos);
    switch (types) {
        case EMPTY:
        case PLATFORM:
            return;
        default:
            player_comp.player_state = STATIONING;
    }
    player_comp.player_state = STATIONING;
}

void GameLevel::HandleInput(int key, int action, int mod) {
    // exit game w/ ESC
    if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
        RenderSystem::isPaused = !RenderSystem::isPaused;
    }
    if (RenderSystem::isPaused) return;

    Entity player = registry.players.entities[0];

    glm::vec2 playerPos = registry.motions.get(player).position;
    int player_tile_x = (int) (playerPos.x / GRID_CELL_WIDTH_PX);
    int player_tile_y = (int) (playerPos.y / GRID_CELL_HEIGHT_PX);

    Player& player_comp = registry.players.get(player);

    if ((action == GLFW_RELEASE) && (key == GLFW_KEY_K)) {
        for (Entity e : registry.helperBunnyIcons.entities) {
            HelperBunnyIcon& icon = registry.helperBunnyIcons.get(e);
            std::cout << "icon: " << icon.tile_pos.x << ' ' << icon.tile_pos.y << '\n'; 
        }
    }


    // Build Mode.
    if ((action == GLFW_RELEASE) && (key == GLFW_KEY_E)) {
        if(RenderSystem::isRenderingBook){
            return;
        }
        if (player_comp.player_state == BUILDING) {
            inventory_system.CloseInventory();
            player_comp.player_state = IDLE;
        } else {
            player_comp.player_state = BUILDING;
            inventory_system.OpenInventory();
        }
        return;
    }

    if ((action == GLFW_RELEASE) && (key == GLFW_KEY_SPACE) &&
        (player_comp.player_state == IDLE || player_comp.player_state == STATIONING)) {
        HandlePlayerStationing(vec2(player_tile_x, player_tile_y));
    }

    if (player_comp.player_state == STATIONING) {
        // Check what module the player is on.
        MODULE_TYPES module = getModuleType(vec2(player_tile_x, player_tile_y));
        switch (module) {
            case EMPTY:
            case PLATFORM: {
                // This shouldn't happen but if it does, then we resolve by making the player back to idle.
                player_comp.player_state = IDLE;
                return;
            }
            case STEERING_WHEEL: {
                UpdateCameraMoveDirection(key, action, mod);
                return;
            }
            default:
                return;
        }
    } else if (player_comp.player_state != BUILDING && !RenderSystem::isRenderingBook) {
        HandlePlayerMovement(key, action, mod);
    }

    LevelHandleInput(key, action, mod);
}

void GameLevel::HandleMouseMove(vec2 pos) {
    l1_mouse_pos = pos;
    
    // Check if paused
    if (RenderSystem::isPaused) {
        std::vector<std::shared_ptr<bnuui::Element>> pause_elems = scene_ui.getPauseUI()->children;
        for (std::shared_ptr<bnuui::Element> ui_elem : pause_elems) {
            if (ui_elem->isPointColliding(pos)) {
                ui_elem->hovering = true;
            } else {
                ui_elem->hovering = false;
            }
        }
        return;
    }

    // Check if hovering over any UI components.
    std::vector<std::shared_ptr<bnuui::Element>> ui_elems = scene_ui.getElems();
    for (std::shared_ptr<bnuui::Element> ui_elem : ui_elems) {
        if (ui_elem->isPointColliding(pos)) {
            ui_elem->hovering = true;
        } else {
            ui_elem->hovering = false;
        }
    }

    // Check if hovering over an interactable tile. IF SO, then place a cute cursor.
    int mouse_tile_x = (int) (pos.x / GRID_CELL_WIDTH_PX);
    int mouse_tile_y = (int) (pos.y / GRID_CELL_HEIGHT_PX);

    // Check if in the grid
    if (mouse_tile_x > 0 && mouse_tile_x < COL_COUNT && mouse_tile_y > 0 && mouse_tile_y < ROW_COUNT) {
        MODULE_TYPES module = getModuleType(vec2(mouse_tile_x, mouse_tile_y));
        if (module != EMPTY) {
            tile_cursor->visible = true;
            tile_cursor->position = TileToVector2(mouse_tile_x, mouse_tile_y);
        } else {
            tile_cursor->visible = false;
        }
    }

    Entity player_entity = registry.players.entities[0];
    glm::vec2 playerPos = registry.motions.get(player_entity).position;
    int player_tile_x = (int) (playerPos.x / GRID_CELL_WIDTH_PX);
    int player_tile_y = (int) (playerPos.y / GRID_CELL_HEIGHT_PX);

    Player& player_comp = registry.players.get(player_entity);

    if (player_comp.player_state == STATIONING) {
        // Check what module the player is on.
        Ship& ship = registry.ships.components[0];
        MODULE_TYPES module = getModuleType(vec2(player_tile_x, player_tile_y));
        switch (module) {
            case SIMPLE_CANNON: {
                // Rotate the simple cannon.
                Entity cannon_entity = ship.ship_modules_entity[player_tile_y][player_tile_x];
                Motion& m = registry.motions.get(cannon_entity);
                m.angle = degrees(atan2(pos.y - m.position.y, pos.x - m.position.x)) + 90.0f;
                return;
            }
            case LASER_WEAPON: {
                Entity laser_entity = ship.ship_modules_entity[player_tile_y][player_tile_x];
                Motion& m = registry.motions.get(laser_entity);
                m.angle = degrees(atan2(pos.y - m.position.y, pos.x - m.position.x)) + 90.0f;
                return; 
            }
            case BUBBLE_MOD: {
                Entity cannon_entity = ship.ship_modules_entity[player_tile_y][player_tile_x];
                Motion& m = registry.motions.get(cannon_entity);
                m.angle = degrees(atan2(pos.y - m.position.y, pos.x - m.position.x)) + 90.0f;
                return;
            }
            default:
                return;
        }
    }

    LevelHandleMouseMove(pos);
}

// Stations the first bunny that's unstationed.
void stationBunny(vec2 tile_pos) {
    // Set one of the bunny to be on_module
    for (Bunny& b : registry.bunnies.components) {
        if (b.on_ship && !b.on_module && !b.on_base) {
            b.on_ship = false;
            b.on_module = true;
            createBunnyIcon(tile_pos);
            return; 
        }
    }
}

// Unstations the first stationed bunny.
void unStationBunny(vec2 tile_pos) {
    std::cout << "Unstation bunny pos: " << tile_pos.x << ' ' << tile_pos.y << '\n';
    // Set one of the bunny to be on_module
    for (Bunny& b : registry.bunnies.components) {
        if (b.on_module) {
            b.on_ship = true;
            b.on_module = false;
            break;
        }
    }
    // Find the bunny icon module and remove it.
    for (Entity e : registry.helperBunnyIcons.entities) {
        HelperBunnyIcon& icon = registry.helperBunnyIcons.get(e);
        std::cout << "icon: " << icon.tile_pos.x << ' ' << icon.tile_pos.y << '\n';
        if (icon.tile_pos == tile_pos) {
            registry.remove_all_components_of(e);
            return;
        }
    }
}

void GameLevel::HandleMouseClick(int button, int action, int mods) {
    if (RenderSystem::isPaused) {
        std::vector<std::shared_ptr<bnuui::Element>> ui_elems = scene_ui.getPauseUI()->children;
        if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_1) {
            for (std::shared_ptr<bnuui::Element> ui_elem : ui_elems) {
                if (ui_elem->hovering) {
                    ui_elem->active = true;
                } else {
                    ui_elem->active = false;
                }
            }
        } else if (action == GLFW_RELEASE) {
            for (std::shared_ptr<bnuui::Element> ui_elem : ui_elems) {
                if (ui_elem->active) {
                    ui_elem->clickButton();
                }
                ui_elem->active = false;
            }
        } 
        return;
    }
    // Check if hovering over any UI components.
    std::vector<std::shared_ptr<bnuui::Element>> ui_elems = scene_ui.getElems();
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_1) {
        for (std::shared_ptr<bnuui::Element> ui_elem : ui_elems) {
            if (ui_elem->hovering) {
                ui_elem->active = true;
            } else {
                ui_elem->active = false;
            }
        }
    } else if (action == GLFW_RELEASE) {
        for (std::shared_ptr<bnuui::Element> ui_elem : ui_elems) {
            if (ui_elem->active) { 
                ui_elem->clickButton();
            }
            ui_elem->active = false;
        }
    }

    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_1) {
        // Handles mouse clicks for different stationings.
        Entity player_entity = registry.players.entities[0];
        glm::vec2 playerPos = registry.motions.get(player_entity).position;
        int player_tile_x = (int) (playerPos.x / GRID_CELL_WIDTH_PX);
        int player_tile_y = (int) (playerPos.y / GRID_CELL_HEIGHT_PX);

        Player& player_comp = registry.players.get(player_entity);

        if (player_comp.player_state == STATIONING) {
            // Check what module the player is on.
            Ship& ship = registry.ships.components[0];
            MODULE_TYPES module = ship.ship_modules[player_tile_y][player_tile_x];
            switch (module) {
                case SIMPLE_CANNON: {
                    // Rotate the simple cannon.
                    Entity cannon_entity = ship.ship_modules_entity[player_tile_y][player_tile_x];
                    SimpleCannon& sc = registry.simpleCannons.get(cannon_entity);
                    if (sc.timer_ms <= 0) {
                        vec2 cannon_pos = registry.motions.get(cannon_entity).position;
                        createCannonProjectile(cannon_pos, l1_mouse_pos);
                        sc.timer_ms = SIMPLE_CANNON_COOLDOWN;
                    }
                    return;
                }
                case LASER_WEAPON: {
                    Entity laser_entity = ship.ship_modules_entity[player_tile_y][player_tile_x];
                    LaserWeapon& lw = registry.laserWeapons.get(laser_entity);
                    if (lw.timer_ms <= 0) {
                        vec2 laser_pos = registry.motions.get(laser_entity).position;
                        createLaserBeam(laser_pos, l1_mouse_pos);
                        lw.timer_ms = LASER_COOLDOWN;
                    }
                    return;
                }
                case HEAL: {
                    Entity heal_entity = ship.ship_modules_entity[player_tile_y][player_tile_x];
                    Heal& healModule = registry.healModules.get(heal_entity);

                    if (healModule.cooldown_ms <= 0) {
                        ship.health = std::min(ship.health + HEAL_AMOUNT, ship.maxHealth);

                        healModule.cooldown_ms = HEAL_COOLDOWN;

                        std::cout << "Ship healed for " << HEAL_AMOUNT << " health points" << std::endl;
                    } else {
                        std::cout << "Healing on cooldown: " << healModule.cooldown_ms / HEAL_COOLDOWN
                                  << " seconds remaining" << std::endl;
                    }
                    return;
                }
                case BUBBLE_MOD: {
                    // Rotate the simple cannon.
                    Entity cannon_entity = ship.ship_modules_entity[player_tile_y][player_tile_x];
                    SimpleCannon& sc = registry.simpleCannons.get(cannon_entity);
                    if (sc.timer_ms <= 0) {
                        vec2 cannon_pos = registry.motions.get(cannon_entity).position;
                        CannonModifier cm = registry.cannonModifiers.get(cannon_entity);
                        createModifiedCannonProjectile(cannon_pos, l1_mouse_pos, cm);
                        sc.timer_ms = SIMPLE_CANNON_COOLDOWN;
                    }
                    return;
                }
                default:
                    return;
            }
        }
    }
    // Place selected items.
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_1 && curr_selected != EMPTY) {
        vec2 tile_pos = getMouseTilePosition();
        MODULE_TYPES module = getModuleType(tile_pos);
        Ship& ship = registry.ships.components[0];
        if (ship.available_modules[curr_selected] == 0) {
            return;
        }

        if (module == PLATFORM && curr_selected != HELPER_BUNNY && curr_selected != BUBBLE_MOD) {
            ship.ship_modules[tile_pos.y][tile_pos.x] = curr_selected;
            Entity e;
            switch (curr_selected) {
                case STEERING_WHEEL: {
                    e = createSteeringWheel(tile_pos);
                    break;
                }
                case SIMPLE_CANNON: {
                    e = createCannon(tile_pos);
                    break;
                }
                case LASER_WEAPON: {
                    e = createLaserWeapon(tile_pos);
                    break;
                }
                case HEAL: {
                    e = createHealModule(tile_pos);
                    break;
                }
                default:
                    break;
            }
            ship.ship_modules_entity[tile_pos.y][tile_pos.x] = e;
            ship.available_modules[curr_selected]--;
        } else if (curr_selected == HELPER_BUNNY && module != EMPTY && module != PLATFORM) {
            switch (module) {
                case SIMPLE_CANNON: {
                    SimpleCannon& sc = registry.simpleCannons.get(ship.ship_modules_entity[tile_pos.y][tile_pos.x]);
                    if (!sc.is_automated) {
                        sc.is_automated = true;
                        ship.available_modules[curr_selected]--;
                        stationBunny(tile_pos);
                    }
                    break;
                }
                case LASER_WEAPON: {
                    LaserWeapon& lw = registry.laserWeapons.get(ship.ship_modules_entity[tile_pos.y][tile_pos.x]);
                    if (!lw.is_automated) {
                        lw.is_automated = true;
                        ship.available_modules[curr_selected]--;
                        stationBunny(tile_pos);
                    }
                    break;
                }
                case HEAL: {
                    Heal& heal = registry.healModules.get(ship.ship_modules_entity[tile_pos.y][tile_pos.x]);
                    if (!heal.is_automated) {
                        heal.is_automated = true;
                        ship.available_modules[curr_selected]--;
                        stationBunny(tile_pos);
                    }
                    break;
                }
                case BUBBLE_MOD: {
                    SimpleCannon& sc = registry.simpleCannons.get(ship.ship_modules_entity[tile_pos.y][tile_pos.x]);
                    if (!sc.is_automated) {
                        sc.is_automated = true;
                        ship.available_modules[curr_selected]--;
                        stationBunny(tile_pos);
                    }
                    break;
                }


                default: {
                    break;
                }
            }
        } else if (curr_selected == BUBBLE_MOD && module != EMPTY && module != PLATFORM) {
            switch (module) {
                case SIMPLE_CANNON: {
                    SimpleCannon& sc = registry.simpleCannons.get(ship.ship_modules_entity[tile_pos.y][tile_pos.x]);
                    if (!sc.is_modified) {
                        ship.available_modules[curr_selected]--;
                        modifyCannon(ship.ship_modules_entity[tile_pos.y][tile_pos.x], BUBBLE);
                        ship.ship_modules[tile_pos.y][tile_pos.x] = BUBBLE_MOD;
                    }
                    break;
                }
                case LASER_WEAPON: {
                    break;
                }
                case HEAL: {
                    break;
                }

                default: {
                    break;
                }
            }
        
        
        }
    }

    // Remove on right click.
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_2 && registry.players.components[0].player_state == BUILDING) {
        vec2 tile_pos = getMouseTilePosition();
        MODULE_TYPES module = getModuleType(tile_pos);
        Ship& ship = registry.ships.components[0];
        switch (module) {
            case EMPTY:
            case PLATFORM:
                return;
            case SIMPLE_CANNON: {
                Entity& cannon_entity = ship.ship_modules_entity[tile_pos.y][tile_pos.x];
                SimpleCannon& sc = registry.simpleCannons.get(cannon_entity);
                if (sc.is_automated) {
                    ship.available_modules[HELPER_BUNNY]++;
                    sc.is_automated = false;
                    unStationBunny(tile_pos);
                    break;
                }

                if (sc.is_modified) {
                    ship.available_modules[BUBBLE_MOD]++;
                    sc.is_modified = false;
                    registry.cannonModifiers.remove(cannon_entity);
                    
                    TEXTURE_ASSET_ID& texture = registry.renderRequests.get(cannon_entity).used_texture;
                    texture = TEXTURE_ASSET_ID::SIMPLE_CANNON01;
                    break;
                }

                RemoveStation(tile_pos, module); 
                break;
            }
            case LASER_WEAPON: {
                LaserWeapon& lw = registry.laserWeapons.get(ship.ship_modules_entity[tile_pos.y][tile_pos.x]);
                if (lw.is_automated) {
                    ship.available_modules[HELPER_BUNNY]++;
                    lw.is_automated = false;
                    unStationBunny(tile_pos);
                    break;
                }
                RemoveStation(tile_pos, module);
                break;
            }
            case HEAL: {
                Heal& hm = registry.healModules.get(ship.ship_modules_entity[tile_pos.y][tile_pos.x]);
                if (hm.is_automated) {
                    ship.available_modules[HELPER_BUNNY]++;
                    hm.is_automated = false;
                    unStationBunny(tile_pos);
                    break;
                }
                RemoveStation(tile_pos, module);
                break;
            }
            case BUBBLE_MOD: {
                Entity& cannon_entity = ship.ship_modules_entity[tile_pos.y][tile_pos.x];
                SimpleCannon& sc = registry.simpleCannons.get(cannon_entity);
                if (sc.is_automated) {
                    ship.available_modules[HELPER_BUNNY]++;
                    sc.is_automated = false;
                    unStationBunny(tile_pos);
                    break;
                } else {
                    ship.available_modules[BUBBLE_MOD]++;
                    sc.is_modified = false;
                    registry.cannonModifiers.remove(cannon_entity);
                    
                    TEXTURE_ASSET_ID& texture = registry.renderRequests.get(cannon_entity).used_texture;
                    texture = TEXTURE_ASSET_ID::SIMPLE_CANNON01;
                    ship.ship_modules[tile_pos.y][tile_pos.x] = MODULE_TYPES::SIMPLE_CANNON;
                    break;
                }

                RemoveStation(tile_pos, module); 
                break;
            }
            default: {
                RemoveStation(tile_pos, module);
            }
        }
        // SUPER LAZY AND BAD WAY BUT IM LAZYYYY
        inventory_system.CloseInventory();
        inventory_system.OpenInventory();
    }

    LevelHandleMouseClick(button, action, mods);
}


/* Update the four lines of the progress bar
* 
*   when a side is full, we add half the width to the length
    so there are no gaps in the corners

    setting end_pos to (0, 0) effectively makes it invisible
*/
void GameLevel::UpdateDropoffProgressBar() {
    int bunnies_on_ship = 0;
    for (Bunny b : registry.bunnies.components) {
        if (b.on_ship) bunnies_on_ship++;
    }
    
    assert(registry.base.entities.size() == 1 && base_corners.size() == 4);
    Entity& base_entity = registry.base.entities[0];
    Motion& base_motion = registry.motions.get(base_entity);

    // percentage value from 0 to 1 of dropoff time
    float dropoffProgress = registry.base.get(base_entity).drop_off_timer / BUNNY_BASE_DROPOFF_TIME;

    float base_width = base_motion.scale.x, base_height = base_motion.scale.y;
    float base_perimeter = 2 * (base_width + base_height);

    std::vector<tson::Vector2i> original_corners = get_poly_from_motion(base_motion);

    GridLine& lineUL = registry.gridLines.get(base_corners[0]);
    GridLine& lineUR = registry.gridLines.get(base_corners[1]);
    GridLine& lineBR = registry.gridLines.get(base_corners[2]);
    GridLine& lineBL = registry.gridLines.get(base_corners[3]);

    auto setLine = [](GridLine& line, vec2 start, vec2 end) {
        line.start_pos = start;
        line.end_pos = end;
    };

    // percentage thresholds for each side
    float thresholds[] = {base_width / base_perimeter,
                          (base_width + base_height) / base_perimeter,
                          (2 * base_width + base_height) / base_perimeter,
                          1.0f};

    float new_length = dropoffProgress * base_perimeter;

    setLine(lineUL,
            {original_corners[0].x + std::min(new_length, base_width) / 2, original_corners[0].y},
            {std::min(new_length, base_width) + PROGRESS_BAR_LINE_WIDTH_PX / 2, PROGRESS_BAR_LINE_WIDTH_PX});
    setLine(
        lineUR,
        (dropoffProgress > thresholds[0])
            ? vec2(original_corners[1].x, original_corners[1].y + std::min(new_length - base_width, base_height) / 2)
            : vec2(0, 0),
        (dropoffProgress > thresholds[0])
            ? vec2(PROGRESS_BAR_LINE_WIDTH_PX, std::min(new_length - base_width, base_height))
            : vec2(0, 0));
    setLine(lineBR,
            (dropoffProgress > thresholds[1])
                ? vec2(original_corners[2].x - std::min(new_length - base_width - base_height, base_width) / 2,
                       original_corners[2].y)
                : vec2(0, 0),
            (dropoffProgress > thresholds[1])
                ? vec2(std::min(new_length - base_width - base_height, base_width) + PROGRESS_BAR_LINE_WIDTH_PX / 2,
                       PROGRESS_BAR_LINE_WIDTH_PX)
                : vec2(0, 0));
    setLine(lineBL,
            (dropoffProgress > thresholds[2])
                ? vec2(original_corners[3].x,
                       original_corners[3].y - std::min(new_length - 2 * base_width - base_height, base_height) / 2)
                : vec2(0, 0),
            (dropoffProgress > thresholds[2])
                ? vec2(PROGRESS_BAR_LINE_WIDTH_PX, std::min(new_length - 2 * base_width - base_height, base_height))
                : vec2(0, 0));
}


void GameLevel::RemoveStation(vec2 tile_pos, MODULE_TYPES module){
    Ship& ship = registry.ships.components[0];
    ship.ship_modules[tile_pos.y][tile_pos.x] = PLATFORM;
    registry.remove_all_components_of(ship.ship_modules_entity[tile_pos.y][tile_pos.x]);
    ship.available_modules[module]++; 
}


void GameLevel::Update(float dt) {
    if(!RenderSystem::isRenderingGacha && 
        registry.players.components[0].player_state != BUILDING && 
        !RenderSystem::isRenderingBook &&
        !RenderSystem::isPaused){
        CameraSystem::GetInstance()->update(dt);
        ai_system.step(dt);
        physics_system.step(dt);
        animation_system.step(dt);
        module_system.step(dt);
        particle_system.step(dt);
        HandleCameraMovement();

        world_system->handle_collisions();

        // Remove projectiles.
        for (Entity e : registry.playerProjectiles.entities) {
            if (registry.playerProjectiles.has(e)) {
                PlayerProjectile& p = registry.playerProjectiles.get(e);
                if (p.alive_time_ms <= 0) {
                    // std::cout << "removing projectile" << std::endl;
                    registry.remove_all_components_of(e);
                    continue;
                }
                p.alive_time_ms -= dt;
            }
        }

        for (Entity e : registry.enemyProjectiles.entities) {
            if (registry.enemyProjectiles.has(e)) {
                EnemyProjectile& p = registry.enemyProjectiles.get(e);
                if (p.alive_time_ms <= 0) {
                    registry.remove_all_components_of(e);
                    continue;
                }
                p.alive_time_ms -= dt;
            }
        }

        for (Entity e : registry.laserBeams.entities) {
            if (registry.laserBeams.has(e)) {
                LaserBeam& l = registry.laserBeams.get(e);
                if (l.alive_time_ms <= 0) {
                    registry.remove_all_components_of(e);
                    continue;
                }
                l.alive_time_ms -= dt;
            }
        }

        // Remove disasters.
        for (Entity e : registry.disasters.entities) {
            Disaster& d = registry.disasters.get(e);
            if (d.type == DISASTER_TYPE::TORNADO) {
                if (d.alive_time_ms <= 0) {
                    std::cout << "remove tornado" << std::endl;
                    registry.remove_all_components_of(e);
                    continue;
                }
                d.alive_time_ms -= dt;
            }
        }

        registry.ships.components[0].available_modules[HELPER_BUNNY] = 0;
        for (Bunny& bunny : registry.bunnies.components) {
            if (bunny.on_ship) {
                registry.ships.components[0].available_modules[HELPER_BUNNY]++;
            }
        }
    }

    UpdateDropoffProgressBar();


    LevelUpdate(dt);
    scene_ui.update(dt);
}

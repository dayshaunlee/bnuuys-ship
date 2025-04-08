#include "modules_system.hpp"
#include "camera_system.hpp"
#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/entity.hpp"
#include "tinyECS/registry.hpp"
#include "world_init.hpp"
#include "gacha_system.hpp"
#include "sceneManager/scene_manager.hpp"
#include "render_system.hpp"

GachaSystem& GachaSystem::getInstance() {
    static GachaSystem instance;
    return instance;
}

// Private constructor
GachaSystem::GachaSystem() {
    rng.seed(static_cast<unsigned>(time(nullptr)));
    levelModulePools.resize(5); // Set to 5 levels for now
    
    // Dayshaun: put level pool loading in each level.cpp file
    
    setDropRate(MODULE_TYPES::STEERING_WHEEL, 0);
    setDropRate(MODULE_TYPES::HELPER_BUNNY, 0);
    setDropRate(MODULE_TYPES::EMPTY, 0);
    setDropRate(MODULE_TYPES::PLATFORM, 100);
    setDropRate(MODULE_TYPES::SIMPLE_CANNON, 100);
    setDropRate(MODULE_TYPES::LASER_WEAPON, 50);
    setDropRate(MODULE_TYPES::BUBBLE_MOD, 50);
    setDropRate(MODULE_TYPES::HEAL, 50);
}


void GachaSystem::setLevelPool(int level, const std::vector<MODULE_TYPES>& modulesPool){
    if (level >= levelModulePools.size()) {
        levelModulePools.resize(level + 1);
    }
    levelModulePools[level] = std::unordered_set<MODULE_TYPES>(modulesPool.begin(), modulesPool.end());
}

void GachaSystem::setDropRate(MODULE_TYPES module, float dropRate) {
    moduleDropRates[module] = dropRate;
}


std::vector<MODULE_TYPES> GachaSystem::getModuleOptions(int level){
    int numOptions = 3;

    if (level >= levelModulePools.size() || levelModulePools[level].empty()) {
        return {};
    }

    std::vector<MODULE_TYPES> availableModules(levelModulePools[level].begin(), levelModulePools[level].end());
    std::vector<float> weights;

    // Assign weights based on drop rate (default to 1.0 if not set)
    for (MODULE_TYPES module : availableModules) {
        weights.push_back(moduleDropRates.count(module) ? moduleDropRates[module] : 1.0f);
    }

    // Normalize weights to create a probability distribution
    float sumWeights = 0.0f;
    for (float weight : weights) {
        sumWeights += weight;
    }

    for (float& weight : weights) {
        weight /= sumWeights;
    }

    // Weighted random selection
    std::vector<MODULE_TYPES> selectedModules;
    std::discrete_distribution<int> distribution(weights.begin(), weights.end());

    for (int i = 0; i < numOptions; ++i) {
        int index = distribution(rng);
        selectedModules.push_back(availableModules[index]);
    }

    return selectedModules;
}

// bool GachaSystem::isOnDisplay(){
//     return onDisplay;
// }


void clearGatchaUI(bnuui::SceneUI& scene_ui){
    scene_ui.clearGacha();
}

void GachaSystem::handleOptionClick(MODULE_TYPES moduleChose){
    // Play sound
    Entity sound_entity = Entity();
    Sound& sound = registry.sounds.emplace(sound_entity);
    sound.sound_type = SOUND_ASSET_ID::CLICK;
    sound.volume = 30;

    if(moduleChose == MODULE_TYPES::PLATFORM){
        Entity sound_entity = Entity();
        Sound& sound = registry.sounds.emplace(sound_entity);
        sound.sound_type = SOUND_ASSET_ID::RAFT;
        sound.volume = 10;
        expandShip();
        setDropRate(MODULE_TYPES::PLATFORM, 0);
    } else{
        Entity sound_entity = Entity();
        Sound& sound = registry.sounds.emplace(sound_entity);
        sound.sound_type = SOUND_ASSET_ID::MODULE;
        sound.volume = 10;
        registry.ships.components[0].available_modules[moduleChose]++;
    }
}

void GachaSystem::displayGacha(int level, bnuui::SceneUI& scene_ui, GameLevel& currentLevel) {
    RenderSystem::isRenderingGacha = true;
    currentLevel.gacha_called = true;
    
    std::vector<MODULE_TYPES> threeOptions = getModuleOptions(level);

    auto gacha_box = std::make_shared<bnuui::Box>(vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2 + 40), vec2(450, 250), 0.0f);
    auto upgrade_title_box = std::make_shared<bnuui::Box>(vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2 - 50), vec2(350, 115), 0.0f);
    upgrade_title_box->texture = TEXTURE_ASSET_ID::UPGRADE_TITLE;

    scene_ui.insertGacha(gacha_box);
    scene_ui.insertGacha(upgrade_title_box);

    // Tooltip.
    auto tooltip_box = std::make_shared<bnuui::LongBox>(vec2(500, 500), vec2(200, 70), 0.0f);
    auto tooltip_text = std::make_shared<bnuui::TextLabel>(vec2(350,500), 1, "Module Desc...");
    tooltip_box->children.push_back(tooltip_text);

    // Positions for the three module options
    std::array<float, 3> xOffsets = { -100, 0, 100 };
    float yOffset = (float) WINDOW_HEIGHT_PX / 2 + 60;

    for (size_t i = 0; i < threeOptions.size(); ++i) {
        vec2 pos((float) WINDOW_WIDTH_PX / 2 + xOffsets[i], yOffset);
        
        auto moduleBg = std::make_shared<bnuui::Box>(pos, vec2(90, 90), 0.0f);
        auto moduleOption = createModuleOption(threeOptions[i], pos, scene_ui, currentLevel);

        moduleOption->setOnUpdate([&, tooltip_box, i](bnuui::Element& e, float dt) {
            if (e.hovering) {
                e.color = vec3(0.8,0.8,0.8);
                hovered_options[i] = true;
                tooltip_box->position = e.position + vec2(0,90);
            }
            else {
                e.color = vec3(1,1,1);
                hovered_options[i] = false;
            }
        });
        
        scene_ui.insertGacha(moduleBg);
        scene_ui.insertGacha(moduleOption);
    }
    tooltip_box->setOnUpdate([&, tooltip_text](bnuui::Element& e, float dt) {
        tooltip_text->setText(tooltip_str);
        tooltip_text->position = e.position - vec2(e.scale.x/2 - 20.0f, -5.0f);
        for (int i = 0; i < 3; i++) {
            if (hovered_options[i]) {
                e.visible = true;
                return;
            }
        }
        e.visible = false;
    });
    scene_ui.insertGacha(tooltip_box);
}

std::shared_ptr<bnuui::Box> GachaSystem::createModuleOption(
    MODULE_TYPES moduleType, vec2 position, 
    bnuui::SceneUI& scene_ui, GameLevel& currentLevel) {
    
    auto moduleOption = std::make_shared<bnuui::Box>(position, vec2(70, 70), 0.0f);
    moduleOption->texture = getTextureFromModuleType(moduleType);

    moduleOption->setOnClick([&, moduleType](bnuui::Element& e) {
        handleOptionClick(moduleType);
        RenderSystem::isRenderingGacha = false;
        currentLevel.upgradesReceived++;
        currentLevel.gacha_called = false;
        clearGatchaUI(scene_ui);
    });

    moduleOption->setOnHover([&, moduleType](bnuui::Element& e) {
        tooltip_str = getModuleName(moduleType);
    });

    return moduleOption;
}



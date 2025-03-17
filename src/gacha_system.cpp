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

    setLevelPool(0, {MODULE_TYPES::SIMPLE_CANNON, MODULE_TYPES::HELPER_BUNNY, MODULE_TYPES::PLATFORM});
    setLevelPool(1, {MODULE_TYPES::SIMPLE_CANNON, MODULE_TYPES::HELPER_BUNNY, MODULE_TYPES::PLATFORM});
    
    setDropRate(MODULE_TYPES::STEERING_WHEEL, 0);
    setDropRate(MODULE_TYPES::HELPER_BUNNY, 100);
    setDropRate(MODULE_TYPES::EMPTY, 0);
    setDropRate(MODULE_TYPES::PLATFORM, 100);
    setDropRate(MODULE_TYPES::SIMPLE_CANNON, 100);
    setDropRate(MODULE_TYPES::FAST_CANNON, 0);
}


void GachaSystem::setLevelPool(int level, const std::vector<MODULE_TYPES>& modulesPool){
    if (level >= levelModulePools.size()) {
        levelModulePools.resize(level + 1);
    }
    levelModulePools[level] = std::unordered_set<MODULE_TYPES>(modulesPool.begin(), modulesPool.end());
}

void GachaSystem::setDropRate(MODULE_TYPES module, float dropRate) {
    std::cout << "moduleDropRates size before insert: " << moduleDropRates.size() << std::endl;
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

TEXTURE_ASSET_ID getTextureFromModuleType(MODULE_TYPES module){
    switch (module)
    {
    case MODULE_TYPES::SIMPLE_CANNON :
        /* code */
        return TEXTURE_ASSET_ID::SIMPLE_CANNON01;
        break;
    case MODULE_TYPES::FAST_CANNON :
        return TEXTURE_ASSET_ID::SIMPLE_CANNON02; // change in the future
        break;
    case MODULE_TYPES::PLATFORM :
        return TEXTURE_ASSET_ID::RAFT;
        break;
    case MODULE_TYPES::HELPER_BUNNY :
        return TEXTURE_ASSET_ID::BUNNY_NPC_IDLE_UP0;
        break;
    default:
        std::cout << "This is not a valid module" << std::endl;
        return TEXTURE_ASSET_ID::WATER_BACKGROUND; 
        break;
    }
}


void clearGatchaUI(bnuui::SceneUI& scene_ui){
        int uiSize = scene_ui.size();
        scene_ui.remove(uiSize-1);
        scene_ui.remove(uiSize-2);
        scene_ui.remove(uiSize-3);
        scene_ui.remove(uiSize-4);
        scene_ui.remove(uiSize-5);
        scene_ui.remove(uiSize-6);
        scene_ui.remove(uiSize-7);
        scene_ui.remove(uiSize-8);
}

void GachaSystem::handleOptionClick(MODULE_TYPES moduleChose){
    if(moduleChose == MODULE_TYPES::PLATFORM){
        expandShip();
        setDropRate(MODULE_TYPES::PLATFORM, 0);
    } else{
        std::cout<< "HEREEE" <<std::endl;
        std::cout <<"Before:" << registry.ships.components[0].available_modules[moduleChose] << std::endl;
        registry.ships.components[0].available_modules[moduleChose]++;
        std::cout <<"Now:" << registry.ships.components[0].available_modules[moduleChose] << std::endl;
    }
}



void GachaSystem::displayGacha(int level, bnuui::SceneUI& scene_ui, GameLevel &currentLevel){
    // std::cout << "gacha pop up..." << std::endl;
    RenderSystem::isRenderingGacha = true;
    currentLevel.gacha_called = true;
    std::vector<MODULE_TYPES> threeOptions = getModuleOptions(level);
    std::cout << "options" << threeOptions[0] << " " << threeOptions[1] << " " << threeOptions[2] << '\n';

    auto gacha_box = std::make_shared<bnuui::Box>(vec2(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2 + 40), vec2(450, 250), 0.0f);
    auto upgrade_title_box = std::make_shared<bnuui::Box>(vec2(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2 - 50), vec2(350, 115), 0.0f);
    upgrade_title_box->texture = TEXTURE_ASSET_ID::UPGRADE_TITLE;

    auto module1bg = std::make_shared<bnuui::Box>(vec2(((WINDOW_WIDTH_PX/2) - 100), (WINDOW_HEIGHT_PX/2 + 60)), vec2(90, 90), 0.0f);
    auto moduleOption1 = std::make_shared<bnuui::Box>(vec2(((WINDOW_WIDTH_PX/2) - 100), (WINDOW_HEIGHT_PX/2 + 60)), vec2(70, 70), 0.0f);
    moduleOption1->texture = getTextureFromModuleType(threeOptions[0]);
    moduleOption1->setOnClick([&, threeOptions](bnuui::Element& e) {
        handleOptionClick(threeOptions[0]);
        RenderSystem::isRenderingGacha = false;
        currentLevel.upgradesReceived++;
        currentLevel.gacha_called = false;
        clearGatchaUI(scene_ui); 
    });
    moduleOption1->setOnHover([](bnuui::Element& e) { // in the future maybe have on hover effect on the module1bg or something so it looks more interactive
    });

    auto module2bg = std::make_shared<bnuui::Box>(vec2(((WINDOW_WIDTH_PX/2)), (WINDOW_HEIGHT_PX/2 + 60)), vec2(90, 90), 0.0f);
    auto moduleOption2 = std::make_shared<bnuui::Box>(vec2(WINDOW_WIDTH_PX/2, (WINDOW_HEIGHT_PX/2 + 60)), vec2(70, 70), 0.0f);
    moduleOption2->texture = getTextureFromModuleType(threeOptions[1]);
    moduleOption2->setOnClick([&, threeOptions](bnuui::Element& e) {
        handleOptionClick(threeOptions[1]);
        RenderSystem::isRenderingGacha = false;
        currentLevel.upgradesReceived++;
        currentLevel.gacha_called = false;
        clearGatchaUI(scene_ui);
    });
    moduleOption2->setOnHover([](bnuui::Element& e) { 
    });

    auto module3bg = std::make_shared<bnuui::Box>(vec2(((WINDOW_WIDTH_PX/2) +100), ((WINDOW_HEIGHT_PX/2 + 60))), vec2(90, 90), 0.0f);
    auto moduleOption3 = std::make_shared<bnuui::Box>(vec2(((WINDOW_WIDTH_PX/2) + 100), (WINDOW_HEIGHT_PX/2 + 60)), vec2(70, 70), 0.0f);
    moduleOption3->texture = getTextureFromModuleType(threeOptions[2]);
    moduleOption3->setOnClick([&, threeOptions](bnuui::Element& e) {
        handleOptionClick(threeOptions[2]);
        RenderSystem::isRenderingGacha = false;
        currentLevel.upgradesReceived++;
        currentLevel.gacha_called = false;
        clearGatchaUI(scene_ui);
    });
    moduleOption3->setOnHover([](bnuui::Element& e) { 
    });

    scene_ui.insert(gacha_box);
    scene_ui.insert(upgrade_title_box);
    scene_ui.insert(module1bg);
    scene_ui.insert(moduleOption1);
    scene_ui.insert(module2bg);
    scene_ui.insert(moduleOption2);
    scene_ui.insert(module3bg);
    scene_ui.insert(moduleOption3); 

}



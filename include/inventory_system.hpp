#pragma once

// A wrapper of bnuui::elements.
#include "bnuui/bnuui.hpp"
#include "tinyECS/components.hpp"
class InventorySystem {
private:
    MODULE_TYPES* selected_module;
    bnuui::SceneUI* scene_ui;
    int max_page = 0;
    int page = 0;

public:
    InventorySystem(bnuui::SceneUI& scene_ui, MODULE_TYPES& selected_module);
    void OpenInventory();
    void CloseInventory();

    void CreateModuleUI(MODULE_TYPES type, float offset);

    void NextPage();
    void PrevPage();
}; 

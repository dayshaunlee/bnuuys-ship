#include "inventory_system.hpp"
#include <memory>
#include <string>
#include "bnuui/buttons.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

InventorySystem::InventorySystem(bnuui::SceneUI& scene_ui, MODULE_TYPES& selected_module) {
    /*// Create the inventory bar.*/
    this->scene_ui = &scene_ui;
    this->selected_module = &selected_module;
}

void InventorySystem::CreateModuleUI(MODULE_TYPES type, float offset) {
    const float INIT_X = 340.0f;
    const float INIT_Y = 547.5f;

    auto module = std::make_shared<bnuui::Box>(vec2(INIT_X + offset, INIT_Y), vec2(40, 40), 0.0f);
    auto cursor = std::make_shared<bnuui::Cursor>(vec2(INIT_X + offset, INIT_Y), vec2(40, 40), 0.0f);
    auto counter = std::make_shared<bnuui::TextLabel>(vec2(INIT_X + 5.0f + offset, INIT_Y-10.0f), 1, "0");

    cursor->visible = false;
    module->children.push_back(cursor);
    module->children.push_back(counter);
    module->texture = getTextureFromModuleType(type);
    module->over_overlay = true;
    module->setOnClick([&, type](bnuui::Element& e) {
        if (*(this->selected_module) == type) {
            *(this->selected_module) = EMPTY;
        } else {
            *(this->selected_module) = type;
        }
    });
    module->setOnUpdate([&, type, cursor, counter](bnuui::Element& e, float dt) {
        Ship& ship = registry.ships.components[0];
        if (ship.available_modules[type] == 0) {
            e.color = vec3(0.5f, 0, 0);
        } else {
            e.color = vec3(1, 1, 1);
        }
        counter->setText(std::to_string(ship.available_modules[type]));

        if (*(this->selected_module) == type)
            cursor->visible = true;
        else
            cursor->visible = false;
    });

    scene_ui->InsertModuleInventory(module);
}

void InventorySystem::OpenInventory() {
    auto inventory_slots = std::make_shared<bnuui::LongBox>(vec2(420, 550), vec2(240, 72), 0.0f, true);
    scene_ui->InsertModuleInventory(inventory_slots);

    // Create all the UI components here.
    Ship& ship = registry.ships.components[0];
    std::unordered_map<MODULE_TYPES, uint> available_modules = ship.available_modules;

    const int MAX_COMPONENTS_IN_A_PAGE = 5;
    int components_in_page = 0;
    float offset = 0;
    max_page = 0;

    // TODO: Implement Paging capability.
    for (const auto& [type, count] : available_modules) {
        if (count == 0)
            continue;
        if (components_in_page > MAX_COMPONENTS_IN_A_PAGE) {
            // We have to create a new page.
            max_page++;
            components_in_page = 0;
            offset = 0;
        }
        CreateModuleUI(type, offset);
        offset += 40.0f;
    }
}

void InventorySystem::CloseInventory() {
    // Erase all the UI components.
    scene_ui->clearInventory();
}

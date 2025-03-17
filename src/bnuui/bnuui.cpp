#include "bnuui/bnuui.hpp"
#include <glm/ext/vector_float2.hpp>
#include <memory>

namespace bnuui {

bool SceneUI::insert(std::shared_ptr<Element> e) {
    ui_elems.push_back(e);
    return true;
}

bool SceneUI::remove(int idx) {
    if (idx < 0 || idx >= ui_elems.size()) return false;

    ui_elems.erase(ui_elems.begin() + idx);
    return true;
}

int SceneUI::size(){
    return ui_elems.size();
}

// This gets ALL ui_elems including the inventory_ui_elems.
std::vector<std::shared_ptr<Element>> SceneUI::getElems() {
    std::vector<std::shared_ptr<Element>> result = ui_elems;
    result.insert(result.end(), inventory_ui_elems.begin(), inventory_ui_elems.end());
    return result;
}

void SceneUI::update(float dt) {
    for (std::shared_ptr<Element> elem : ui_elems) {
        elem->doUpdate(dt);
    }
}

// API to be used by the inventory system.
bool SceneUI::InsertModuleInventory(std::shared_ptr<Element> elem) {
    inventory_ui_elems.push_back(elem);
    return true;
}

bool SceneUI::RemoveModuleInventory(int idx) {
    if (idx < 0 || idx >= inventory_ui_elems.size()) return false;

    inventory_ui_elems.erase(inventory_ui_elems.begin() + idx);
    return true;
}

std::vector<std::shared_ptr<Element>> SceneUI::getInventoryElems() {
    return inventory_ui_elems;
}

};  // namespace bnuui

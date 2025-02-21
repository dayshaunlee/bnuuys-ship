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

std::vector<std::shared_ptr<Element>> SceneUI::getElems() {
    return ui_elems;
}

void SceneUI::update(float dt) {
    for (std::shared_ptr<Element> elem : ui_elems) {
        elem->doUpdate(dt);
    }
}

};  // namespace bnuui

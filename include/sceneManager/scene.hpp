#pragma once

#include <string>
class Scene {
protected:
    std::string name;

public:
    virtual ~Scene() = default;
    virtual void Init() = 0;
    virtual void Exit() = 0;
    virtual void HandleInput(int key, int action, int mod) = 0;
    virtual void Update(float dt) = 0;

    const std::string& getName() {
        return name;
    }
};

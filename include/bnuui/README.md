# Bnuui Documentation
> I love this name 🐰.
> It's either this or one of bnu-ui, bnu/ui.
>
> -Brian.


# How the system works.

An `Element` forms the basis of the UI system.

```cpp
class Element {
public:
    // Appearance of the Element
    glm::vec2 position;
    glm::vec2 scale;
    float rotation;
    glm::vec2 offset;
    
    glm::vec3 color;

    TEXTURE_ASSET_ID texture;
    EFFECT_ASSET_ID effect;
    GEOMETRY_BUFFER_ID geometry;

    bool visible = true;
    
    // Parent/Child semantics for the Element
    std::vector<std::shared_ptr<Element>> children;
    std::weak_ptr<Element> parent;

    // Interactivity
    bool hovering = false;
    bool active = false;

    void setOnActive(std::function<void(Element&)> callback);
    void setOnClick(std::function<void(Element&)> callback);
    void setOnHover(std::function<void(Element&)> callback);
    void setOnUpdate(std::function<void(Element&, float dt)> callback);

    // Function to be used for system.
    virtual void doUpdate(float dt) = 0;
    
    bool isPointColliding(const vec2& point);
    void clickButton();

    virtual ~Element() = default;

protected:
    std::function<void(Element&)> onHover;
    std::function<void(Element&)> onActive;
    std::function<void(Element&, float dt)> onUpdate;
    std::function<void(Element&)> onClick;
};

```

All UI Elements that I have created all inherits this `Element` class. The main things to note is that the **UI Element is not an entity.** 

**Rendering of UI Elements.**

Another thing is I did reuse their rendering pipeline, that is the code for rendering a UI element is found on `render_system.cpp` , and follows the same structure as those found on a `RenderRequest` component.

**Features of UI Elements.**

All UI Element have **4** main ways to interact with. Those are, `OnActive`, `OnClick`, `OnHover`, and `OnUpdate`. Here’s what they do:

- `OnActive`: A function that you can set whenever the user is **HOLDING** that element.
- `OnClick`: A function that runs when the user **RELEASES** an element.
- `OnHover`: A function that runs when the user **HOVERS** over the element.
- `OnUpdate`: A function that runs every time in the game loop. (This helps in allowing us making our UI dynamic, for example if we want to change the picture of our UI from happy bunny to sad bunny when `hp < 0.5f`.)

# SceneUI

Eh, idk why I didn’t simply just store a vector of Elements in the scene itself. But I guess this does make the code in each Scene look nicer.

```cpp
// This is just a wrapper for a list of UI Elements.
// This is also the Public API that will be used in our scenes.
class SceneUI {
public:
    bool insert(std::shared_ptr<Element> elem);
    bool remove(int idx);
    std::vector<std::shared_ptr<Element>> getElems();

    void update(float dt);
private: 
    std::vector<std::shared_ptr<Element>> ui_elems;
};
```

**⚠️ POTENTIAL TODO:** 
Currently I deal with the UI interactivity in the Scene itself (inside the onMouseMove, etc functions). So I can probably write a function here to deal with those to make the Scene look cleaner as all of those are just boilerplate code anyways.

---

### Example Usage:

```cpp
// This is already declared in the State class definition.
bnuui::SceneUI scene_ui;

// Creating a Square Button.
auto button = std::make_shared<bnuui::SquareButton>(
    vec2(100,100),  // Position
    vec2(100,100),  // Scale
    0.0f);          // Rotation

// Set a lambda function for when the user clicks the button.
button->setOnClick([](bnuui::Element& e) {
    std::cout << "Clicking the button\n";
});

// Set a lambda function for what to do every second to the frame.
button->setOnUpdate([](bnuui::Element& e, float dt) {
    e.position.x++;
});

scene_ui.insert(button);
```

---

# Premade UI Elements.

Here are a list of UI elements that I made to make things easy to build stuff.

| **Class Name** | **Description** |
| --- | --- |
| SquareButton | A square button using the SproutsLand assets |
| PlayButton | A play button from the SproutsLand assets pack |
| Box | The same as SquareButton, but it doesn’t have any texture changes when hovering/clicking. |
| LongBox | A long box using the SproutsLand assets pack. |
| PlayerStatus | A cute bunny UI from the sprouts land. The face changes from happy to angry when it’s `curr_health` is at a specified percentage. |
| SimpleBox | A box using raw OpenGL geometry. |
| ProgressBar | A progress bar that indicates progress based on `max_val` and `curr_val`. Made using SimpleBoxes. |

**Minor rant for future self:**

> I’d argue that some of these are redundant and is just changing textures. And I’d agree with you that removing some of these and manually setting them might be better for “modularity”. 
>
> But I think that doing that somewhat makes things more messy and add a lot of boilerplate which I don’t want to deal with. So I think this is good enough for what I’m trying to accomplish.

---

### Todos:

- Create **text rendering**. (Will be shown in class)
- Create slider element.
- Create input fields. (Text Input, Combo box, etc.)
- Hot code reloading? (nah)
- Finish UI Editor Scene.
- Dialog box with animation.
- Inventory system for modules.

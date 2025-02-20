#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/tiny_ecs.hpp"

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem {
    /**
     * The following arrays store the assets the game will use. They are loaded
     * at initialization and are assumed to not be modified by the render loop.
     *
     * Whenever possible, add to these lists instead of creating dynamic state
     * it is easier to debug and faster to execute for the computer.
     */
    std::array<GLuint, texture_count> texture_gl_handles;
    std::array<ivec2, texture_count> texture_dimensions;

    // Make sure these paths remain in sync with the associated enumerators.
    // Associated id with .obj path
    const std::vector<std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths = {
        std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::CHICKEN, mesh_path("chicken.obj"))
        // specify meshes of other assets here
    };

    // Make sure these paths remain in sync with the associated enumerators (see
    // TEXTURE_ASSET_ID).
    const std::array<std::string, texture_count> texture_paths = {
        // Idle Up
        textures_path("bunny/idle_up0.png"),  // BUNNY_IDLE_UP0
        textures_path("bunny/idle_up1.png"),  // BUNNY_IDLE_UP1

        // Idle Right
        textures_path("bunny/idle_right0.png"),  // BUNNY_IDLE_RIGHT0
        textures_path("bunny/idle_right1.png"),  // BUNNY_IDLE_RIGHT1

        // Idle Down
        textures_path("bunny/idle_down0.png"),  // BUNNY_IDLE_DOWN0
        textures_path("bunny/idle_down1.png"),  // BUNNY_IDLE_DOWN1

        // Idle Left
        textures_path("bunny/idle_left0.png"),  // BUNNY_IDLE_LEFT0
        textures_path("bunny/idle_left1.png"),  // BUNNY_IDLE_LEFT1

        // Walk Up
        textures_path("bunny/walk_up0.png"),  // BUNNY_UP_WALK0
        textures_path("bunny/walk_up1.png"),  // BUNNY_UP_WALK1

        // Walk Right
        textures_path("bunny/walk_right0.png"),  // BUNNY_RIGHT_WALK0
        textures_path("bunny/walk_right1.png"),  // BUNNY_RIGHT_WALK1

        // Walk Down
        textures_path("bunny/walk_down0.png"),  // BUNNY_DOWN_WALK0
        textures_path("bunny/walk_down1.png"),  // BUNNY_DOWN_WALK1

        // Walk Left
        textures_path("bunny/walk_left0.png"),  // BUNNY_LEFT_WALK0
        textures_path("bunny/walk_left1.png"),  // BUNNY_LEFT_WALK1

        // water background
        textures_path("background/water_background.png"),

        // island background
        map_path("m1.png"),
    };

    std::array<GLuint, effect_count> effects;
    // Make sure these paths remain in sync with the associated enumerators.
    const std::array<std::string, effect_count> effect_paths = {shader_path("coloured"),
                                                                shader_path("egg"),
                                                                shader_path("chicken"),
                                                                shader_path("textured"),
                                                                shader_path("vignette")};

    std::array<GLuint, geometry_count> vertex_buffers;
    std::array<GLuint, geometry_count> index_buffers;
    std::array<Mesh, geometry_count> meshes;

   public:
    // Initialize the window
    bool init(GLFWwindow* window);

    template <class T>
    void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

    void initializeGlTextures();

    void initializeGlEffects();

    void initializeGlMeshes();

    Mesh& getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int) id]; };

    void initializeGlGeometryBuffers();

    // Initialize the screen texture used as intermediate render target
    // The draw loop first renders to this texture, then it is used for the
    // vignette shader
    bool initScreenTexture();

    // Destroy resources associated to one or all entities created by the system
    ~RenderSystem();

    // Draw all entities
    void draw();

    mat3 createProjectionMatrix();

    Entity get_screen_state_entity() { return screen_state_entity; }

   private:
    // Internal drawing functions for each entity type
    void drawGridLine(Entity entity, const mat3& projection);
    void drawTexturedMesh(Entity entity, const mat3& projection);
    void drawToScreen();

    // Window handle
    GLFWwindow* window;

    // Screen texture handles
    GLuint frame_buffer;
    GLuint off_screen_render_buffer_color;
    GLuint off_screen_render_buffer_depth;

    Entity screen_state_entity;
};

bool loadEffectFromFile(const std::string& vs_path, const std::string& fs_path, GLuint& out_program);

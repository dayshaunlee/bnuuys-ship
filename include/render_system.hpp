#pragma once
#include <array>
#include <utility>

#include "bnuui/bnuui.hpp"
#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/tiny_ecs.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_clip_space.hpp>


// fonts
#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>

// fonts
struct Character {
	unsigned int TextureID;  // ID handle of the glyph texture
	glm::ivec2   Size;       // Size of glyph
	glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
	unsigned int Advance;    // Offset to advance to next glyph
	char character;
};

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

	GLuint m_VAO;

	// fonts
	std::map<char, Character> m_ftCharacters;
	GLuint m_font_shaderProgram;
	GLuint m_font_VAO;
	GLuint m_font_VBO;

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
        textures_path("background/water_bg.png"),
        
        // island background
        map_path("m2_tutorial.png"),
        map_path("m3_level1.png"),
        map_path("m3_level2.png"),
        map_path("m3_level3.png"),
        map_path("m3_level4.png"),
        
      
        // Enemy (balloon/flyer)
        textures_path("enemies/balloon0.png"),
        textures_path("enemies/balloon1.png"),
        textures_path("enemies/balloon2.png"),

        // Enemy (chicken-boat)
        textures_path("enemies/chicken-boat0.png"),
        textures_path("enemies/chicken-boat1.png"),

        // Enemy (cow)
        textures_path("enemies/cow0.png"),
        textures_path("enemies/cow1.png"),
        textures_path("enemies/cow2.png"),

        // Tornado
        textures_path("effects/tornado0.png"),
        textures_path("effects/tornado1.png"),
        textures_path("effects/tornado2.png"),

        // Whirlpool
        textures_path("effects/whirlpool0.png"),
        textures_path("effects/whirlpool1.png"),
        textures_path("effects/whirlpool2.png"),
        
        // UI Assets.
        textures_path("ui/buttons/square3Normal.png"),
        textures_path("ui/buttons/square3Hover.png"),
        textures_path("ui/buttons/square3Clicked.png"),

        textures_path("ui/buttons/play_neutral.png"),
        textures_path("ui/buttons/play_clicked.png"),

        textures_path("ui/buttons/btn_neutral.png"),
        textures_path("ui/buttons/btn_clicked.png"),

        // Bunny UI Face Neutral
        textures_path("ui/faces/face_long_anim010.png"),
        textures_path("ui/faces/face_long_anim011.png"),
        textures_path("ui/faces/face_long_anim012.png"),
        textures_path("ui/faces/face_long_anim013.png"),
        textures_path("ui/faces/face_long_anim014.png"),
        textures_path("ui/faces/face_long_anim015.png"),
        textures_path("ui/faces/face_long_anim016.png"),
        textures_path("ui/faces/face_long_anim017.png"),
        textures_path("ui/faces/face_long_anim018.png"),

        // Bunny UI Angry face
        textures_path("ui/faces/face_angry01.png"),
        textures_path("ui/faces/face_angry02.png"),
        textures_path("ui/faces/face_angry03.png"),
        textures_path("ui/faces/face_angry04.png"),
        textures_path("ui/faces/face_angry05.png"),

        // Cursor UI
        textures_path("ui/cursor.png"),

        // Raft
        textures_path("raft.png"),

        // Simple Cannon
        textures_path("cannons/simple_cannon000.png"),
        textures_path("cannons/simple_cannon001.png"),
        textures_path("cannons/simple_cannon002.png"),
        textures_path("cannons/simple_cannon003.png"),
        textures_path("cannons/simple_cannon004.png"),
        textures_path("cannons/simple_cannon005.png"),

        // Projectiles
        textures_path("effects/bullet0.png"),
        textures_path("effects/bullet1.png"),
        textures_path("effects/bullet2.png"),

        // NPC bunny (jailed)
        textures_path("bunny/jailed-bunny0.png"),
        textures_path("bunny/jailed-bunny1.png"),

        // NPC bunny - idle up
        textures_path("bunny/npc_idle_down0.png"),
        textures_path("bunny/npc_idle_down1.png"),

        // Main Menu bg
        textures_path("background/main_menu_bg.png"),

        // Filled tile for enemy pathing debug
        textures_path("enemies/filledTile.png"),

        // Restart button
        textures_path("ui/buttons/restartbtn_neutral.png"),
        textures_path("ui/buttons/restartbtn_clicked.png"),

        // Game over background
        textures_path("background/game_over_bg.png"),
        
        // Tutorial Dialog.
        textures_path("ui/tutorial_ui/movementKeyTutorial.png"),
        textures_path("ui/tutorial_ui/spaceKeyTutorial.png"),
        textures_path("ui/tutorial_ui/mouseTutorial.png"),
        textures_path("ui/tutorial_ui/homeTutorial.png"),

        // Next level scene background
        textures_path("background/next_level_bg.png"),

        // Choose upgrade title on gacha pop-up
        textures_path("ui/upgrade_title.png"),

        // tutorial buttons
        textures_path("ui/buttons/tutorial_neutral.png"),
        textures_path("ui/buttons/tutorial_clicked.png"),

        textures_path("cannons/laser_weapon.png"),

        textures_path("cannons/laser_beam.png")
    };

    std::array<GLuint, effect_count> effects;
    // Make sure these paths remain in sync with the associated enumerators.
    const std::array<std::string, effect_count> effect_paths = {
        shader_path("coloured"), 
        shader_path("egg"), 
        shader_path("chicken"), 
        shader_path("textured"), 
        shader_path("vignette"),
        shader_path("font"),
    };

    std::array<GLuint, geometry_count> vertex_buffers;
    std::array<GLuint, geometry_count> index_buffers;
    std::array<Mesh, geometry_count> meshes;

   public:
    // Initialize the window
    bool init(GLFWwindow* window);

	bool fontInit(const std::string& font_filename, unsigned int font_default_size);

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

    // this is used to check that if redendering Gacha UI we don't render player on top
    static bool isRenderingGacha;

    // Destroy resources associated to one or all entities created by the system
    ~RenderSystem();

    // Draw all entities
    void draw();

    mat3 createProjectionMatrix();
    mat4 createUIMatrix();

    void renderText(std::string text, float x, float y, float scale, const glm::vec3& color, const glm::mat4& trans);

    Entity get_screen_state_entity() { return screen_state_entity; } 

   private:
    // Internal drawing functions for each entity type
    void drawGridLine(Entity entity, const mat3& projection);
    void drawTexturedMesh(Entity entity, const mat3& projection);
    void drawToScreen();

    // Drawing function for UI elements
    void drawUIElement(bnuui::Element& element, const mat3& projection);

    // Window handle
    GLFWwindow* window;

    // Screen texture handles
    GLuint frame_buffer;
    GLuint off_screen_render_buffer_color;
    GLuint off_screen_render_buffer_depth;

    Entity screen_state_entity;
};

bool loadEffectFromFile(const std::string& vs_path, const std::string& fs_path, GLuint& out_program);

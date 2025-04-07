// stdlib
#include <iostream>
#include <sstream>
#include <array>
#include <fstream>

// internal
#include "../ext/stb_image/stb_image.h"
#include "common.hpp"
#include "glcorearb.h"
#include "render_system.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

void APIENTRY openglDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                               const GLchar* message, const void* userParam) {
    std::cerr << "OpenGL Debug Message: " << message << std::endl;
}

bool RenderSystem::particleSystemInit() {
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f
    };

    glGenVertexArrays(1, &m_QuadVAO);
    glBindVertexArray(m_QuadVAO);

    GLuint quadVB, quadIB;
    glGenBuffers(1, &quadVB);
    glBindBuffer(GL_ARRAY_BUFFER, quadVB);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Set up vertex attribute for position (location = 1).
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // Create and upload index buffer.
    uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };
    glGenBuffers(1, &quadIB);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIB);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // ---- Set Up Instance Attributes ----

    // Generate instance transform VBO (for a 3x3 matrix per instance).
    glGenBuffers(1, &m_InstanceTransformVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_InstanceTransformVBO);
    // Allocate space for instance transforms (e.g., space for 1000 instances; adjust as needed).
    glBufferData(GL_ARRAY_BUFFER, 1000 * sizeof(mat3), nullptr, GL_DYNAMIC_DRAW);

    // Setup three vertex attributes for the transform matrix at locations 2, 3, and 4.
    // Each row of the 3x3 matrix is sent as a vec3.
    for (int i = 0; i < 3; i++) {
        glEnableVertexAttribArray(2 + i);
        glVertexAttribPointer(2 + i, 3, GL_FLOAT, GL_FALSE, sizeof(mat3), (void*)(i * sizeof(vec3)));
        // Tell OpenGL these attributes update once per instance.
        glVertexAttribDivisor(2 + i, 1);
    }

    // Generate instance color VBO.
    glGenBuffers(1, &m_InstanceColorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_InstanceColorVBO);
    // Allocate space for instance colors (vec4 per instance).
    glBufferData(GL_ARRAY_BUFFER, 1000 * sizeof(vec4), nullptr, GL_DYNAMIC_DRAW);

    // Setup the vertex attribute for color at location 5.
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)0);
    glVertexAttribDivisor(5, 1);

    // Unbind the VAO (and array buffer) to avoid accidental modifications.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ---- Shader Setup ----

    // Retrieve the shader program.
    const GLuint particle_effect_enum = (GLuint) EFFECT_ASSET_ID::PARTICLE;
    m_Particle_shaderProgram = (GLuint) effects[particle_effect_enum];

    // Get the uniform location for the view-projection matrix (remains a uniform).
    m_ParticleShaderViewProj = glGetUniformLocation(m_Particle_shaderProgram, "u_ViewProj");
    assert(m_ParticleShaderViewProj > -1);

    // Note: We no longer need uniforms for transform or color as they are now per-instance.
    return true;
}

// Render initialization
bool RenderSystem::init(GLFWwindow* window_arg) {
    this->window = window_arg;

    glfwMakeContextCurrent(window);
    // glfwSwapInterval(1);  // vsync

    // Load OpenGL function pointers
    const int is_fine = gl3w_init();
    assert(is_fine == 0);

    // Create a frame buffer
    frame_buffer = 0;
    glGenFramebuffers(1, &frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
    gl_has_errors();

    // For some high DPI displays (ex. Retina Display on Macbooks)
    // https://stackoverflow.com/questions/36672935/why-retina-screen-coordinate-value-is-twice-the-value-of-pixel-value
    int frame_buffer_width_px, frame_buffer_height_px;
    glfwGetFramebufferSize(
        window,
        &frame_buffer_width_px,
        &frame_buffer_height_px);  // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
    if (frame_buffer_width_px != WINDOW_WIDTH_PX) {
        printf(
            "WARNING: retina display! "
            "https://stackoverflow.com/questions/36672935/"
            "why-retina-screen-coordinate-value-is-twice-the-value-of-pixel-value\n");
        printf("glfwGetFramebufferSize = %d,%d\n", frame_buffer_width_px, frame_buffer_height_px);
        printf("requested window width,height = %d,%d\n", WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX);
    }

    // Hint: Ask your TA for how to setup pretty OpenGL error callbacks.
    // This can not be done in mac os, so do not enable
    // it unless you are on Linux or Windows. You will need to change the window creation
    // code to use OpenGL 4.3 (not suported on mac) and add additional .h and .cpp
    // glDebugMessageCallback((GLDEBUGPROC)errorCallback, nullptr);

    // We are not really using VAO's but without at least one bound we will crash in
    // some systems.
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);
    gl_has_errors();

    initScreenTexture();
    initializeGlTextures();
    initializeGlEffects();
    initializeGlGeometryBuffers();

    //// opengl debugging windows only
    //int flags;
    //glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    //if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
    //    glEnable(GL_DEBUG_OUTPUT);
    //    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    //    glDebugMessageCallback(openglDebugCallback, nullptr);
    //    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    //}

    particleSystemInit();
    return true;
}


bool RenderSystem::fontInit(const std::string& font_filename, unsigned int font_default_size) {
    // read in our shader files
    // enable blending or you will just get solid boxes instead of text
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // font buffer setup
    glGenVertexArrays(1, &m_font_VAO);
    glGenBuffers(1, &m_font_VBO);

    const GLuint font_effect_enum = (GLuint) EFFECT_ASSET_ID::FONT;
    m_font_shaderProgram = (GLuint) effects[font_effect_enum];

    // apply projection matrix for font
    glUseProgram(m_font_shaderProgram);
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(WINDOW_WIDTH_PX), 0.0f, static_cast<float>(WINDOW_HEIGHT_PX));
    GLint project_location = glGetUniformLocation(m_font_shaderProgram, "projection");
    assert(project_location > -1);
    std::cout << "project_location: " << project_location << std::endl;
    glUniformMatrix4fv(project_location, 1, GL_FALSE, glm::value_ptr(projection));

    // init FreeType fonts
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return false;
    }

    FT_Face face;
    if (FT_New_Face(ft, font_filename.c_str(), 0, &face))
    {
        std::cerr << "ERROR::FREETYPE: Failed to load font: " << font_filename << std::endl;
        return false;
    }

    // extract a default size
    FT_Set_Pixel_Sizes(face, 0, font_default_size);

    // disable byte-alignment restriction in OpenGL
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // load each of the chars - note only first 128 ASCII chars
    for (unsigned char c = (unsigned char)0; c < (unsigned char)128; c++)
    {
        // load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cerr << "ERROR::FREETYTPE: Failed to load Glyph " << c << std::endl;
            continue;
        }

        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        // std::cout << "texture: " << c << " = " << texture << std::endl;

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


        // now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x),
            (char)c
        };
        m_ftCharacters.insert(std::pair<char, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    // clean up
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // bind buffers
    glBindVertexArray(m_font_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_font_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    // release buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(m_VAO);

    return true;

}

void RenderSystem::initializeGlTextures() {
    glGenTextures((GLsizei) texture_gl_handles.size(), texture_gl_handles.data());

    for (uint i = 0; i < texture_paths.size(); i++) {
        const std::string& path = texture_paths[i];
        ivec2& dimensions = texture_dimensions[i];

        stbi_uc* data;
        data = stbi_load(path.c_str(), &dimensions.x, &dimensions.y, NULL, 4);

        if (data == NULL) {
            const std::string message = "Could not load the file " + path + ".";
            fprintf(stderr, "%s", message.c_str());
            assert(false);
        }
        glBindTexture(GL_TEXTURE_2D, texture_gl_handles[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dimensions.x, dimensions.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        gl_has_errors();
        stbi_image_free(data);
    }
    gl_has_errors();
}

void RenderSystem::initializeGlEffects() {
    for (uint i = 0; i < effect_paths.size(); i++) {
        const std::string vertex_shader_name = effect_paths[i] + ".vs.glsl";
        const std::string fragment_shader_name = effect_paths[i] + ".fs.glsl";

        bool is_valid = loadEffectFromFile(vertex_shader_name, fragment_shader_name, effects[i]);
        assert(is_valid && (GLuint) effects[i] != 0);
    }
}

// One could merge the following two functions as a template function...
template <class T>
void RenderSystem::bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices) {
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(uint) gid]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    gl_has_errors();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffers[(uint) gid]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), indices.data(), GL_STATIC_DRAW);
    gl_has_errors();
}

void RenderSystem::initializeGlMeshes() {
    for (uint i = 0; i < mesh_paths.size(); i++) {
        // Initialize meshes
        GEOMETRY_BUFFER_ID geom_index = mesh_paths[i].first;
        std::string name = mesh_paths[i].second;
        Mesh::loadFromOBJFile(name,
                              meshes[(int) geom_index].vertices,
                              meshes[(int) geom_index].vertex_indices,
                              meshes[(int) geom_index].original_size);

        bindVBOandIBO(geom_index, meshes[(int) geom_index].vertices, meshes[(int) geom_index].vertex_indices);
    }
}

void RenderSystem::initializeGlGeometryBuffers() {
    // Vertex Buffer creation.
    glGenBuffers((GLsizei) vertex_buffers.size(), vertex_buffers.data());
    // Index Buffer creation.
    glGenBuffers((GLsizei) index_buffers.size(), index_buffers.data());

    // Index and Vertex buffer data initialization.
    initializeGlMeshes();

    //////////////////////////
    // Initialize sprite
    // The position corresponds to the center of the texture.
    std::vector<TexturedVertex> textured_vertices(4);
    textured_vertices[0].position = {-1.f / 2, +1.f / 2, 0.f};
    textured_vertices[1].position = {+1.f / 2, +1.f / 2, 0.f};
    textured_vertices[2].position = {+1.f / 2, -1.f / 2, 0.f};
    textured_vertices[3].position = {-1.f / 2, -1.f / 2, 0.f};
    textured_vertices[0].texcoord = {0.f, 1.f};
    textured_vertices[1].texcoord = {1.f, 1.f};
    textured_vertices[2].texcoord = {1.f, 0.f};
    textured_vertices[3].texcoord = {0.f, 0.f};

    // Counterclockwise as it's the default OpenGL front winding direction.
    const std::vector<uint16_t> textured_indices = {0, 3, 1, 1, 3, 2};
    bindVBOandIBO(GEOMETRY_BUFFER_ID::SPRITE, textured_vertices, textured_indices);

    /* LEGACY - not used, but code below still relies on it...*/
    ////////////////////////
    // Initialize egg
    std::vector<ColoredVertex> egg_vertices;
    std::vector<uint16_t> egg_indices;
    constexpr float z = -0.1f;
    constexpr int NUM_TRIANGLES = 62;

    for (int i = 0; i < NUM_TRIANGLES; i++) {
        const float t = float(i) * M_PI * 2.f / float(NUM_TRIANGLES - 1);
        egg_vertices.push_back({});
        egg_vertices.back().position = {0.5 * cos(t), 0.5 * sin(t), z};
        egg_vertices.back().color = {0.8, 0.8, 0.8};
    }
    egg_vertices.push_back({});
    egg_vertices.back().position = {0, 0, 0};
    egg_vertices.back().color = {1, 1, 1};
    for (int i = 0; i < NUM_TRIANGLES; i++) {
        egg_indices.push_back((uint16_t) i);
        egg_indices.push_back((uint16_t) ((i + 1) % NUM_TRIANGLES));
        egg_indices.push_back((uint16_t) NUM_TRIANGLES);
    }
    int geom_index = (int) GEOMETRY_BUFFER_ID::EGG;
    meshes[geom_index].vertices = egg_vertices;
    meshes[geom_index].vertex_indices = egg_indices;
    bindVBOandIBO(GEOMETRY_BUFFER_ID::EGG, meshes[geom_index].vertices, meshes[geom_index].vertex_indices);

    //////////////////////////////////
    // Initialize debug line
    std::vector<ColoredVertex> line_vertices;
    std::vector<uint16_t> line_indices;

    constexpr float depth = 0.5f;
    // constexpr vec3 red = { 0.8, 0.1, 0.1 };
    constexpr vec3 red = {1.0, 1.0, 1.0};

    // Corner points
    line_vertices = {
        {{-0.5, -0.5, depth}, red},
        {{-0.5, 0.5, depth}, red},
        {{0.5, 0.5, depth}, red},
        {{0.5, -0.5, depth}, red},
    };

    // Two triangles
    line_indices = {0, 1, 3, 1, 2, 3};

    geom_index = (int) GEOMETRY_BUFFER_ID::DEBUG_LINE;
    meshes[geom_index].vertices = line_vertices;
    meshes[geom_index].vertex_indices = line_indices;
    bindVBOandIBO(GEOMETRY_BUFFER_ID::DEBUG_LINE, line_vertices, line_indices);

    ///////////////////////////////////////////////////////
    // Initialize screen triangle (yes, triangle, not quad; its more efficient).
    std::vector<vec3> screen_vertices(3);
    screen_vertices[0] = {-1, -6, 0.f};
    screen_vertices[1] = {6, -1, 0.f};
    screen_vertices[2] = {-1, 6, 0.f};

    // Counterclockwise as it's the default opengl front winding direction.
    const std::vector<uint16_t> screen_indices = {0, 1, 2};
    bindVBOandIBO(GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE, screen_vertices, screen_indices);

    ///////////////////////////////////////////////////////
    // Initialize square geometry for the laser
    std::vector<ColoredVertex> square_vertices;
    std::vector<uint16_t> square_indices;

    constexpr float square_depth = 0.0f;  // Adjust if needed

    // Define the four corners of the square (centered at origin)
    square_vertices = {
        {{-0.5f, -0.5f, square_depth}, {1.0f, 0.f, 0.f}},  // Bottom-left (brown color)
        {{0.5f, -0.5f, square_depth}, {1.0f, 0.f, 0.f}},   // Bottom-right
        {{0.5f, 0.5f, square_depth}, {1.0f, 0.f, 0.f}},    // Top-right
        {{-0.5f, 0.5f, square_depth}, {1.0f, 0.f, 0.f}},   // Top-left
    };

    // Define two triangles forming the square
    square_indices = {0, 1, 3, 1, 2, 3};

    // Store in the mesh system
    int square_geom_index = (int) GEOMETRY_BUFFER_ID::LASER_SQUARE;
    meshes[square_geom_index].vertices = square_vertices;
    meshes[square_geom_index].vertex_indices = square_indices;
    bindVBOandIBO(GEOMETRY_BUFFER_ID::LASER_SQUARE, square_vertices, square_indices);

    ///////////////////////////////////////////////////////
    // Square Geom for Progressbar.
    std::vector<ColoredVertex> UISquare_vertices;
    std::vector<uint16_t> UISquare_indices;

    constexpr float UISquare_depth = 0.0f;

    // Define the four corners of the square (centered at origin)
    UISquare_vertices = {
        {{-0.5f, -0.5f, UISquare_depth}, {1.0f, 1.0f, 1.0f}},
        {{0.5f, -0.5f, UISquare_depth}, {1.0f, 1.0f, 1.0f}},
        {{0.5f, 0.5f, UISquare_depth}, {1.0f, 1.0f, 1.0f}},
        {{-0.5f, 0.5f, UISquare_depth}, {1.0f, 1.0f, 1.0f}},
    };

    // Define two triangles forming the square
    UISquare_indices = {0, 1, 3, 1, 2, 3};

    int UISquare_geom_index = (int) GEOMETRY_BUFFER_ID::UI_SQUARE;
    meshes[UISquare_geom_index].vertices = UISquare_vertices;
    meshes[UISquare_geom_index].vertex_indices = UISquare_indices;
    bindVBOandIBO(GEOMETRY_BUFFER_ID::UI_SQUARE, UISquare_vertices, UISquare_indices);

    
    ///////////////////////////////////////////////////////
    // Square Geom for Overlay.
    std::vector<ColoredVertex> ASquare_vertices;
    std::vector<uint16_t> ASquare_indices;

    constexpr float ASquare_depth = 1.0f;  // always on top

    // Define the four corners of the square (centered at origin)
    ASquare_vertices = {
        {{-0.5f, -0.5f, ASquare_depth}, {1.0f, 1.0f, 1.0f}},
        {{0.5f, -0.5f, ASquare_depth}, {1.0f, 1.0f, 1.0f}},
        {{0.5f, 0.5f, ASquare_depth}, {1.0f, 1.0f, 1.0f}},
        {{-0.5f, 0.5f, ASquare_depth}, {1.0f, 1.0f, 1.0f}},
    };

    // Define two triangles forming the square
    ASquare_indices = {0, 1, 3, 1, 2, 3};

    int ASquare_geom_index = (int) GEOMETRY_BUFFER_ID::OVERLAY_SQUARE;
    meshes[ASquare_geom_index].vertices = ASquare_vertices;
    meshes[ASquare_geom_index].vertex_indices = ASquare_indices;
    bindVBOandIBO(GEOMETRY_BUFFER_ID::OVERLAY_SQUARE, ASquare_vertices, ASquare_indices);


    ///////////////////////////////////////////////////////
    // Square Geom for highlighting.
    std::vector<ColoredVertex> highlightSquare_vertices;
    std::vector<uint16_t> highlightSquare_indices;

    constexpr float highlightSquare_depth = 0.0f;
    constexpr float squareSize = 0.5f;
    constexpr float thickness = 0.05f;


    // Define the four corners of the square (centered at origin)
    // highlightSquare_vertices = {
    //     {{-0.5f, -0.5f, UISquare_depth}, {1.f, 0.f, 0.f}},
    //     {{ 0.5f, -0.5f, UISquare_depth}, {0.f, 1.f, 0.f}},
    //     {{ 0.5f,  0.5f, UISquare_depth}, {0.f, 0.f, 1.f}},
    //     {{-0.5f,  0.5f, UISquare_depth}, {1.f, 1.f, 0.f}},
    // };

    // Define the vertices for Square 1 top
    highlightSquare_vertices.push_back({{-squareSize, -squareSize, highlightSquare_depth}, {1.0f, 1.0f, 1.0f}});
    highlightSquare_vertices.push_back({{ squareSize, -squareSize, highlightSquare_depth}, {1.0f, 1.0f, 1.0f}});
    highlightSquare_vertices.push_back({{ squareSize, -squareSize + thickness, highlightSquare_depth}, {1.0f, 1.0f, 1.0f}});
    highlightSquare_vertices.push_back({{-squareSize, -squareSize + thickness, highlightSquare_depth}, {1.0f, 1.0f, 1.0f}});

    // // Define the vertices for Square 2 bottom
    highlightSquare_vertices.push_back({{-squareSize, squareSize, highlightSquare_depth}, {1.0f, 1.0f, 1.0f}});
    highlightSquare_vertices.push_back({{squareSize, squareSize, highlightSquare_depth}, {1.0f, 1.0f, 1.0f}});
    highlightSquare_vertices.push_back({{squareSize, squareSize - thickness, highlightSquare_depth}, {1.0f, 1.0f, 1.0f}});
    highlightSquare_vertices.push_back({{-squareSize, squareSize - thickness, highlightSquare_depth}, {1.0f, 1.0f, 1.0f}});

    // // Define the vertices for Square 3 left
    highlightSquare_vertices.push_back({{-squareSize, -squareSize, highlightSquare_depth}, {1.0f, 1.0f, 1.0f}});
    highlightSquare_vertices.push_back({{-squareSize + thickness, -squareSize, highlightSquare_depth}, {1.0f, 1.0f, 1.0f}});
    highlightSquare_vertices.push_back({{-squareSize + thickness, squareSize, highlightSquare_depth}, {1.0f, 1.0f, 1.0f}});
    highlightSquare_vertices.push_back({{-squareSize, squareSize, highlightSquare_depth}, {1.0f, 1.0f, 1.0f}});

    // // Define the vertices for Square 4 right
    highlightSquare_vertices.push_back({{squareSize, -squareSize, highlightSquare_depth}, {1.0f, 1.0f, 1.0f}});
    highlightSquare_vertices.push_back({{squareSize - thickness, -squareSize, highlightSquare_depth}, {1.0f, 1.0f, 1.0f}});
    highlightSquare_vertices.push_back({{squareSize - thickness, squareSize, highlightSquare_depth}, {1.0f, 1.0f, 1.0f}});
    highlightSquare_vertices.push_back({{squareSize, squareSize, highlightSquare_depth}, {1.0f, 1.0f, 1.0f}});

    // Define two triangles forming the square
    // highlightSquare_indices = {0, 1, 2, 2, 3, 0};

    // Define indices for Square 1
    highlightSquare_indices.push_back(0); highlightSquare_indices.push_back(1); highlightSquare_indices.push_back(2);
    highlightSquare_indices.push_back(2); highlightSquare_indices.push_back(3); highlightSquare_indices.push_back(0);

    // // Define indices for Square 2
    highlightSquare_indices.push_back(4); highlightSquare_indices.push_back(5); highlightSquare_indices.push_back(6);
    highlightSquare_indices.push_back(6); highlightSquare_indices.push_back(7); highlightSquare_indices.push_back(4);

    // // Define indices for Square 3
    highlightSquare_indices.push_back(8); highlightSquare_indices.push_back(9); highlightSquare_indices.push_back(10);
    highlightSquare_indices.push_back(10); highlightSquare_indices.push_back(11); highlightSquare_indices.push_back(8);

    // // Define indices for Square 4
    highlightSquare_indices.push_back(12); highlightSquare_indices.push_back(13); highlightSquare_indices.push_back(14);
    highlightSquare_indices.push_back(14); highlightSquare_indices.push_back(15); highlightSquare_indices.push_back(12);

    int highlightSquare_geom_index = (int) GEOMETRY_BUFFER_ID::HIGHLIGHT_SQUARE;
    meshes[highlightSquare_geom_index].vertices = highlightSquare_vertices;
    meshes[highlightSquare_geom_index].vertex_indices = highlightSquare_indices;
    bindVBOandIBO(GEOMETRY_BUFFER_ID::HIGHLIGHT_SQUARE, highlightSquare_vertices, highlightSquare_indices);
}

RenderSystem::~RenderSystem() {
    // Don't need to free gl resources since they last for as long as the program,
    // but it's polite to clean after yourself.
    glDeleteBuffers((GLsizei) vertex_buffers.size(), vertex_buffers.data());
    glDeleteBuffers((GLsizei) index_buffers.size(), index_buffers.data());
    glDeleteTextures((GLsizei) texture_gl_handles.size(), texture_gl_handles.data());
    glDeleteTextures(1, &off_screen_render_buffer_color);
    glDeleteRenderbuffers(1, &off_screen_render_buffer_depth);
    gl_has_errors();

    for (uint i = 0; i < effect_count; i++) {
        glDeleteProgram(effects[i]);
    }
    // delete allocated resources
    glDeleteFramebuffers(1, &frame_buffer);
    gl_has_errors();

    // remove all entities created by the render system
    while (registry.renderRequests.entities.size() > 0)
        registry.remove_all_components_of(registry.renderRequests.entities.back());
}

// Initialize the screen texture from a standard sprite
bool RenderSystem::initScreenTexture() {
    // create a single entry
    registry.screenStates.emplace(screen_state_entity);

    int framebuffer_width, framebuffer_height;
    glfwGetFramebufferSize(
        const_cast<GLFWwindow*>(window),
        &framebuffer_width,
        &framebuffer_height);  // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

    glGenTextures(1, &off_screen_render_buffer_color);
    glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, framebuffer_width, framebuffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl_has_errors();

    glGenRenderbuffers(1, &off_screen_render_buffer_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, off_screen_render_buffer_depth);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, off_screen_render_buffer_color, 0);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, framebuffer_width, framebuffer_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, off_screen_render_buffer_depth);
    gl_has_errors();

    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    return true;
}

bool gl_compile_shader(GLuint shader) {
    glCompileShader(shader);
    gl_has_errors();
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        GLint log_len;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
        std::vector<char> log(log_len);
        glGetShaderInfoLog(shader, log_len, &log_len, log.data());
        glDeleteShader(shader);

        gl_has_errors();

        fprintf(stderr, "GLSL: %s", log.data());
        return false;
    }

    return true;
}

bool loadEffectFromFile(const std::string& vs_path, const std::string& fs_path, GLuint& out_program) {
    // Opening files
    std::ifstream vs_is(vs_path);
    std::ifstream fs_is(fs_path);
    if (!vs_is.good() || !fs_is.good()) {
        fprintf(stderr, "Failed to load shader files %s, %s", vs_path.c_str(), fs_path.c_str());
        assert(false);
        return false;
    }

    // Reading sources
    std::stringstream vs_ss, fs_ss;
    vs_ss << vs_is.rdbuf();
    fs_ss << fs_is.rdbuf();
    std::string vs_str = vs_ss.str();
    std::string fs_str = fs_ss.str();
    const char* vs_src = vs_str.c_str();
    const char* fs_src = fs_str.c_str();
    GLsizei vs_len = (GLsizei) vs_str.size();
    GLsizei fs_len = (GLsizei) fs_str.size();

    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vs_src, &vs_len);
    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fs_src, &fs_len);
    gl_has_errors();

    // Compiling
    if (!gl_compile_shader(vertex)) {
        fprintf(stderr, "Vertex compilation failed");
        assert(false);
        return false;
    }
    if (!gl_compile_shader(fragment)) {
        fprintf(stderr, "Fragment compilation failed");
        assert(false);
        return false;
    }

    // Linking
    out_program = glCreateProgram();
    glAttachShader(out_program, vertex);
    glAttachShader(out_program, fragment);
    glLinkProgram(out_program);
    gl_has_errors();

    {
        GLint is_linked = GL_FALSE;
        glGetProgramiv(out_program, GL_LINK_STATUS, &is_linked);
        if (is_linked == GL_FALSE) {
            GLint log_len;
            glGetProgramiv(out_program, GL_INFO_LOG_LENGTH, &log_len);
            std::vector<char> log(log_len);
            glGetProgramInfoLog(out_program, log_len, &log_len, log.data());
            gl_has_errors();

            fprintf(stderr, "Link error: %s", log.data());
            assert(false);
            return false;
        }
    }

    // No need to carry this around. Keeping these objects is only useful if we recycle
    // the same shaders over and over, which we don't, so no need and this is simpler.
    glDetachShader(out_program, vertex);
    glDetachShader(out_program, fragment);
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    gl_has_errors();

    return true;
}

/*float GetTextWidth(const std::string& text, float font_scale) {*/
/*    float width = 0;*/
/*    for (char c : text) {*/
/*        if (CHARACTERS.find(c) != CHARACTERS.end()) {*/
/*            width += (CHARACTERS[c].Advance >> 6) * font_scale;*/
/*        }*/
/*    }*/
/**/
/*    return width;*/
/*}*/
/**/
/*float GetTextHeight(const std::string& text, float font_scale) {*/
/*    float height = 0;*/
/*    for (char c : text) {*/
/*        if (CHARACTERS.find(c) != CHARACTERS.end()) {*/
/*            height += max(height, (CHARACTERS[c].Size.y) * font_scale);*/
/*        }*/
/*    }*/
/**/
/*    return height;*/
/*}*/

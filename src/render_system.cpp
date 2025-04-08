#include <SDL.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>
#include <memory>

// internal
#include "render_system.hpp"
#include "bnuui/bnuui.hpp"
#include "camera_system.hpp"
#include "common.hpp"
#include "sceneManager/scene_manager.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "gacha_system.hpp"

bool RenderSystem::isRenderingGacha = false;
bool RenderSystem::isRenderingBook = false;
bool RenderSystem::isPaused = false;
bool RenderSystem::isInGame = false;

void RenderSystem::drawGridLine(Entity entity, const mat3& projection) {
    GridLine& gridLine = registry.gridLines.get(entity);

    // Transformation code, see Rendering and Transformation in the template
    // specification for more info Incrementally updates transformation matrix,
    // thus ORDER IS IMPORTANT
    Transform transform;
    if (registry.backgroundObjects.has(entity)) {
        transform.translate(gridLine.start_pos + CameraSystem::GetInstance()->position);
    } else {
        transform.translate(gridLine.start_pos);
    }
    transform.scale(gridLine.end_pos);

    assert(registry.renderRequests.has(entity));
    const RenderRequest& render_request = registry.renderRequests.get(entity);

    const GLuint used_effect_enum = (GLuint) render_request.used_effect;
    assert(used_effect_enum != (GLuint) EFFECT_ASSET_ID::EFFECT_COUNT);
    const GLuint program = (GLuint) effects[used_effect_enum];

    // setting shaders
    glUseProgram(program);
    gl_has_errors();

    assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
    const GLuint vbo = vertex_buffers[(GLuint) render_request.used_geometry];
    const GLuint ibo = index_buffers[(GLuint) render_request.used_geometry];

    // Setting vertex and index buffers
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    gl_has_errors();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    gl_has_errors();

    if (render_request.used_effect == EFFECT_ASSET_ID::EGG) {
        GLint in_position_loc = glGetAttribLocation(program, "in_position");
        gl_has_errors();

        GLint in_color_loc = glGetAttribLocation(program, "in_color");
        gl_has_errors();

        glEnableVertexAttribArray(in_position_loc);
        glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (void*) 0);
        gl_has_errors();

        glEnableVertexAttribArray(in_color_loc);
        glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (void*) sizeof(vec3));
        gl_has_errors();
    } else {
        assert(false && "Type of render request not supported");
    }

    // Getting uniform locations for glUniform* calls
    GLint color_uloc = glGetUniformLocation(program, "fcolor");
    const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
    // CK: std::cout << "line color: " << color.r << ", " << color.g << ", " << color.b << std::endl;
    glUniform3fv(color_uloc, 1, (float*) &color);
    gl_has_errors();

    // Get number of indices from index buffer, which has elements uint16_t
    GLint size = 0;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    gl_has_errors();

    GLsizei num_indices = size / sizeof(uint16_t);

    GLint currProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
    // Setting uniform values to the currently bound program
    GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
    glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float*) &transform.mat);
    gl_has_errors();

    GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
    glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*) &projection);
    gl_has_errors();

    // Drawing of num_indices/3 triangles specified in the index buffer
    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
    gl_has_errors();
}

void RenderSystem::drawOverlay(Entity entity, const mat3& projection) {
    Overlay& overlay = registry.overlays.get(entity);

    // Transformation code, see Rendering and Transformation in the template
    // specification for more info Incrementally updates transformation matrix,
    // thus ORDER IS IMPORTANT
    Transform transform;
    transform.translate(overlay.pos);
    transform.scale(overlay.size);

    assert(registry.renderRequests.has(entity));
    const RenderRequest& render_request = registry.renderRequests.get(entity);

    const GLuint used_effect_enum = (GLuint) render_request.used_effect;
    assert(used_effect_enum != (GLuint) EFFECT_ASSET_ID::EFFECT_COUNT);
    const GLuint program = (GLuint) effects[used_effect_enum];

    // setting shaders
    glUseProgram(program);
    gl_has_errors();

    assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
    const GLuint vbo = vertex_buffers[(GLuint) render_request.used_geometry];
    const GLuint ibo = index_buffers[(GLuint) render_request.used_geometry];

    // Setting vertex and index buffers
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    gl_has_errors();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    gl_has_errors();

    if (render_request.used_effect == EFFECT_ASSET_ID::ALPHA) {
        GLint in_position_loc = glGetAttribLocation(program, "in_position");
        gl_has_errors();

        GLint in_color_loc = glGetAttribLocation(program, "in_color");
        gl_has_errors();

        glEnableVertexAttribArray(in_position_loc);
        glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (void*) 0);
        gl_has_errors();

        glEnableVertexAttribArray(in_color_loc);
        glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (void*) sizeof(vec3));
        gl_has_errors();
    } else {
        assert(false && "Type of render request not supported");
    }

    // Getting uniform locations for glUniform* calls
    GLint color_uloc = glGetUniformLocation(program, "fcolor");
    const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
    // CK: std::cout << "line color: " << color.r << ", " << color.g << ", " << color.b << std::endl;
    glUniform3fv(color_uloc, 1, (float*) &color);
    gl_has_errors();

    GLint alpha_uloc = glGetUniformLocation(program, "alpha");
    glUniform1f(alpha_uloc, overlay.alpha);
    gl_has_errors();

    GLint visible_uloc = glGetUniformLocation(program, "visible");
    glUniform1i(visible_uloc, overlay.visible);
    gl_has_errors();

    GLint hl_count_uloc = glGetUniformLocation(program, "num_centers");
    glUniform1i(hl_count_uloc, highlight_count);
    gl_has_errors();

    GLint centers_uloc = glGetUniformLocation(program, "centers");
    glUniform3fv(centers_uloc, 5, value_ptr(highlight_centers[0]));
    gl_has_errors();



    // Get number of indices from index buffer, which has elements uint16_t
    GLint size = 0;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    gl_has_errors();

    GLsizei num_indices = size / sizeof(uint16_t);

    GLint currProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
    // Setting uniform values to the currently bound program
    GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
    glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float*) &transform.mat);
    gl_has_errors();

    GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
    glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*) &projection);
    gl_has_errors();

    // Drawing of num_indices/3 triangles specified in the index buffer
    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
    gl_has_errors();
}

void RenderSystem::drawTexturedMesh(Entity entity, const mat3& projection) {
    Motion& motion = registry.motions.get(entity);
    // Transformation code, see Rendering and Transformation in the template
    // specification for more info Incrementally updates transformation matrix,
    // thus ORDER IS IMPORTANT

    // BRIAN TODO:
    Transform transform;
    if (registry.backgroundObjects.has(entity)) {
        transform.translate(motion.position + CameraSystem::GetInstance()->position);
    } else {
        transform.translate(motion.position);
    }
    transform.scale(motion.scale);
    transform.rotate(radians(motion.angle));

    assert(registry.renderRequests.has(entity));
    const RenderRequest& render_request = registry.renderRequests.get(entity);

    const GLuint used_effect_enum = (GLuint) render_request.used_effect;
    assert(used_effect_enum != (GLuint) EFFECT_ASSET_ID::EFFECT_COUNT);
    const GLuint program = (GLuint) effects[used_effect_enum];

    // Setting shaders
    glUseProgram(program);
    gl_has_errors();

    assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
    const GLuint vbo = vertex_buffers[(GLuint) render_request.used_geometry];
    const GLuint ibo = index_buffers[(GLuint) render_request.used_geometry];

    // Setting vertex and index buffers
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    gl_has_errors();

    // texture-mapped entities - use data location as in the vertex buffer
    if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED) {
        GLint in_position_loc = glGetAttribLocation(program, "in_position");
        GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
        gl_has_errors();
        assert(in_texcoord_loc >= 0);

        glEnableVertexAttribArray(in_position_loc);
        glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*) 0);
        gl_has_errors();

        glEnableVertexAttribArray(in_texcoord_loc);
        glVertexAttribPointer(in_texcoord_loc,
                              2,
                              GL_FLOAT,
                              GL_FALSE,
                              sizeof(TexturedVertex),
                              (void*) sizeof(vec3));  // note the stride to skip the preceeding vertex position

        // Enabling and binding texture to slot 0
        glActiveTexture(GL_TEXTURE0);
        gl_has_errors();

        assert(registry.renderRequests.has(entity));
        GLuint texture_id = texture_gl_handles[(GLuint) registry.renderRequests.get(entity).used_texture];

        glBindTexture(GL_TEXTURE_2D, texture_id);
        // Brian: Disable smoothing
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        gl_has_errors();
    }
    // .obj entities
    else if (render_request.used_effect == EFFECT_ASSET_ID::CHICKEN ||
             render_request.used_effect == EFFECT_ASSET_ID::EGG) {
        GLint in_position_loc = glGetAttribLocation(program, "in_position");
        GLint in_color_loc = glGetAttribLocation(program, "in_color");
        gl_has_errors();

        glEnableVertexAttribArray(in_position_loc);
        glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (void*) 0);
        gl_has_errors();

        glEnableVertexAttribArray(in_color_loc);
        glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (void*) sizeof(vec3));
        gl_has_errors();
    } else {
        assert(false && "Type of render request not supported");
    }

    // Getting uniform locations for glUniform* calls
    GLint color_uloc = glGetUniformLocation(program, "fcolor");
    const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
    glUniform3fv(color_uloc, 1, (float*) &color);
    gl_has_errors();

    // Get number of indices from index buffer, which has elements uint16_t
    GLint size = 0;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    gl_has_errors();

    GLsizei num_indices = size / sizeof(uint16_t);
    // GLsizei num_triangles = num_indices / 3;

    GLint currProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
    // Setting uniform values to the currently bound program
    GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
    glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float*) &transform.mat);
    gl_has_errors();

    GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
    glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*) &projection);
    gl_has_errors();

    // Drawing of num_indices/3 triangles specified in the index buffer
    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
    gl_has_errors();
}

void RenderSystem::drawUIElement(bnuui::Element& element, const mat3& projection) {
    if (!element.visible) return;

    glm::mat4 UI_Matrix = mat4(1.0f);

    Transform transform;
    transform.translate(element.position);
    transform.scale(element.scale);
    transform.rotate(radians(element.rotation));

    const GLuint program = (GLuint) effects[(GLuint) element.effect];
    glUseProgram(program);
    gl_has_errors();

    const GLuint vbo = vertex_buffers[(GLuint) element.geometry];
    const GLuint ibo = index_buffers[(GLuint) element.geometry];
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    gl_has_errors();

    // Texture handling
    if (element.effect == EFFECT_ASSET_ID::TEXTURED) {
        GLint in_position_loc = glGetAttribLocation(program, "in_position");
        GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
        
        // Check if attribute locations are valid
        if (in_position_loc < 0 || in_texcoord_loc < 0) {
            std::cerr << "Error: Failed to get attribute locations in UI element shader" << std::endl;
            return;
        }
        
        glEnableVertexAttribArray(in_position_loc);
        glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*) 0);
        gl_has_errors();

        glEnableVertexAttribArray(in_texcoord_loc);
        glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*) sizeof(vec3));
        gl_has_errors();

        glActiveTexture(GL_TEXTURE0);
        gl_has_errors();
        
        // Validate texture index
        if ((GLuint)element.texture >= texture_gl_handles.size()) {
            std::cerr << "Error: Invalid texture index: " << (GLuint)element.texture 
                      << ", max is " << texture_gl_handles.size() - 1 << std::endl;
            return;
        }
        
        GLuint texture_id = texture_gl_handles[(GLuint) element.texture];
        
        // Validate texture ID
        if (texture_id == 0) {
            std::cerr << "Error: Texture ID is 0 for texture index: " << (GLuint)element.texture << std::endl;
            return;
        }
        
        glBindTexture(GL_TEXTURE_2D, texture_id);
        // Brian: Disable smoothing
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    } else {
        GLint in_position_loc = glGetAttribLocation(program, "in_position");
        GLint in_color_loc = glGetAttribLocation(program, "in_color");
        gl_has_errors();

        glEnableVertexAttribArray(in_position_loc);
        glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (void*) 0);
        gl_has_errors();

        glEnableVertexAttribArray(in_color_loc);
        glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (void*) sizeof(vec3));
        gl_has_errors();
    }

    // Set color uniform
    GLint color_uloc = glGetUniformLocation(program, "fcolor");
    glUniform3fv(color_uloc, 1, (float*) &element.color);
    gl_has_errors();

    // Set transformation uniforms
    GLuint transform_loc = glGetUniformLocation(program, "transform");
    glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float*) &transform.mat);
    gl_has_errors();

    GLuint projection_loc = glGetUniformLocation(program, "projection");
    glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*) &projection);
    gl_has_errors();

    // Draw the UI element
    GLint size = 0;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    GLsizei num_indices = size / sizeof(uint16_t);
    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
    gl_has_errors();

    // Recursively draw children
    for (auto& child : element.children) {
        if (child->getText() == "")
            drawUIElement(*child, projection);
        else 
            renderText(child->getText(), child->position.x, WINDOW_HEIGHT_PX - child->position.y, child->getFontSize(), child->color, UI_Matrix);
    }
}


void RenderSystem::drawParticles(Entity entity, const mat3& projection) {
    ParticleEmitter& emitter = registry.particleEmitters.get(entity); 
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(m_Particle_shaderProgram);
    glUniformMatrix3fv(m_ParticleShaderViewProj, 1, GL_FALSE, (float*)&projection);

    std::vector<mat3> transforms;
    std::vector<vec4> colors;

    for (auto& particle : emitter.particles) {
        if (!particle.Active)
            continue;

        float life = particle.LifeRemaining / particle.LifeTime;
        vec4 color = glm::mix(particle.ColorEnd, particle.ColorBegin, life);
        float size = glm::mix(particle.SizeEnd, particle.SizeBegin, life);

        Transform transform;
        if (registry.backgroundObjects.has(entity)) {
            transform.translate(particle.Position + CameraSystem::GetInstance()->position);
        } else {
            transform.translate(particle.Position);
        }
        transform.rotate(particle.Rotation);
        transform.scale({ size * 10.0f, size * 10.0f });

        transforms.push_back(transform.mat);
        colors.push_back(color);
    }

    // Determine the number of instances.
    unsigned int instanceCount = transforms.size();
    if (instanceCount == 0)
        return;

    glBindBuffer(GL_ARRAY_BUFFER, m_InstanceTransformVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, instanceCount * sizeof(mat3), transforms.data());

    glBindBuffer(GL_ARRAY_BUFFER, m_InstanceColorVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, instanceCount * sizeof(vec4), colors.data());

    glBindVertexArray(m_QuadVAO);

    // INSTANCE RENDERING: LOOK HERE TA THIS IS THE CODE!!!!
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, instanceCount);

    glBindVertexArray(m_VAO);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// first draw to an intermediate texture,
// apply the "vignette" texture, when requested
// then draw the intermediate texture
void RenderSystem::drawToScreen() {
    // Setting shaders
    // get the vignette texture, sprite mesh, and program
    glUseProgram(effects[(GLuint) EFFECT_ASSET_ID::VIGNETTE]);
    gl_has_errors();

    // Clearing backbuffer
    int w, h;
    glfwGetFramebufferSize(window,
                           &w,
                           &h);  // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, w, h);
    glDepthRange(0, 10);
    glClearColor(1.f, 0, 0, 1.0);
    glClearDepth(1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gl_has_errors();
    // Enabling alpha channel for textures
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    // Draw the screen texture on the quad geometry
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint) GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
    glBindBuffer(
        GL_ELEMENT_ARRAY_BUFFER,
        index_buffers[(GLuint) GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);  // Note, GL_ELEMENT_ARRAY_BUFFER associates
                                                                       // indices to the bound GL_ARRAY_BUFFER
    gl_has_errors();

    // add the "vignette" effect
    const GLuint vignette_program = effects[(GLuint) EFFECT_ASSET_ID::VIGNETTE];

    // set clock
    GLuint time_uloc = glGetUniformLocation(vignette_program, "time");
    GLuint dead_timer_uloc = glGetUniformLocation(vignette_program, "darken_screen_factor");

    glUniform1f(time_uloc, (float) (glfwGetTime() * 10.0f));

    ScreenState& screen = registry.screenStates.get(screen_state_entity);
    // std::cout << "screen.darken_screen_factor: " << screen.darken_screen_factor << " entity id: " <<
    // screen_state_entity << std::endl;
    glUniform1f(dead_timer_uloc, screen.darken_screen_factor);
    gl_has_errors();

    // Set the vertex position and vertex texture coordinates (both stored in the
    // same VBO)
    GLint in_position_loc = glGetAttribLocation(vignette_program, "in_position");
    glEnableVertexAttribArray(in_position_loc);
    glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*) 0);
    gl_has_errors();

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
    gl_has_errors();

    // Draw
    glDrawElements(GL_TRIANGLES,
                   3,
                   GL_UNSIGNED_SHORT,
                   nullptr);  // one triangle = 3 vertices; nullptr indicates that there is
                              // no offset from the bound index buffer
    gl_has_errors();
}


// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw() {
    // Getting size of window
    int w, h;
    glfwGetFramebufferSize(window,
                           &w,
                           &h);  // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

    // First render to the custom framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
    gl_has_errors();

    // clear backbuffer
    glViewport(0, 0, w, h);
    glDepthRange(0.00001, 10);

    // white background
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glClearDepth(10.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);  // native OpenGL does not work with a depth buffer
                               // and alpha blending, one would have to sort
                               // sprites back to front
    gl_has_errors();

    mat3 projection_2D = createProjectionMatrix();
    glm::mat4 UI_Matrix = mat4(1.0f);

    // Render Disaster whirlpool
    for (Entity entity : registry.disasters.entities) {
        if (registry.disasters.get(entity).type == DISASTER_TYPE::WHIRLPOOL)
            drawTexturedMesh(entity, projection_2D);
    }

    highlight_count = 0;
    // draw all entities with a render request to the frame buffer
    for (Entity entity : registry.renderRequests.entities) {
        if (registry.spotlights.has(entity)) {
            float diagonal = sqrt(WINDOW_WIDTH_PX * WINDOW_WIDTH_PX + WINDOW_HEIGHT_PX * WINDOW_HEIGHT_PX);
            float rad = registry.spotlights.get(entity).radius / diagonal;
            if (rad != 0) {
                vec2 pos = registry.spotlights.get(entity).position;
                if (registry.backgroundObjects.has(entity)) {
                    pos += vec2((CameraSystem::GetInstance()->position.x) * WINDOW_WIDTH_PX / WINDOW_HEIGHT_PX,
                                CameraSystem::GetInstance()->position.y);
                }
                pos = pos / vec2(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX) - vec2(0.5, 0.5);
                if (registry.backgroundObjects.has(entity)) pos.x -= rad;
                highlight_centers[highlight_count] = vec3(pos.x, pos.y, rad);
                highlight_count += 1;
            }
        }
        // filter to entities that have a motion component
        if (registry.motions.has(entity)) {
            // SKIP PLAYER TO RENDER THEM LAST.
            if (registry.players.has(entity)) continue;

            if (registry.disasters.has(entity)) {
                if (registry.disasters.get(entity).type == DISASTER_TYPE::TORNADO) {
                    continue;
                }
            }
            if (registry.bunnies.has(entity)) {
                if (registry.bunnies.get(entity).on_module) continue;
            }

            // Note, its not very efficient to access elements indirectly via the entity
            // albeit iterating through all Sprites in sequence. A good point to optimize
            drawTexturedMesh(entity, projection_2D);
        }
        // draw grid lines separately, as they do not have motion but need to be rendered
        else if (registry.gridLines.has(entity)) {
            drawGridLine(entity, projection_2D);
        }
    }

    for (Entity e : registry.particleEmitters.entities) {
        drawParticles(e, projection_2D);
    }

    // Render Disaster tornado above bg/islands/enemies
    for (Entity entity : registry.disasters.entities) {
        if (registry.disasters.get(entity).type == DISASTER_TYPE::TORNADO) {
            drawTexturedMesh(entity, projection_2D);
        }
    }

    // Brian: Add draw UI components here.
    SceneManager& sm = SceneManager::getInstance();
    Scene* s = sm.getCurrentScene();
    if (s) {
        bnuui::SceneUI scene_ui = s->getUIElems();
        if (!isPaused) {
            std::vector<std::shared_ptr<bnuui::Element>> elems = scene_ui.getElems();
			for (std::shared_ptr<bnuui::Element> elem : elems) {
        if (elem->over_overlay) continue; // skip the ones above overlay
				if (elem->getText() != "") {
					renderText(elem->getText(), elem->position.x, WINDOW_HEIGHT_PX - elem->position.y, elem->getFontSize(), elem->color, UI_Matrix);
				} else {
					drawUIElement(*elem, projection_2D);
				}
			}           
        } else {
            // Draw only the pause UI.
            drawUIElement(*scene_ui.getPauseUI(), projection_2D);
        }
    }

    if (registry.overlays.components.size() > 0) {
        Entity overlay_entity = registry.overlays.entities[0];
        if (registry.renderRequests.has(overlay_entity)) {
            drawOverlay(overlay_entity, projection_2D);
        }
    }

    if (isInGame && !isRenderingGacha && !isRenderingBook) {
        Player& player = registry.players.components[0];
        if (player.player_state == STATIONING) {
            vec2& player_position = registry.motions.get(registry.players.entities[0]).position;
            int player_tile_x = (int) (player_position.x / GRID_CELL_WIDTH_PX);
            int player_tile_y = (int) (player_position.y / GRID_CELL_HEIGHT_PX);
            vec2 highlight_position = TileToVector2(player_tile_x, player_tile_y);
            drawSquareOutline(highlight_position, {56.f, 56.f}, 
                vec3(190 / 255.f, 209 / 255.f, 237 / 255.f), projection_2D);
        }
    }

    // Dayshaun: draw the UI elements over the shaded overlay
    if (s) {
        bnuui::SceneUI scene_ui = s->getUIElems();
        std::vector<std::shared_ptr<bnuui::Element>> elems = scene_ui.getElems();
        for (std::shared_ptr<bnuui::Element> elem : elems) {
            if (!elem->over_overlay) continue; // skip the ones under overlay
            if (elem->getText() != "") {
                renderText(elem->getText(),
                           elem->position.x,
                           WINDOW_HEIGHT_PX - elem->position.y,
                           elem->getFontSize(),
                           elem->color,
                           UI_Matrix);
            } else {
                drawUIElement(*elem, projection_2D);
            }
        }
    }
  
    // if there is no gacha ui displayed
    // std::cout << "Gacha rendering? " << isRenderingGacha<< std::endl; 
    if(!isRenderingGacha && !isRenderingBook){
        // Render Player.
        for (Entity entity : registry.players.entities) {
            if (registry.motions.has(entity) && registry.renderRequests.has(entity))
                drawTexturedMesh(entity, projection_2D);
        }
    }


    // draw framebuffer to screen
    // adding "vignette" effect when applied
    drawToScreen();

    // flicker-free display with a double buffer
    glfwSwapBuffers(window);
    gl_has_errors();
}

mat4 RenderSystem::createUIMatrix() {
    return glm::ortho(0.0f, static_cast<float>(WINDOW_WIDTH_PX), 0.0f, static_cast<float>(WINDOW_HEIGHT_PX));
}

mat3 RenderSystem::createProjectionMatrix() {
    // fake projection matrix, scaled to window coordinates
    float left = 0.f;
    float top = 0.f;
    float right = (float) WINDOW_WIDTH_PX;
    float bottom = (float) WINDOW_HEIGHT_PX;

    float sx = 2.f / (right - left);
    float sy = 2.f / (top - bottom);
    float tx = -(right + left) / (right - left);
    float ty = -(top + bottom) / (top - bottom);

    return {{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f}};
}

void RenderSystem::renderText(std::string text, float x, float y, float scale, const glm::vec3& color, const glm::mat4& trans) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // activate corresponding render state
    glUseProgram(m_font_shaderProgram);

    GLint textColor_location = glGetUniformLocation(m_font_shaderProgram, "textColor");
    assert(textColor_location > -1);
    // std::cout << "textColor_location: " << textColor_location << std::endl;
    glUniform3f(textColor_location, color.x, color.y, color.z);

    auto transformLoc = glGetUniformLocation(m_font_shaderProgram, "transform");
    // std::cout << "transformLoc: " << transformLoc << std::endl;
    assert(transformLoc > -1);
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

    glBindVertexArray(m_font_VAO);

    // iterate through each character
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = m_ftCharacters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };

        // render glyph texture over quad

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        /*std::cout << "binding texture: " << ch.character << " = " << ch.TextureID << std::endl;*/

        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, m_font_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // advance to next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(m_VAO);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderSystem::drawSquareOutline(vec2 position, vec2 size, vec3 color, const mat3& projection) {
    glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::EGG]); // or a dedicated square shader
    gl_has_errors();

    // Use a simple colored quad geometry (reuse EGG or add a new one)
    GLuint vbo = vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::HIGHLIGHT_SQUARE];
    GLuint ibo = index_buffers[(GLuint)GEOMETRY_BUFFER_ID::HIGHLIGHT_SQUARE];

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    gl_has_errors();

    GLint in_position_loc = glGetAttribLocation(effects[(GLuint)EFFECT_ASSET_ID::EGG], "in_position");
    GLint in_color_loc = glGetAttribLocation(effects[(GLuint)EFFECT_ASSET_ID::EGG], "in_color");
    glEnableVertexAttribArray(in_position_loc);
    glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (void*)0);
    glEnableVertexAttribArray(in_color_loc);
    glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (void*)sizeof(vec3));
    gl_has_errors();

    // Set uniforms
    GLint color_uloc = glGetUniformLocation(effects[(GLuint)EFFECT_ASSET_ID::EGG], "fcolor");
    glUniform3fv(color_uloc, 1, (float*)&color);

    Transform transform;
    transform.translate(position);
    transform.scale(size);

    GLuint transform_loc = glGetUniformLocation(effects[(GLuint)EFFECT_ASSET_ID::EGG], "transform");
    glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float*)&transform.mat);

    GLuint projection_loc = glGetUniformLocation(effects[(GLuint)EFFECT_ASSET_ID::EGG], "projection");
    glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&projection);

    // Draw
    GLint size_bytes = 0;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size_bytes);
    GLsizei num_indices = size_bytes / sizeof(uint16_t);

    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
}


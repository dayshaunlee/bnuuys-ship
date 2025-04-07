#version 330

// From Vertex Shader
in vec3 vcolor;
in vec2 vpos;

// Application data
uniform vec3 fcolor;
uniform float alpha;
uniform bool visible;

uniform int num_centers; // actual amount of highlights
uniform vec3 centers[5]; // max of 5 highlights
// Output color
layout(location = 0) out vec4 color;

void main()
{  
    if (!visible) {
        discard;
    }

    for (int i = 0; i < num_centers; ++i) {
        // 1.36 = WINDOW_WIDTH_PX / WINDOW_HEIGHT_PX (change scaling with aspect ratio)
        float dist = length(vec2(vpos.x * 1.36, vpos.y) - vec2(centers[i].x, centers[i].y));
        if (dist < centers[i].z * 2) {
            discard;
        }
    }
    //color = vec4(vpos.x, vpos.y, 0.0, 1.0);
    color = vec4(fcolor * vcolor, alpha);
    // color = vec4(vec3(centers[0].x, centers[0].y, 0.0), alpha);
}
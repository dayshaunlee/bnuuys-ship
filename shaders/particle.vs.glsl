#version 330

// Per-vertex attribute for the quad's position.
layout (location = 1) in vec3 a_Position;

// Per-instance attributes for the particle transform.
// We use three vec3's to form a 3x3 matrix.
layout (location = 2) in vec3 instanceTransformRow0;
layout (location = 3) in vec3 instanceTransformRow1;
layout (location = 4) in vec3 instanceTransformRow2;

// Per-instance attribute for the particle color.
layout (location = 5) in vec4 instanceColor;

uniform mat3 u_ViewProj;

out vec4 f_Color;  // Pass the instance color to the fragment shader.

void main()
{
    // Reconstruct the per-instance transform matrix.
    mat3 instanceTransform = mat3(instanceTransformRow0, instanceTransformRow1, instanceTransformRow2);
    
    // Transform the vertex position by the instance transform and then by the view-projection matrix.
    vec3 pos = u_ViewProj * instanceTransform * vec3(a_Position.xy, 1.0);
    gl_Position = vec4(pos.xy, a_Position.z, 1.0);
    
    // Pass along the per-instance color.
    f_Color = instanceColor;
}

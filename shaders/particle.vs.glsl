#version 330

layout (location = 1) 
in vec3 a_Position;

uniform mat3 u_ViewProj;
uniform mat3 u_Transform;

void main()
{
	vec3 pos = u_ViewProj * u_Transform * vec3(a_Position.xy, 1.0);
	gl_Position = vec4(pos.xy, a_Position.z, 1.0);
}

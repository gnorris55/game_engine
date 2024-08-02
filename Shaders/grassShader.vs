#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in float height;
layout (location = 2) in vec3 grass_params;

out vec3 FragPos;
out VS_OUT {
    float height;
    vec3 grass_params;
} vs_out;

void main()
{
    vs_out.height = height;
    vs_out.grass_params = grass_params;

    gl_Position = vec4(aPos, 1.0);

}
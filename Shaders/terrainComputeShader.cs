#version 460 core

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D clippingTex;

//uniform vec3 position;
uniform mat4 projView;
uniform mat4 model;
uniform vec3 camera_position;

void main()
{

    vec3 position;
    vec4 value = vec4(0.0, 0.0, 0.0, 1.0);
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    position.x = texelCoord.x * 2;
    position.y = 0;
    position.z = texelCoord.y * 2;
    position = (vec4(position, 1.0) * model).xyz;
    float distance = length(position - camera_position);
    if (distance > 70) {
        value = vec4(0.0, 0.0, 0.0, 0.0);
    }   else if (distance > 10) {
        value = vec4(0.5, 0.0, 0.0, 1.0);
    }   else {
        value = vec4(1.0, 1.0, 1.0, 1.0);
    }
    
    value.y = length(position - camera_position);
    value.z = position.z;
    value.z = texelCoord.x*2;
    //imageStore(clippingTex, texelCoord, vec4(position, 1.0)*model);
    //imageStore(clippingTex, texelCoord, point_c);
    imageStore(clippingTex, texelCoord, value);

}
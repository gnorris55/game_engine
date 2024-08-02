#version 430 core

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D grass_tex1;
layout(rgba32f, binding = 1) uniform image2D grass_tex2;

uniform vec2 position;

float rand(vec2 co)
{
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    vec4 value = vec4(0.0, 0.0, 0.0, 1.0);
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    vec4 texelColor1 = imageLoad(grass_tex1, texelCoord);
    vec4 texelColor2 = imageLoad(grass_tex1, texelCoord);
    //for determining grass xz displacement
    texelColor1.x = rand(position+texelCoord);

    texelColor1.y = rand(100+position*10+texelCoord*0.5f);
    //grass height
    texelColor1.z = rand(position*100 + 22+ texelCoord*0.25);
    //grass angle
    texelColor2.x = rand(position*100 + 50 + texelCoord*0.75);
    //grass curve
    texelColor2.y = rand(position*100 + 33 +  texelCoord*0.876);
    // extra
    texelColor2.z = rand(position*100 + 75 + texelCoord*0.333);


    imageStore(grass_tex1, texelCoord, texelColor1);
    imageStore(grass_tex2, texelCoord, texelColor2);

}
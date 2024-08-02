#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;
out vec3 tn;

uniform mat4 model;
uniform mat4 projView;

void main()
{
    FragPos = vec3(vec4(aPos, 1.0)*model);
    Normal = aNormal * mat3(transpose(inverse(model)));  
    tn = aNormal;
    gl_Position = projView * vec4(FragPos, 1.0);

    TexCoords = aTexCoords;

}

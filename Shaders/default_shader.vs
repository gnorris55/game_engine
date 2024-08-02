#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(vec4(aPos, 1.0)*model);
    Normal = aNormal * mat3(transpose(inverse(model)));  
        
    gl_Position = projection * view * vec4(FragPos, 1.0);
    TexCoords = aTexCoords*60.0f;
    //TexCoords = aTexCoords;

}
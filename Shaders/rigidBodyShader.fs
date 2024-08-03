#version 460 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos; 
in vec2 TexCoords;
// texture samplers
uniform vec3 lightPos; 
uniform vec3 viewPos; 
uniform vec3 lightColor;
uniform vec3 objectColor;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D Normal1;
uniform bool hasTexture;

void main()
{
// ambient
    float ambientStrength = 0.5;
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse_vec = diff * vec3(texture(texture_diffuse1, TexCoords))* lightColor;
    
    // specular Blinn-phong method
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 16);


    vec3 specular = specularStrength * spec * vec3(texture(texture_specular1, TexCoords))*lightColor;  
    vec3 result = (ambient + diffuse_vec + specular) * objectColor; 




    if (hasTexture) {
        vec3 texCol = texture(texture_diffuse1, TexCoords).rgb;      
        FragColor = vec4(result, 1.0) * vec4(texCol, 1.0);
    } else {
        FragColor = vec4(result, 1.0);
    }
}

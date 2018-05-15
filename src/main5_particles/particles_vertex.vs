#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in mat4 instanceMatrix;
layout (location = 6) in float aAlpha;

out vec3 Normal;
out vec3 FragPos;
out float Alpha;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0f);
    Normal = mat3(transpose(inverse(instanceMatrix))) * aNormal;
    FragPos = vec3(instanceMatrix * vec4(aPos, 1.0));
    Alpha = aAlpha;
}
#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

// texture samplers
uniform sampler2D texture1;

void main()
{
	// linearly interpolate between both textures (80% container, 20% awesomeface)

	float ambientStrength = 0.1;
	// vec3 ambient = ambientStrength * lightColor;
	vec3 ambient = ambientStrength * lightColor; 
	// FragColor = vec4(ambient * ourColor, 1.0f);
	// FragColor = mix(ambient * texture(texture1, TexCoord), ambient * texture(texture2, TexCoord), 0.2);

	// diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // specular
    float specularStrength = 0.8;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor; 

	vec3 result = (ambient + diffuse + specular) * texture(texture1, TexCoord).rgb;
	FragColor = vec4(result, 1.0);
}

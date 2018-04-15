#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;
uniform vec3 lightColor;

// texture samplers
uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
	// linearly interpolate between both textures (80% container, 20% awesomeface)

	float ambientStrength = 0.5;
	// vec3 ambient = ambientStrength * lightColor;
	vec3 ambient = ambientStrength * texture(texture1, TexCoord).rgb; 
	// FragColor = vec4(ambient * ourColor, 1.0f);
	// FragColor = mix(ambient * texture(texture1, TexCoord), ambient * texture(texture2, TexCoord), 0.2);
	FragColor = vec4(ambient, 1.0);
}

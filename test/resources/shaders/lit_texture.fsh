#version 330 core
out vec4 fragColour;

in vec3 normal;
in vec3 fragPos;
in vec2 texCoord;

struct Material
{
	sampler2D diffuse1;
	sampler2D specular1;
	float shininess;
};

struct Light
{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Material material;
uniform Light light;

uniform vec3 viewPos;
uniform vec4 tint = vec4(1.0f, 1.0f, 1.0f, 1.0f);

void main()
{
	vec3 norm = normalize(normal);

	// Ambient
	vec3 ambient = light.ambient * vec3(texture(material.diffuse1, texCoord));

	// Diffuse
	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = light.diffuse * (diff * vec3(texture(material.diffuse1, texCoord)));

	// Specular
	vec3 reflectDir = reflect(-lightDir, norm);
	vec3 viewDir = normalize(viewPos - fragPos);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * (spec * vec3(texture(material.specular1, texCoord)));

	// Final
	vec3 result = ambient + diffuse + specular;
	vec4 tint2 = vec4(1.0f, 0.0f, 1.0f, 1.0f);
	fragColour = vec4(result, 1.0) * tint;
	//fragColour = vec4(result, 1.0) * tint2;
}

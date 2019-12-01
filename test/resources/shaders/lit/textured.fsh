#ifdef GL_ES
	precision mediump float;
#endif

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

struct PtLight
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	vec3 position;

	float constant;
	float linear;
	float quadratic;
};

struct DirLight
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	vec3 direction;
};

#define MAX_PT_LIGHTS 4
#define MAX_DIR_LIGHTS 4

uniform Material material;
uniform int dirLightCount;
uniform PtLight ptLights[MAX_PT_LIGHTS];
uniform int ptLightCount;
uniform DirLight dirLights[MAX_DIR_LIGHTS];
uniform vec3 viewPos;
#ifdef GL_ES
	uniform vec4 tint;
#else
	uniform vec4 tint = vec4(1.0f, 1.0f, 1.0f, 1.0f);
#endif

vec3 calcDirLight(DirLight light, vec3 norm, vec3 toView)
{
	vec3 toLight = normalize(-light.direction);
	float diff = max(dot(norm, toLight), 0.0);
	vec3 reflectDir = reflect(-toLight, norm);
	float spec = pow(max(dot(toView, reflectDir), 0.0), material.shininess);
	vec3 ambient  = light.ambient  * vec3(texture(material.diffuse1, texCoord));
	vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse1, texCoord));
	vec3 specular = light.specular * spec * vec3(texture(material.specular1, texCoord));
	return ambient + diffuse + specular;
}

vec3 calcPtLight(PtLight light, vec3 norm, vec3 fragPos, vec3 toView)
{
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + distance * light.linear + distance * distance * light.quadratic);
	vec3 toLight = normalize(light.position - fragPos);
	vec3 reflectDir = reflect(-toLight, norm);
	float diff = max(dot(norm, toLight), 0.0);
	float spec = pow(max(dot(toView, reflectDir), 0.0), material.shininess);
	vec3 ambient = light.ambient * vec3(texture(material.diffuse1, texCoord)) * attenuation;
	vec3 diffuse = light.diffuse * (diff * vec3(texture(material.diffuse1, texCoord))) * attenuation;
	vec3 specular = light.specular * (spec * vec3(texture(material.specular1, texCoord))) * attenuation;
	return ambient + diffuse + specular;
}

void main()
{
	vec3 norm = normalize(normal);
	vec3 toView = normalize(viewPos - fragPos);

	vec3 result = vec3(0.0f);
	for (int i = 0; i < dirLightCount; i++)
	{
		result += calcDirLight(dirLights[i], norm, toView);
	}
	for (int i = 0; i < ptLightCount; ++i)
	{
		result += calcPtLight(ptLights[i], norm, fragPos, toView);
	}
	fragColour = vec4(result, 1.0) * tint;
}

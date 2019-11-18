#ifdef GL_ES
	precision mediump float;
#endif

out vec4 fragColour;

in vec3 normal;
in vec3 fragPos;

struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

struct PointLight
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

#define POINT_LIGHT_COUNT 4
#define DIR_LIGHT_COUNT 4

uniform Material material;
uniform PointLight pointLights[POINT_LIGHT_COUNT];
uniform DirLight dirLights[DIR_LIGHT_COUNT];
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
	vec3 ambient  = light.ambient  * material.ambient;
	vec3 diffuse  = light.diffuse  * diff * material.diffuse;
	vec3 specular = light.specular * spec * material.specular;
	return ambient + diffuse + specular;
}

vec3 calcPointLight(PointLight light, vec3 norm, vec3 fragPos, vec3 toView)
{
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + distance * light.linear + distance * distance * light.quadratic);
	vec3 toLight = normalize(light.position - fragPos);
	vec3 reflectDir = reflect(-toLight, norm);
	float diff = max(dot(norm, toLight), 0.0);
	float spec = pow(max(dot(toView, reflectDir), 0.0), material.shininess);
	vec3 ambient = light.ambient * material.ambient * attenuation;
	vec3 diffuse = light.diffuse * (diff * material.diffuse) * attenuation;
	vec3 specular = light.specular * (spec * material.specular) * attenuation;
	return ambient + diffuse + specular;
}

void main()
{
	vec3 norm = normalize(normal);
	vec3 toView = normalize(viewPos - fragPos);

	vec3 result = vec3(0.0f);
	for (int i = 0; i < DIR_LIGHT_COUNT; i++)
	{
		result += calcDirLight(dirLights[i], norm, toView);
	}
	for (int i = 0; i < POINT_LIGHT_COUNT; ++i)
	{
		result += calcPointLight(pointLights[i], norm, fragPos, toView);
	}
	fragColour = vec4(result, 1.0) * tint;
}

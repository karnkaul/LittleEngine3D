#ifdef GL_ES
	precision mediump float;
#endif

#define MAX_PT_LIGHTS 4
#define MAX_DIR_LIGHTS 4

out vec4 fragColour;

in vec3 normal;
in vec3 fragPos;
in vec3 viewPos;

struct Albedo
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

struct Material
{
	Albedo albedo;
};

struct PtLight
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 position;
	vec4 clq;
};

struct DirLight
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 direction;
};

layout (std140) uniform Lights
{
	PtLight ptLights[MAX_PT_LIGHTS];
	DirLight dirLights[MAX_DIR_LIGHTS];
};

uniform Material material;
#ifdef GL_ES
	uniform vec4 tint;
#else
	uniform vec4 tint = vec4(1.0);
#endif

vec3 calcDirLight(DirLight light, vec3 norm, vec3 toView)
{
	vec3 toLight = normalize(-vec3(light.direction));
	vec3 reflectDir = reflect(-toLight, norm);
	float diff = max(dot(norm, toLight), 0.0);
	float spec = pow(max(dot(toView, reflectDir), 0.0), material.albedo.shininess);
	vec3 ambient  = vec3(light.ambient)  * material.albedo.ambient;
	vec3 diffuse  = vec3(light.diffuse)  * diff * material.albedo.diffuse;
	vec3 specular = vec3(light.specular) * spec * material.albedo.specular;
	vec3 total = max(ambient, 0.0) + max(diffuse, 0.0) + max(specular, 0.0);
	return total;
}

vec3 calcPtLight(PtLight light, vec3 norm, vec3 fragPos, vec3 toView)
{
	vec3 toLight = vec3(light.position) - fragPos;
	vec3 nToLight = normalize(toLight);
	vec3 reflectDir = reflect(-nToLight, norm);
	float distance = length(toLight);
	float attenuation = 1.0 / (light.clq.x + distance * light.clq.y + distance * distance * light.clq.z);
	float diff = max(dot(norm, nToLight), 0.0);
	float spec = pow(max(dot(toView, reflectDir), 0.0), material.albedo.shininess);
	vec3 ambient = vec3(light.ambient) * material.albedo.ambient * attenuation;
	vec3 diffuse = vec3(light.diffuse) * (diff * material.albedo.diffuse) * attenuation;
	vec3 specular = vec3(light.specular) * (spec * material.albedo.specular) * attenuation;
	vec3 total = max(ambient, 0.0) + max(diffuse, 0.0) + max(specular, 0.0);
	return total;
}

void main()
{
	vec3 norm = normalize(normal);
	vec3 toView = normalize(viewPos - fragPos);
	vec3 result = vec3(0.0);
	for (int i = 0; i < MAX_DIR_LIGHTS; i++)
	{
		result += calcDirLight(dirLights[i], norm, toView);
	}
	for (int i = 0; i < MAX_PT_LIGHTS; ++i)
	{
		result += calcPtLight(ptLights[i], norm, fragPos, toView);
	}
	fragColour = vec4(result, 1.0) * tint;
}

#ifdef GL_ES
	precision mediump float;
#endif

#define MAX_PT_LIGHTS 4
#define MAX_DIR_LIGHTS 4

out vec4 fragColour;

in vec3 normal;
in vec3 fragPos;
in vec3 viewPos;
in vec2 texCoord;

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
	sampler2D diffuse;
	sampler2D specular;
	float hasSpecular;
	int isOpaque;
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

vec4 calcDirColour(DirLight light, vec4 diffTexColour, vec4 specTexColour, float diff, float spec)
{
	vec4 ambient  = vec4(vec3(light.ambient), 1.0) * diffTexColour;
	vec4 diffuse  = vec4(vec3(light.diffuse), 1.0) * diff * diffTexColour;
	vec4 specular = vec4(vec3(light.specular), 1.0) * spec * specTexColour;
	return max(ambient + diffuse, 0.0) + max(specular, 0.0);
}

vec4 calcPtColour(PtLight light, vec4 diffTexColour, vec4 specTexColour, float diff, float spec, float attenuation)
{
	vec4 ambient = vec4(vec3(light.ambient), 1.0) * diffTexColour * attenuation;
	vec4 diffuse = vec4(vec3(light.diffuse), 1.0) * diffTexColour * attenuation;
	vec4 specular = vec4(vec3(light.specular), 1.0) * spec * specTexColour * attenuation;
	return max(ambient + diffuse, 0.0) + max(specular, 0.0);
}

vec4 calcDirLight(DirLight light, vec4 diffTexColour, vec4 specTexColour, vec3 norm, vec3 toView)
{
	vec3 nToLight = normalize(-vec3(light.direction));
	vec3 reflectDir = reflect(-nToLight, norm);
	float diff = max(dot(norm, nToLight), 0.0);
	float spec = pow(max(dot(toView, reflectDir), 0.0), material.albedo.shininess);
	return calcDirColour(light, diffTexColour, specTexColour, diff, spec);
}

vec4 calcPtLight(PtLight light, vec4 diffTexColour, vec4 specTexColour, vec3 norm, vec3 fragPos, vec3 toView)
{
	vec3 toLight = vec3(light.position) - fragPos;
	vec3 nToLight = normalize(toLight);
	vec3 reflectDir = reflect(-nToLight, norm);
	float distance = length(toLight);
	float attenuation = 1.0 / (light.clq.x + distance * light.clq.y + distance * distance * light.clq.z);
	float diff = max(dot(norm, nToLight), 0.0);
	float spec = pow(max(dot(toView, reflectDir), 0.0), material.albedo.shininess);
	return calcPtColour(light, diffTexColour, specTexColour, diff, spec, attenuation);
}

void main()
{
	vec4 result = vec4(0.0);
	vec3 norm = normalize(normal);
	vec3 toView = normalize(viewPos - fragPos);
	vec4 diffTexColour = texture(material.diffuse, texCoord) * vec4(material.albedo.diffuse + material.albedo.ambient, 1.0);
	vec4 specTexColour = texture(material.specular, texCoord) * vec4(material.albedo.specular, 1.0) * material.hasSpecular;
	if (material.isOpaque == 1)
	{
		diffTexColour.a = 1.0;
		specTexColour.a = 1.0;
	}
	for (int i = 0; i < MAX_DIR_LIGHTS; i++)
	{
		result += calcDirLight(dirLights[i], diffTexColour, specTexColour, norm, toView);
	}
	for (int i = 0; i < MAX_PT_LIGHTS; ++i)
	{
		result += calcPtLight(ptLights[i], diffTexColour, specTexColour, norm, fragPos, toView);
	}
	if (result.a < 0.1)
	{
		discard;
	}
	fragColour = result * tint;
}

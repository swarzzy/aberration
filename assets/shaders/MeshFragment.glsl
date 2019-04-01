in Vector3 f_Position;
in Vector2 f_UV;
in Vector3 f_Normal;

out vec4 color;

#define POINT_LIGHTS_NUMBER  2

struct Material {
	bool use_diff_map;
	bool use_spec_map;
	sampler2D diffuse_map;
	sampler2D spec_map;
	float shininess;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight {
	Vector3 position;
	Vector3 ambient;
	Vector3 diffuse;
	Vector3 specular;
	float32 linear;
	float32 quadratic;
};

struct DirLight {
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Material material;
layout (std140) uniform pointLightsData {
	PointLight pointLights[POINT_LIGHTS_NUMBER];
};
uniform DirLight dir_light;


vec3 CalcDirectionalLight(DirLight light, vec3 normal, vec3 view_dir, vec3 diff_sample, vec3 spec_sample) {
	vec3 light_dir = normalize(-light.direction);
	vec3 light_dir_reflected = reflect(-light_dir, normal);	

	float Kd = max(dot(normal, light_dir), 0.0);
	float Ks = pow(max(dot(view_dir, light_dir_reflected ), 0.0), material.shininess);
	
	vec3 ambient = light.ambient * diff_sample ;
	vec3 diffuse = Kd * light.diffuse * diff_sample;
	vec3 specular = Ks * light.specular * spec_sample;
	return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight light, Vector3 normal, Vector3 viewDir, Vector3 diffSample, Vector3 specSample) {
	float32 distance = length(light.position - f_Position);
	float32 attenuation = 1.0f / (1.0f + 
								light.linear * distance + 
								light.quadratic * distance * distance);
	
	Vector3 lightDir = normalize(light.position - f_Position);
	Vector3 reflectDir = reflect(-lightDir, normal);
	float32 Kd = max(dot(normal, lightDir), 0.0);
	float32 Ks = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	Vector3 ambient = light.ambient * diffSample * attenuation;
	Vector3 diffuse = light.diffuse * Kd * diffSample * attenuation;
	Vector3 specular = Ks * light.specular * specSample * attenuation;
	return ambient + diffuse + specular;
}

void main()
{
	vec3 normal = normalize(f_Normal);
	vec3 viewDir = normalize(sys_ViewPos - f_Position);

	vec3 diffSample;
	float32 alpha;
	if (material.use_diff_map) {
		Vector4 _sample = texture(material.diffuse_map, f_UV); 
		diffSample = _sample.rgb;
		alpha = _sample.a;
	} else {
		diffSample = material.diffuse;
		alpha = 1.0f;
	}

	vec3 specSample;
	if (material.use_spec_map) {
		specSample = texture(material.spec_map, f_UV).xyz;
	} else {
		specSample = material.specular;
	}

	vec3 directional = CalcDirectionalLight(dir_light, normal, viewDir, diffSample, specSample);

	vec3 point = vec3(0.0f);
	for (int i = 0; i < POINT_LIGHTS_NUMBER; i++) {
		point += CalcPointLight(pointLights[i], f_Normal, viewDir, diffSample, specSample);
	}

	vec3 sum_point = clamp(point, 0.0f, 1.0f);

	color = vec4(sum_point + directional, alpha);
	//color = vec4(sum_point + directional, 1.0f);

}

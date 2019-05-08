in v3 f_Position;
in v2 f_UV;
in v3 f_Normal;

out v4 color;

#define POINT_LIGHTS_NUMBER  4

struct Material {
	bool use_diff_map;
	bool use_spec_map;
	sampler2D diffuse_map;
	sampler2D spec_map;
	f32 shininess;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight {
	v3 position;
	v3 ambient;
	v3 diffuse;
	v3 specular;
	f32 linear;
	f32 quadratic;
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
uniform v3 u_ViewPos;


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

vec3 CalcPointLight(PointLight light, v3 normal, v3 viewDir, v3 diffSample, v3 specSample) {
	f32 distance = length(light.position - f_Position);
	f32 attenuation = 1.0f / (1.0f + 
								light.linear * distance + 
								light.quadratic * distance * distance); 
	
	v3 lightDir = normalize(light.position - f_Position);
	v3 reflectDir = reflect(-lightDir, normal);
	f32 Kd = max(dot(normal, lightDir), 0.0);
	f32 Ks = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	v3 ambient = light.ambient * diffSample * attenuation;
	v3 diffuse = light.diffuse * Kd * diffSample * attenuation;
	v3 specular = Ks * light.specular * specSample * attenuation;
	return ambient + diffuse + specular;
}

void main()
{
	vec3 normal = normalize(f_Normal);
	vec3 viewDir = normalize(u_ViewPos - f_Position);

	vec3 diffSample;
	f32 alpha;
	if (material.use_diff_map) {
		v4 _sample = texture(material.diffuse_map, f_UV); 
		diffSample = _sample.rgb;
		alpha = _sample.a;
	} else {
		diffSample = material.diffuse;
		alpha = 1.0f;
	}

	//if (alpha == 0.0f) {
	//	discard;
	//}

	//alpha = (alpha - 0.1) / max(fwidth(alpha), 0.0001) - 0.5;

	vec3 specSample;
	if (material.use_spec_map) {
		specSample = texture(material.spec_map, f_UV).xyz;
	} else {
		specSample = material.specular;
	}

	vec3 directional = CalcDirectionalLight(dir_light, normal, viewDir, diffSample, specSample);

	vec3 point = vec3(0.0f);
	for (int i = 0; i < POINT_LIGHTS_NUMBER; i++) {
		point += CalcPointLight(pointLights[i], normal, viewDir, diffSample, specSample);
	}

	vec3 sum_point = point;
	color = v4(sum_point + directional, alpha);
	//color = v4(alpha, alpha, alpha, 1.0);

}

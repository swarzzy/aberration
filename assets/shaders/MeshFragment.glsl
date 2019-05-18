in v3 f_Position;
in v4 f_LightSpacePosition;
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
uniform sampler2D shadowMap;


vec3 CalcDirectionalLight(DirLight light, vec3 normal,
						  vec3 view_dir,
						  vec3 diff_sample, vec3 spec_sample,
						  f32 shadowKoef)
{
	vec3 light_dir = normalize(-light.direction);
	vec3 light_dir_reflected = reflect(-light_dir, normal);	

	float Kd = max(dot(normal, light_dir), 0.0);
	float Ks = pow(max(dot(view_dir, light_dir_reflected ), 0.0), material.shininess);
	
	vec3 ambient = light.ambient * diff_sample ;
	vec3 diffuse = Kd * light.diffuse * diff_sample * shadowKoef;
	vec3 specular = Ks * light.specular * spec_sample * shadowKoef;
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

f32 CalcShadow(v4 lightSpacePos, v3 lightDir, v3 normal)
{
	f32 result = 0.0f;
	//f32 bias = 0.0f;
	//f32 bias = 0.005f;
	f32 bias = max(0.005f * (1.0f - dot(normal, lightDir)), 0.0005f);
	v3 coord = lightSpacePos.xyz * 0.5f + 0.5f;
	f32 currentDepth = coord.z;
	v2 texelSize = 1.0f / textureSize(shadowMap, 0);
	if (lightSpacePos.z > 1.0f)
	{
		result = 0.0f;
	}
	else
	{
		for (int x = -1; x <= 1; x++)
		{
			for (int y = -1; y <= 1; y++)
			{
				f32 pcfDepth = texture(shadowMap, coord.xy + v2(x, y) * texelSize).r;
				result += currentDepth - bias > pcfDepth ? 1.0f : 0.0f;
			}
		}
		result = result / 9.0f;
	}
	return result;
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

	f32 shadowKoef = CalcShadow(f_LightSpacePosition, viewDir, normal);
	shadowKoef = 1.0f - shadowKoef;
	//shadowKoef = 1.0f;

	vec3 specSample;
	if (material.use_spec_map) {
		specSample = texture(material.spec_map, f_UV).xyz;
	} else {
		specSample = material.specular;
	}

	vec3 directional = CalcDirectionalLight(dir_light, normal,
											viewDir, diffSample,
											specSample, shadowKoef);

	vec3 point = vec3(0.0f);
	for (int i = 0; i < POINT_LIGHTS_NUMBER; i++) {
		point += CalcPointLight(pointLights[i], normal, viewDir, diffSample, specSample);
	}

	vec3 sum_point = point;
	color = v4(sum_point + directional, alpha);
	//color = v4(alpha, alpha, alpha, 1.0);

}

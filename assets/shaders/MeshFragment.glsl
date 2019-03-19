#version 330 core
in vec2 TexCoord;
in vec3 norm;
in vec3 pos;
out vec4 color;

#define POINT_LIGHTS_NUMBER  2

struct Material {
	sampler2D diffuse_map;
	sampler2D spec_map;
	float shininess;
	// TODO: Temporary
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

//struct Light {
//	vec3 position;
//	vec3 ambient;
//	vec3 diffuse;
//	vec3 specular;
//	float linear;
//	float quadratic;
//};

struct DirLight {
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Material material;
//uniform Light lights[POINT_LIGHTS_NUMBER];
uniform DirLight dir_light;

uniform vec3 view_pos;

vec3 DirectionalLight(DirLight light, vec3 normal, vec3 view_dir, vec3 diff_sample, vec3 spec_sample) {
	vec3 light_dir = normalize(-light.direction);
	vec3 light_dir_reflected = reflect(-light_dir, normal);	

	float Kd = max(dot(normal, light_dir), 0.0);
	float Ks = pow(max(dot(view_dir, light_dir_reflected ), 0.0), material.shininess);
	
	vec3 ambient = light.ambient * diff_sample ;
	vec3 diffuse = Kd * light.diffuse * diff_sample;
	vec3 specular = Ks * light.specular * spec_sample;
	return ambient + diffuse + specular;
}
//
//vec3 PointLight(Light light, vec3 normal, vec3 view_dir, vec3 diff_sample, vec3 spec_sample) {
//	float distance = length(light.position - pos);
//	float attenuation = 1.0f / (1.0f + 
//								light.linear * distance + 
//								light.quadratic * distance * distance);
//	
//	vec3 light_dir = normalize(light.position - pos);
//	vec3 reflect_dir = reflect(-light_dir, normal);
//	float Kd = max(dot(normal, light_dir), 0.0);
//	float Ks = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
//	vec3 ambient = light.ambient * diff_sample * attenuation;
//	vec3 diffuse = light.diffuse * Kd * diff_sample * attenuation;
//	vec3 specular = Ks * light.specular * spec_sample * attenuation;
//	return ambient + diffuse + specular;
//}
//
void main()
{
	vec3 normal = normalize(norm);
	vec3 view_dir = normalize(view_pos - pos);

	vec4 diff_texel = texture(material.diffuse_map, TexCoord);
	vec3 diff_sample = vec3(1.0, 0.0, 1.0);//diff_texel.xyz;
	//vec3 spec_sample = texture(material.spec_map, TexCoord).xyz;
	//vec3 diff_sample = material.diffuse;
	vec3 spec_sample = material.specular;

	vec3 directional = DirectionalLight(dir_light, normal, view_dir, diff_sample, spec_sample);

	vec3 point = vec3(0.0f);
	//for (int i = 0; i < POINT_LIGHTS_NUMBER; i++) {
	//	point += PointLight(lights[i], normal, view_dir, diff_sample, spec_sample);
	//}

	vec3 sum_point = clamp(point, 0.0f, 1.0f);

	color = vec4(sum_point + directional, diff_texel.a);
	//color = vec4(sum_point + directional, 1.0f);

}

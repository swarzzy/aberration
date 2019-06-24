in v3 f_Position;
in v4 f_LightSpacePosition;
in v2 f_UV;
in v3 f_Normal;

out v4 color;

#define POINT_LIGHTS_NUMBER  4

struct Material
{
	vec3 ambient;
	vec3 diffuse;
};

struct DirLight
{
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Material material;
uniform DirLight dir_light;
uniform v3 u_ViewPos;

vec3 CalcDirectionalLight(DirLight light, vec3 normal,
						  vec3 view_dir,
						  vec3 diff_sample)
{
	vec3 light_dir = normalize(-light.direction);
	vec3 light_dir_reflected = reflect(-light_dir, normal);	

	float Kd = max(dot(normal, light_dir), 0.0);
	
	vec3 ambient = light.ambient * diff_sample;
	vec3 diffuse = Kd * light.diffuse * diff_sample;
	return ambient + diffuse;
}

void main()
{
	vec3 normal = normalize(f_Normal);
	vec3 viewDir = normalize(u_ViewPos - f_Position);

	vec3 diffSample;
	f32 alpha;
	diffSample = material.diffuse;
	alpha = 1.0f;

	vec3 directional = CalcDirectionalLight(dir_light, normal,
											viewDir, diffSample);

	color = v4(directional, alpha);
}

in v3 f_Position;
in v3 f_MeshSpacePos;
in v4 f_LightSpacePosition;
flat in float f_TileId;
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
uniform sampler2DArray u_TerrainAtlas;

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

vec4 FourTapSample(vec2 tileUV, sampler2D atlas)
{
	vec4 color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	float totalWeight = 0.0f;

	for (int dx = 0; dx < 2; dx++)
	{
		for (int dy = 0; dy < 2; dy++)
		{
			vec2 tileCoord = 2.0f * fract(0.5f * (tileUV + vec2(dx, dy)));
			float weight = pow(1.0f - max(abs(tileCoord.x - 1.0f), abs(tileCoord.y - 1.0f)), 16.0f);
			vec2 atlasUV = tileCoord;
			color += weight * texture(atlas, atlasUV);
			totalWeight += weight;
		}
	}
	color /= totalWeight;
	return color;
}

void main()
{
	vec3 normal = normalize(f_Normal);
	vec3 viewDir = normalize(u_ViewPos - f_Position);

	vec3 diffSample;
	f32 alpha;
	vec2 tileUV = vec2(dot(normal.zxy, f_MeshSpacePos), dot(normal.yzx, f_MeshSpacePos));
	diffSample = texture(u_TerrainAtlas, vec3(tileUV.x, tileUV.y, f_TileId)).rgb;
	//diffSample = material.diffuse;
	alpha = 1.0f;
	//diffSample = vec3(f_UV.y, 0.0f, 0.0f);

	vec3 directional = CalcDirectionalLight(dir_light, normal,
											viewDir, diffSample);

	color = v4(directional, alpha);
}

layout (location = 0) in v3 v_Position;
layout (location = 1) in v3 v_Normal;
layout (location = 2) in float v_TileId;

out v3 f_Position;
out v3 f_MeshSpacePos;
out v4 f_LightSpacePosition;
flat out float f_TileId;
out v3 f_Normal;

uniform m4x4 modelMatrix;
uniform m4x4 normalMatrix;
uniform m4x4 viewProjMatrix;
uniform m4x4 lightSpaceMatrix;

#define TERRAIN_TEX_ARRAY_NUM_LAYERS 32

void main()
{
	f_TileId = v_TileId;
	f_MeshSpacePos = v_Position;
	f_Position = (modelMatrix * v4(v_Position, 1.0f)).xyz;
	f_Normal = m3x3(normalMatrix) * v_Normal;
	f_LightSpacePosition = lightSpaceMatrix * modelMatrix * v4(v_Position, 1.0f);
    gl_Position = viewProjMatrix * modelMatrix * v4(v_Position, 1.0f);
}
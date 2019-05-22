layout (location = 0) in v3 v_Position;
layout (location = 1) in v2 v_UV;
layout (location = 2) in v3 v_Normal;

layout (location = 3) in m4x4 i_ModelMatrix;
layout (location = 7) in v3 i_DebugColor;

out v3 f_Position;
out v4 f_LightSpacePosition;
out v2 f_UV;
out v3 f_Normal;
out v3 f_DebugColor;

uniform m4x4 viewProjMatrix;
uniform m4x4 lightSpaceMatrix;

void main()
{
	m4x4 modelMatrix = i_ModelMatrix;//transpose(i_ModelMatrix);
	// NOTE: Calculate normal matrix here for now
	m4x4 normalMatrix = m4x4(transpose(inverse(m3x3(modelMatrix))));
	f_Position = (modelMatrix * v4(v_Position, 1.0f)).xyz;
	f_Normal = m3x3(normalMatrix) * v_Normal;
	f_DebugColor = i_DebugColor;
	// TODO: UV fliping?
    f_UV = v2(v_UV.x, 1.0 - v_UV.y);
	f_LightSpacePosition = lightSpaceMatrix * modelMatrix * v4(v_Position, 1.0f);
    gl_Position = viewProjMatrix * modelMatrix * v4(v_Position, 1.0f);
}
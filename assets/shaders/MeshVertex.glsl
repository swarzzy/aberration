out v3 f_Position;
out v4 f_LightSpacePosition;
out v2 f_UV;
out v3 f_Normal;

uniform m4x4 modelMatrix;
uniform m4x4 normalMatrix;
uniform m4x4 viewProjMatrix;
uniform m4x4 lightSpaceMatrix;

void main()
{
	f_Position = (modelMatrix * v4(v_Position, 1.0f)).xyz;
	f_Normal = m3x3(normalMatrix) * v_Normal;
    f_UV = v2(v_UV.x, 1.0 - v_UV.y);
	f_LightSpacePosition = lightSpaceMatrix * modelMatrix * v4(v_Position, 1.0f);
    gl_Position = viewProjMatrix * modelMatrix * v4(v_Position, 1.0f);
}
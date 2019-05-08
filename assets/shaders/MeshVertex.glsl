out v3 f_Position;
out v2 f_UV;
out v3 f_Normal;

uniform m4x4 modelMatrix;
uniform m4x4 normalMatrix;
uniform m4x4 viewProjMatrix;

void main()
{
	f_Position = (modelMatrix * v4(v_Position, 1.0f)).xyz;
	f_Normal = m3x3(normalMatrix) * v_Normal;
    f_UV = v2(v_UV.x, 1.0 - v_UV.y);
    gl_Position = viewProjMatrix * modelMatrix * v4(v_Position, 1.0f);
}
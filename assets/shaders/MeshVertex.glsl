out v3 f_Position;
out v2 f_UV;
out v3 f_Normal;

void main()
{
	f_Position = (sys_ModelMatrix * v4(v_Position, 1.0f)).xyz;
	f_Normal = m3x3(sys_NormalMatrix) * v_Normal;
    f_UV = v2(v_UV.x, 1.0 - v_UV.y);
    gl_Position = sys_ViewProjMatrix * sys_ModelMatrix * v4(v_Position, 1.0f);
}
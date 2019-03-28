out Vector3 f_Position;
out Vector2 f_UV;
out Vector3 f_Normal;

void main()
{
	f_Position = (sys_ModelMatrix * vec4(v_Position, 1.0f)).xyz;
	f_Normal = mat3(sys_NormalMatrix) * v_Normal;
    f_UV = vec2(v_UV.x, 1.0 - v_UV.y);
    gl_Position = sys_ViewProjMatrix * sys_ModelMatrix * vec4(v_Position, 1.0f);
}
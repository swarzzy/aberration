layout (location = 0) in v3 v_Pos;

out v3 f_Color;

uniform m4x4 u_ViewProjMatrix;
uniform v3 u_Color;

void main()
{
    gl_Position = u_ViewProjMatrix * v4(v_Pos, 1.0f);
	f_Color = u_Color;
}
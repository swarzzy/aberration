layout (location = 0) in v3 v_Position;
layout (location = 1) in v2 v_UV;
layout (location = 2) in v3 v_Normal;

uniform m4x4 modelMatrix;
uniform m4x4 viewProjMatrix;

void main()
{
    gl_Position = viewProjMatrix * modelMatrix * v4(v_Position, 1.0f);
}
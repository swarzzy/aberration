uniform m4x4 modelMatrix;
uniform m4x4 viewProjMatrix;

void main()
{
    gl_Position = viewProjMatrix * modelMatrix * v4(v_Position, 1.0f);
}
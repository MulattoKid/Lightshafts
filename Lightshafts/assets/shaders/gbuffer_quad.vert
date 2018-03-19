#version 430

in layout(location=0) vec3 position;
in layout(location=1) vec3 normal;

out vec3 f_position;

void main()
{
    f_position = position;

    gl_Position = vec4(position, 1.0f);
}
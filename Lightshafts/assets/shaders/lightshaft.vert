#version 430

in layout(location=0) vec3 position;
in layout(location=1) vec2 uv;

out vec2 f_uv;

void main()
{
	f_uv = uv;

    gl_Position = vec4(position, 1.0f);
}
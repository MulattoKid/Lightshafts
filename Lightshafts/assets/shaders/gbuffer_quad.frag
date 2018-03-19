#version 430

in vec3 f_position;

out layout(location=0) vec4 g_color;
out layout(location=1) vec4 g_position;

void main()
{
    g_color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	g_position = vec4(f_position.xy, 999999.0f, 1.0f);
}
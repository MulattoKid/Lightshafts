#version 430

in layout(location=0) vec3 position;

void main()
{
	gl_Position = vec4(position, 1.0f);
}
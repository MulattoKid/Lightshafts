#version 430

in vec2 f_uv;

layout(location=0) uniform sampler2D sampler;

out vec4 color;

void main()
{
    float depth_value = texture(sampler, f_uv).r;
    color = vec4(vec3(depth_value), 1.0f);
}
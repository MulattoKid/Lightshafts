#version 430

in vec2 f_uv;

uniform layout(location=0) sampler2D color_sampler;
uniform layout(location=1) sampler2D scattering_sampler;
layout (std140) uniform UBOData
{
	vec4 viewport;
    mat4 camera_vp;
	mat4 camera_to_world;
    vec4 camera_pos;
    vec4 camera_dir;
    vec4 light_pos_0;
    vec4 light_color_0;
	mat4 light_vp_0;
} ubo_data;

out vec4 color;

void main()
{
	color = texture(color_sampler, f_uv);
	vec3 scattering = vec3(texture(scattering_sampler, f_uv).x) * ubo_data.light_color_0;
	color.xyz += scattering;

	//color = vec4(texture(scattering_sampler, f_uv).x, 0.0f, 0.0f, 1.0f);
}
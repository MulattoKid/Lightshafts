#version 430

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

vec3 GenerateRay()
{
	vec2 NDC = gl_FragCoord.xy / vec2(1920.0f, 1080.0f);
	vec2 SS = vec2(NDC * 2.0f - 1.0f);
	SS.x = SS.x * (1920.0f / 1080.0f) * tan(70.0f / 2);
	SS.y = SS.y * tan(70.0f / 2);
	vec3 point_world_space = vec3(ubo_data.camera_to_world * vec4(SS, -1.0f, 1.0f));

	return normalize(point_world_space - vec3(ubo_data.camera_pos));
}

void main()
{
	color = vec4(GenerateRay(), 1.0f);
}
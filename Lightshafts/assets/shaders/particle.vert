#version 430

in layout(location=0) vec3 position;
in layout(location=1) vec3 normal;
in layout(location=2) vec3 center;

layout (std140) uniform UBOData
{
	vec4 viewport;
    mat4 camera_vp;
	mat4 camera_to_world;
    vec4 camera_pos;
    vec4 camera_dir;
    vec4 light_pos_0;
	vec4 light_dir_0;
	vec4 light_cutoff_0;
    vec4 light_color_0;
	mat4 light_vp_0;
} ubo_data;

out vec4 f_camera_space_position;
out vec4 f_light_space_position;
out vec3 f_normal;

void main()
{
	vec3 particle_position = position + center;

	f_camera_space_position = ubo_data.camera_vp * vec4(particle_position, 1.0f);
	f_light_space_position = ubo_data.light_vp_0 * vec4(particle_position, 1.0f);
	f_normal = normal;

	gl_Position = ubo_data.camera_vp * vec4(particle_position, 1.0f);
}
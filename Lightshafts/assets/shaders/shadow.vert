#version 430

in layout(location=0) vec3 position;

uniform mat4 model_matrix;
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

void main()
{
    gl_Position = ubo_data.light_vp_0 * model_matrix * vec4(position, 1.0f);
}
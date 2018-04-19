#version 430

in vec4 f_camera_space_position;
in vec4 f_light_space_position;
in vec3 f_normal;

uniform layout(location=0) sampler2D shadow_sampler;
uniform layout(location=1) sampler2D position_sampler;
uniform layout(location=2) sampler2D scattering_sampler;
uniform float time_uniform;
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

out vec4 color;

void main()
{	
	//Check if the current fragment is in shadow or not
	vec3 light_space_pos = f_light_space_position.xyz / f_light_space_position.w;
    light_space_pos = light_space_pos * 0.5f + 0.5f; //Range [-1,1] -> [0,1]
    float light_min_depth = texture(shadow_sampler, light_space_pos.xy).r;
    float bias = 0.001;
	float in_shadow = light_space_pos.z - bias > light_min_depth ? 1.0f : 0.0f; //If in shadow then 1, else 0

	//Check that this fragment is closer to the camera than that already in the Gbuffer
	vec3 camera_space_pos = f_camera_space_position.xyz / f_camera_space_position.w;
	camera_space_pos = camera_space_pos * 0.5f + 0.5f; //Range [-1,1] -> [0,1]
	float gbuffer_min_depth = texture(position_sampler, camera_space_pos.xy).r;
	float too_far_away = camera_space_pos.z > gbuffer_min_depth ? 1.0f : 0.0f; //If too far away then 1, else 0

	color = vec4(ubo_data.light_color_0.xyz, 0.3f - max(in_shadow, too_far_away));
	//color = vec4(1.0f, 0.0f, 0.0f, 1.0f - too_far_away);
	//color = vec4(1.0f, 0.0f, 0.0f, 1.0f - in_shadow);
	//color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
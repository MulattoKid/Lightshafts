#version 430

in vec4 f_camera_space_position;
in vec4 f_light_space_position;
in vec3 f_normal;

uniform layout(location=0) sampler2D shadow_sampler;
uniform layout(location=1) sampler2D scattering_sampler;
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
	vec3 particle_color = vec3(1.0f, 1.0f, 1.0f);

	vec3 light_space_pos = f_light_space_position.xyz / f_light_space_position.w;
    light_space_pos = light_space_pos * 0.5f + 0.5f; //Range [-1,1] -> [0,1]
    float light_min_depth = texture(shadow_sampler, light_space_pos.xy).r;
    float bias = 0.001;
	if (light_space_pos.z - bias <= light_min_depth) //Not in shadow
	{
		vec3 camera_space_pos = f_camera_space_position.xyz / f_camera_space_position.w;
		camera_space_pos = camera_space_pos * 0.5f + 0.5f; //Range [-1,1] -> [0,1]
		float scattering = texture(scattering_sampler, camera_space_pos.xy).r;

		color = vec4(particle_color, 1.0f);
	}
	/*else
	{
		color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	}*/
}
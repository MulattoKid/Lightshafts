#version 430

//http://www.alexandre-pestana.com/volumetric-lights/
//https://www.slideshare.net/BenjaminGlatzel/volumetric-lighting-for-many-lights-in-lords-of-the-fallen

//MATH
#define PI 3.14159265359f

//LIGHTING
#define AMBIENT 0.4f
#define EMISSIVE 0.0f
#define DIFFUSE 0.5f
#define SPECULAR 0.1f
#define LIGHT_INTENSITY 1.0f

//SCATTERING
#define G 0.2f
#define G_SQ G * G
//Bayer-matrix: https://en.wikipedia.org/wiki/Ordered_dithering
mat4 dither_pattern = {
	{ 0.0f, 0.5f, 0.125f, 0.625f},
	{ 0.75f, 0.22f, 0.875f, 0.375f},
	{ 0.1875f, 0.6875f, 0.0625f, 0.5625},
	{ 0.9375f, 0.4375f, 0.8125f, 0.3125}
};

in vec2 f_uv;

uniform layout(location=0) sampler2D shadow_sampler;
uniform layout(location=1) sampler2D position_sampler;
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

out vec4 scattering;

//https://www.astro.umd.edu/~jph/HG_note.pdf
float CalculateScattering(float ray_dot_light)
{
	float scattering = 1.0f - G_SQ;
	scattering /= (4.0f * PI) * pow(1.0f + G_SQ - (2 * G * ray_dot_light), 1.5f);
	return scattering;
}

void main()
{
	int num_samples = 30;
	vec3 ray_start = vec3(ubo_data.camera_pos);
	vec3 ray_end = texture(position_sampler, f_uv).xyz;
	vec3 ray = ray_end - ray_start;
	vec3 ray_dir = normalize(ray);
	float step_size = length(ray) / num_samples;
	vec3 step = ray_dir * step_size;

	//Dither start position -> adds noise
	vec2 ssp = f_uv * ubo_data.viewport.zw;
	float dither_value = dither_pattern[int(ssp.x) % 4][int(ssp.y) % 4];
	ray_start += step * dither_value;
	vec3 current_pos = ray_start;

	//Raymarch
	float tmp_scattering = 0.0f;
	for (int i = 0; i < num_samples; i++)
	{
		vec4 current_pos_LS = ubo_data.light_vp_0 * vec4(current_pos, 1.0f);
		current_pos_LS.xyz /= current_pos_LS.w;
		vec3 current_pos_LSP = current_pos_LS.xyz * 0.5f + 0.5f; //Range [-1,1] -> [0,1]
		float shadow_map_depth = texture(shadow_sampler, current_pos_LSP.xy).r;
		if (shadow_map_depth > current_pos_LSP.z)
		{
			tmp_scattering += CalculateScattering(dot(ray_dir, normalize(current_pos_LS.xyz - vec3(ubo_data.light_pos_0))));
		}
		current_pos += step;
	}
	tmp_scattering /= num_samples;

	scattering = vec4(tmp_scattering, 0.0f, 0.0f, 1.0f);
}
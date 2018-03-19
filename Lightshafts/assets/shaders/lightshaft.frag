#version 430

#define AMBIENT 0.4f
#define EMISSIVE 0.0f
#define DIFFUSE 0.5f
#define SPECULAR 0.1f
#define LIGHT_INTENSITY 1.0f

in vec2 f_uv;

uniform layout(location=0) sampler2D shadow_sampler;
uniform layout(location=1) sampler2D color_sampler;
uniform layout(location=2) sampler2D position_sampler;
uniform layout (std140) UBOData
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

float LinearizeDepth(float depth)
{
    float zNear = 0.1;
    float zFar  = 1000.0;
    return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

vec3 GenerateRay()
{
	vec2 NDC = gl_FragCoord.xy / ubo_data.viewport.zw;
	vec2 SS = vec2(NDC * 2.0f - 1.0f);
	SS.x = SS.x * (ubo_data.viewport.z / ubo_data.viewport.w) * tan(70.0f / 2);
	SS.y = SS.y * tan(70.0f / 2);
	vec3 point_world_space = vec3(ubo_data.camera_to_world * vec4(SS, -1.0f, 1.0f));

	return normalize(point_world_space - ubo_data.camera_pos);
}

float InShadow(vec4 frag_pos_light_space)
{
    //vec3 frag_pos_proj = frag_pos_light_space.xyz / frag_pos_light_space.w; //Only needed when using perspective projection
    vec3 frag_pos_proj = frag_pos_light_space.xyz; //Orthographics projection
    frag_pos_proj = frag_pos_proj * 0.5f + 0.5f; //Range [-1,1] -> [0,1]
    float light_min_depth = texture(shadow_sampler, frag_pos_proj.xy).r;
    float bias = 0.005;
    float in_shadow = frag_pos_proj.z - bias > light_min_depth ? 1.0f : 0.0f; //If in shadow, return 1, else 0
    if (frag_pos_proj.z > 1.0)
    {
        in_shadow = 0.0f;
    }
    return in_shadow;
}

vec3 CalculateLightshaft(vec3 ray_pos, vec3 ray_dir, float ray_distance, int num_samples)
{
	vec3 current_pos = ray_pos;
	float step_size = ray_distance / num_samples;
	float contribution = 0.0f;

	for (int i = 0; i < num_samples; i++)
	{
		current_pos += step_size * ray_dir;
		vec4 current_pos_light_space_0 = ubo_data.light_vp_0 * vec4(current_pos, 1.0f); //No divide-by-w because orthographics projection
		float in_shadow = InShadow(current_pos_light_space_0);
		//if (in_shadow == 0.0f) return vec3(0.5f);
		contribution += in_shadow;
	}

	return vec3(contribution / num_samples) * ubo_data.light_color_0;
}

void main()
{
	/*vec3 ray_pos = vec3(ubo_data.camera_pos);
	vec3 ray_dir = GenerateRay();
	float ray_distance = 40.0f; //MAX
	vec3 lightshaft = CalculateLightshaft(ray_pos, ray_dir, ray_distance, 200);*/

	//color = vec4(texture(position_sampler, f_uv).xyz, 1.0f);
	color = texture(position_sampler, f_uv);
	//color = texture(color_sampler, f_uv);
}
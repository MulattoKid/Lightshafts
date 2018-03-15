#version 430

#define AMBIENT 0.2f
#define EMISSIVE 0.0f
#define DIFFUSE 0.6f
#define SPECULAR 0.2f
#define LIGHT_INTENSITY 1.0f

in vec3 f_position;
in vec3 f_normal;
in vec4 f_position_light_space_0;

layout(location=0) uniform sampler2D shadow_sampler;

layout (std140) uniform UBOData
{
    mat4 camera_vp;
    vec4 camera_pos;
    vec4 camera_dir;
    vec4 light_pos_0;
    vec4 light_color_0;
    mat4 light_vp_0;
} ubo_data;

out vec4 color;

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

vec3 CalculateLighting(vec3 camera_pos, vec3 light_pos, vec3 light_color)
{
    vec4 albedo = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    vec3 light_dir = normalize(light_pos - f_position);
    vec3 reflect_dir = normalize(reflect(-light_dir, f_normal));
    vec3 view_dir = normalize(camera_pos - f_position);

    //Ambient
    vec3 ambient_intensity = vec3(AMBIENT);

    //Emissive
    vec3 emissive_intensity = vec3(1.0f, 1.0f, 1.0f) * EMISSIVE;

    //Diffuse
    float diffuse = dot(light_dir, f_normal);
    vec3 diffuse_intensity = vec3(diffuse) * DIFFUSE;

    //Specular
    float specular = dot(view_dir, reflect_dir);
    vec3 specular_intensity = vec3(specular) * SPECULAR;

    float in_shadow_light_0 = InShadow(f_position_light_space_0);
    return vec3(albedo) * (ambient_intensity + emissive_intensity + (1.0f - in_shadow_light_0) * (diffuse_intensity + specular_intensity * vec3(ubo_data.light_color_0)));
}

void main()
{
    vec3 lighting_0 = CalculateLighting(vec3(ubo_data.camera_pos), vec3(ubo_data.light_pos_0), vec3(ubo_data.light_color_0));
    color = vec4(lighting_0, 1.0f);
}
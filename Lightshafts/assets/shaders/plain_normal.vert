#version 430

in layout(location=0) vec3 position;
in layout(location=1) vec3 normal;

layout (std140) uniform UBOData
{
    mat4 camera_vp;
    vec4 camera_pos;
    vec4 camera_dir;
    vec4 light_pos_0;
    vec4 light_color_0;
    mat4 light_vp_0;
} ubo_data;

out vec3 f_position;
out vec3 f_normal;
out vec4 f_position_light_space_0;

void main()
{
    f_position = position;
    f_normal = normal;
    f_position_light_space_0 = ubo_data.light_vp_0 * vec4(position, 1.0f);

    gl_Position = ubo_data.camera_vp * vec4(position, 1.0f);
}
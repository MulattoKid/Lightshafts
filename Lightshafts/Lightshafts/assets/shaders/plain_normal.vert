#version 430

in layout(location=0) vec3 position;
in layout(location=1) vec3 normal;

layout (std140) uniform UBOData
{
    mat4 vp;
    //mat4 light_vp;
    //vec4 camera_position;
    //vec4 light_position;
} ubo_data;

void main()
{
    gl_Position = ubo_data.vp * vec4(position, 1.0f);
}
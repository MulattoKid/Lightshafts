#pragma once

struct UBOData
{
	float viewport[4];
	float camera_vp[16];
	float camera_to_world[16];
	float camera_pos[4];
	float camera_dir[4];
	float light_pos_0[4];
	float light_dir_0[4];
	float light_cutoff_0[4];
	float light_color_0[4];
	float light_vp_0[16];
};

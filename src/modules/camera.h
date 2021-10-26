/**
 * @ingroup     module_camera
 * @author      LACROIX - Impulse
 * @copyright   Copyright (c) 2021 LACROIX - Impulse. All rights reserved.
 * @brief       Camera module (default one)
 *
 * @{
 */
#ifndef _CAMERA_H
#define _CAMERA_H

#include <stdint.h>

/* Used by main to communicate with parse_opt. */
struct camera_arguments {
    int camera_id;
    int streamer_id;
    int display;
    int record;
    int record_duration;
    int reg;
    uint32_t reg_address;
    int val;
    int reg_value;
    int read;
    int write;
    int reboot;
    int fps_value;
    int play;
    char *path_frames_dir;
    int blending;
    char *path_blend_frame;
    int stop_blending;
    int blender_id;
    int boot_mode;
    int heartbeat;
    int cropping;
    char *cropping_coord;
    int monitoring_info;
    int exposure;
    int gain;
    int x_offset;
    int y_offset;
    int cmd_pattern; /* Pattern command activated */
    uint8_t pattern; /* Selected pattern  */
};

/* Possible patterns */
struct camera_pattern_mode {
    uint8_t tp;
    const char *name;
};

/**
 * @brief Parse the parameters and execute the  function
 * @param[in] argc arguments count
 * @param[in] argv arguments
 * @return
 */
int camera_parse(int argc, char **argv);

#endif /* _CAMERA_H */
/** @} */

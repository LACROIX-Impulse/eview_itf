/**
 * \file main.c
 * \brief Main program entry for communication between A53 and R7 CPUs
 * \author esoftthings
 *
 * Main program entry to communicate with the R7 CPU from the A53 (Linux).
 *
 */

#include <stdlib.h>
#include <argp.h>
#include "eviewitf.h"

#define DEFAULT_FPS   30

const char *argp_program_version = "eviewitf-" VERSION;
const char *argp_program_bug_address = "<support-ecube@esoftthings.com>";

/* Program documentation */
static char doc[] = "eviewitf -- Program for communication between A53 and R7 CPUs";

/* Arguments description */
static char args_doc[] =
    "to record: -c [0-7] -r [???] delay(s) \n"
    "to change display: -d -c [0-7] \n "
    "to write register: -c [0-7] -Wa [0x????] -v [0x??] \n"
    "to read register: -c [0-7] -Wa [0x????] \n"
    "to reboot a camera: -s -c [0-7]"
    "to change camera fps: -f [0-60] -c [0-7] \n";

/* Program options */
static struct argp_option options[] = {
    {"camera", 'c', "ID", 0, "Select camera on which command occurs"},
    {"display", 'd', 0, 0, "Select camera as display"},
    {"record", 'r', "DURATION", 0, "Record camera ID stream on SSD for DURATION (in seconds)"},
    /* {"type", 't', "TYPE", 0, "Select camera type"}, not used for now */
    {"address", 'a', "ADDRESS", 0, "Register ADDRESS on which read or write"},
    {"value", 'v', "VALUE", 0, "VALUE to write in the register"},
    {"read", 'R', 0, 0, "Read register"},
    {"write", 'W', 0, 0, "Write register"},
    {"reboot", 's', 0, 0, "Software reboot camera"},
    {"fps", 'f', "FPS", 0, "Set camera FPS"},
    {"virtupdate", 'u', "PATH", 0, "Virtual camera update"},
    {0},
};

/* Used by main to communicate with parse_opt. */
struct arguments {
    int camera;
    int camera_id;
    int display;
    int record;
    int record_duration;
    int type;
    int camera_type;
    int reg;
    int reg_address;
    int val;
    int reg_value;
    int read;
    int write;
    int reboot;
    int set_fps;
    int fps_value;
    int virt_update;
    char* path_frames_dir;
};

/* Parse a single option. */
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    /* Get the input argument from argp_parse */
    struct arguments *arguments = state->input;

    switch (key) {
        case 'c':
            arguments->camera = 1;
            arguments->camera_id = atoi(arg);
            if ((arguments->camera_id < 0) || (arguments->camera_id >= EVIEWITF_MAX_CAMERA)) {
                argp_usage(state);
            }
            break;
        case 'd':
            arguments->display = 1;
            break;
        case 'r':
            arguments->record = 1;
            arguments->record_duration = atoi(arg);
            if (arguments->record_duration < 0) {
                argp_usage(state);
            }
            break;
            /*    case 't':
                    arguments->type = 1;
                    arguments->camera_type = atoi(arg);
                    break;*/
        case 'a':
            arguments->reg = 1;
            arguments->reg_address = (int)strtol(arg, NULL, 16);
            break;
        case 'v':
            arguments->val = 1;
            arguments->reg_value = (int)strtol(arg, NULL, 16);
            break;
        case 'R':
            arguments->read = 1;
            break;
        case 'W':
            arguments->write = 1;
            break;
        case 's':
            arguments->reboot = 1;
            break;
        case 'f':
            arguments->set_fps = 1;
            arguments->fps_value = atoi(arg);
            break;
        case 'u':
            arguments->virt_update = 1;
            arguments->path_frames_dir = arg;
            break;
        case ARGP_KEY_ARG:
            if (state->arg_num >= 0) {
                /* Too many arguments. */
                argp_usage(state);
            }
            break;
        case ARGP_KEY_END:
            if (state->arg_num < 0) {
                /* Not enough arguments. */
                argp_usage(state);
            }
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

/* argp parser. */
static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char **argv) {
    struct arguments arguments;
    int ret;
    uint16_t register_value = 0;
    /* Default values. */
    arguments.camera = 0;
    arguments.camera_id = 0;
    arguments.display = 0;
    arguments.record = 0;
    arguments.record_duration = 0;
    arguments.type = 1;        /* not used for now default value*/
    arguments.camera_type = 0; /* not used for now default value*/
    arguments.reg = 0;
    arguments.reg_address = 0;
    arguments.val = 0;
    arguments.reg_value = 0;
    arguments.read = 0;
    arguments.write = 0;
    arguments.reboot = 0;
    arguments.set_fps = 0;
    arguments.virt_update = 0;
    arguments.fps_value = 0;
    arguments.path_frames_dir = NULL;

    /* Parse arguments; every option seen by parse_opt will
       be reflected in arguments. */
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    /* Select camera for display */
    if (arguments.camera && arguments.display) {
        if (eviewitf_set_display_cam(arguments.camera_id) >= 0) {
            fprintf(stdout, "Camera %d selected for display\n", arguments.camera_id);
        }
    }
    /* Select camera for record */
    if (arguments.camera && arguments.record) {
        eviewitf_init_api();
        if (eviewitf_record_cam(arguments.camera_id, arguments.record_duration) >= 0) {
            fprintf(stdout, "Recorded %d s from camera %d\n", arguments.record_duration, arguments.camera_id);
        } else {
            fprintf(stdout, "Fail to record stream from camera %d\n", arguments.camera_id);
        }
        eviewitf_deinit_api();
    }

    /* Set camera register value */
    if (arguments.camera && arguments.type && arguments.reg && arguments.val && arguments.write) {
        ret = eviewitf_set_camera_param(arguments.camera_id, arguments.camera_type, arguments.reg_address,
                                        arguments.reg_value);
        if (ret >= EVIEWITF_OK) {
            fprintf(stdout, "0X%hhX written in register 0X%X of camera id %d \n", arguments.reg_value,
                    arguments.reg_address, arguments.camera_id);
        } else if (ret == EVIEWITF_BLOCKED) {
            fprintf(stdout, "You are not allowed to modify register 0X%X \n", arguments.reg_address);
        } else {
            fprintf(stdout, "Fail to set value 0X%hhX, of camera id %d in register 0X%X\n", arguments.reg_value,
                    arguments.camera_id, arguments.reg_address);
        }
    }
    /* Get camera register value*/
    if (arguments.camera && arguments.type && arguments.reg && arguments.read) {
        ret = eviewitf_get_camera_param(arguments.camera_id, arguments.camera_type, arguments.reg_address,
                                        &register_value);
        if (ret >= EVIEWITF_OK) {
            fprintf(stdout, "Register 0X%X Value: 0X%hhX, of camera id %d \n", arguments.reg_address, register_value,
                    arguments.camera_id);
        } else if (ret == EVIEWITF_BLOCKED) {
            fprintf(stdout, "You are not allowed to read register 0X%X \n", arguments.reg_address);
        } else {
            fprintf(stdout, "Fail to get register 0X%X value, of camera id %d  \n", arguments.reg_address,
                    arguments.camera_id);
        }
    }
    /* reboot a camera */
    if (arguments.camera && arguments.reboot) {
        ret = eviewitf_reboot_cam(arguments.camera_id);

        if (ret >= EVIEWITF_OK) {
            fprintf(stdout, "Camera %d rebooted \n", arguments.camera_id);
        } else if (ret == EVIEWITF_INVALID_PARAM) {
            fprintf(stdout, "You send a wrong camera Id");
        } else {
            fprintf(stdout, "Fail to reboot camera %d  \n", arguments.camera_id);
        }
    }

    /* change camera fps */
    if (arguments.camera && arguments.set_fps && !arguments.virt_update) {
        if (arguments.set_fps < 0) {
            fprintf(stdout, "Camera %d negative values not allowed \n", arguments.camera_id);
        } else {
            ret = eviewitf_set_camera_fps(arguments.camera_id, (uint32_t)arguments.fps_value);

            if (ret >= EVIEWITF_OK) {
                fprintf(stdout, "Camera %d new fps %d \n", arguments.camera_id, arguments.fps_value);
            } else if (ret == EVIEWITF_INVALID_PARAM) {
                fprintf(stdout, "You send a wrong camera Id or a wrong FPS value");
            } else {
                fprintf(stdout, "Fail to set camera %d fps: %d \n", arguments.camera_id, arguments.fps_value);
            }
        }
    }

    /* Update VCam */
    if (arguments.camera && arguments.virt_update) {
        eviewitf_init_api();

        printf("Main path %s\n", arguments.path_frames_dir);

        if (arguments.set_fps) {
            ret = eviewitf_virtual_cam_update(arguments.camera_id, arguments.fps_value, arguments.path_frames_dir);
        }
        else {
            ret = eviewitf_virtual_cam_update(arguments.camera_id, DEFAULT_FPS, arguments.path_frames_dir);
        }
        if (ret >= EVIEWITF_OK) {
            fprintf(stdout, "Camera %d update frame \n", arguments.camera_id);
        } else if (ret == EVIEWITF_INVALID_PARAM) {
            fprintf(stdout, "You sent a wrong camera Id");
        } else {
            fprintf(stdout, "Fail\n");
        }
        eviewitf_deinit_api();
    }

    exit(0);
}

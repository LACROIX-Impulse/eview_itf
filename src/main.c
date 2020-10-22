/**
 * \file main.c
 * \brief Main program entry for communication between A53 and R7 CPUs
 * \author esoftthings
 *
 * Main program entry to communicate with the R7 CPU from the A53 (Linux).
 *
 */

#include <argp.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "eviewitf_priv.h"

#define FPS_MIN_VALUE     5
#define FPS_DEFAULT_VALUE 30
#define FPS_MAX_VALUE     60

const char *argp_program_version = "eviewitf-" VERSION;
const char *argp_program_bug_address = "<support-ecube@esoftthings.com>";

/* Program documentation */
static char doc[] = "eviewitf -- Program for communication between A53 and R7 CPUs";

/* Arguments description */
static char args_doc[] =
    "change display:  -d -c [0-7]\n"
    "change display:  -d -s [0-7]\n"
    "record:          -c [0-7] -r [???] (-p [PATH])\n"
    "play recordings: -s [0-7] -f [5-60] -p [PATH]\n"
    "write register:  -c [0-7] -Wa [0x????] -v [0x??]\n"
    "read register:   -c [0-7] -Ra [0x????]\n"
    "reboot a camera: -x -c [0-7]\n"
    "set blending:    -b [PATH] -o [0-1]\n"
    "stop blending:   -n\n"
    "set R7 heartbeat state: -H [0-1]\n"
    "set R7 boot mode: -B [0-?]\n"
    "start cropping -U x1:y1:x2:y2\n"
    "stop cropping -u"
    "raw monitoring info -m";

/* Program options */
static struct argp_option options[] = {
    {"camera", 'c', "ID", 0, "Select camera on which command occurs", 0},
    {"streamer", 's', "ID", 0, "Select streamer on which command occurs", 0},
    {"display", 'd', 0, 0, "Select camera as display", 0},
    {"record", 'r', "DURATION", 0, "Record camera ID stream on SSD for DURATION (s)", 0},
    {"address", 'a', "ADDRESS", 0, "Register ADDRESS on which read or write", 0},
    {"value", 'v', "VALUE", 0, "VALUE to write in the register", 0},
    {"read", 'R', 0, 0, "Read register", 0},
    {"write", 'W', 0, 0, "Write register", 0},
    {"reboot", 'x', 0, 0, "Software reboot camera", 0},
    {"fps", 'f', "FPS", 0, "Set playback FPS", 0},
    {"play", 'p', "PATH", 0, "Play a stream in <PATH> as a virtual camera", 0},
    {"blending", 'b', "PATH", 0, "Set the blending frame <PATH> over the display", 0},
    {"no-blending", 'n', 0, 0, "Stop the blending", 0},
    {"heartbeat", 'H', "STATE", 0, "Set R7 heartbeat state", 0},
    {"boot", 'B', "MODE", 0, "Select R7 boot mode", 0},
    {"blending interface", 'o', "BLENDING", 0, "Select blending interface on which command occurs", 0},
    {"cropping start", 'U', "COORDINATES", 0, "Start the cropping according to coordinates", 0},
    {"cropping stop", 'u', 0, 0, "Stop the cropping according", 0},
    {"raw monitoring info", 'm', 0, 0, "Get monitoring info in RAW format", 0},
    {0},
};

/* Used by main to communicate with parse_opt. */
struct arguments {
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
};

/* Parse a single option. */
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    /* Get the input argument from argp_parse */
    struct arguments *arguments = state->input;

    switch (key) {
        case 'a':
            arguments->reg = 1;
            arguments->reg_address = (int)strtol(arg, NULL, 16);
            break;
        case 'B':
            arguments->boot_mode = atoi(arg);
            if (arguments->boot_mode < 0) {
                argp_usage(state);
            }
            break;
        case 'b':
            arguments->blending = 1;
            arguments->path_blend_frame = arg;
            break;
        case 'c':
            arguments->camera_id = atoi(arg);
            if ((arguments->camera_id < 0) || (arguments->camera_id >= EVIEWITF_MAX_CAMERA)) {
                argp_usage(state);
            }
            break;
        case 'd':
            arguments->display = 1;
            break;
        case 'f':
            arguments->fps_value = atoi(arg);
            if ((arguments->fps_value < FPS_MIN_VALUE) || (arguments->fps_value > FPS_MAX_VALUE)) {
                argp_usage(state);
            }
            break;
        case 'H':
            arguments->heartbeat = atoi(arg);
            if (arguments->heartbeat < 0) {
                argp_usage(state);
            }
            break;
        case 'm':
            arguments->monitoring_info = 1;
            break;
        case 'n':
            arguments->stop_blending = 1;
            break;
        case 'o':
            arguments->blender_id = atoi(arg);
            if ((arguments->blender_id < 0) || (arguments->blender_id >= EVIEWITF_MAX_BLENDER)) {
                argp_usage(state);
            }
            break;
        case 'p':
            arguments->play = 1;
            arguments->path_frames_dir = arg;
            break;
        case 's':
            arguments->streamer_id = atoi(arg);
            if ((arguments->streamer_id < 0) || (arguments->streamer_id >= EVIEWITF_MAX_STREAMER)) {
                argp_usage(state);
            }
            break;
        case 'R':
            arguments->read = 1;
            break;
        case 'r':
            arguments->record_duration = atoi(arg);
            if (arguments->record_duration < 0) {
                argp_usage(state);
            }
            break;
        case 'U':
            arguments->cropping = 1;
            arguments->cropping_coord = arg;
            break;
        case 'u':
            arguments->cropping = 0;
            break;
        case 'v':
            arguments->val = 1;
            arguments->reg_value = (int)strtol(arg, NULL, 16);
            break;
        case 'W':
            arguments->write = 1;
            break;
        case 'x':
            arguments->reboot = 1;
            break;
        case ARGP_KEY_ARG:
            argp_usage(state);
            break;
        case ARGP_KEY_END:
            if (state->argc <= 1) {
                /* Not enough args */
                argp_state_help(state, state->out_stream, ARGP_HELP_USAGE | ARGP_HELP_LONG);
            }
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

/* argp parser. */
static struct argp argp = {options, parse_opt, args_doc, doc, NULL, NULL, NULL};

int main(int argc, char **argv) {
    struct arguments arguments;
    int ret = EVIEWITF_OK;
    uint32_t register_value = 0;
    /* cropping deparse variables */
    char *cropping_args;
    uint32_t cropp_x1, cropp_y1, cropp_x2, cropp_y2;
    /* Default values. */
    arguments.camera_id = -1;
    arguments.streamer_id = -1;
    arguments.display = 0;
    arguments.record_duration = -1;
    arguments.reg = 0;
    arguments.reg_address = 0;
    arguments.val = 0;
    arguments.reg_value = 0;
    arguments.read = 0;
    arguments.write = 0;
    arguments.reboot = 0;
    arguments.play = 0;
    arguments.fps_value = -1;
    arguments.path_frames_dir = NULL;
    arguments.blender_id = -1;
    arguments.path_blend_frame = NULL;
    arguments.stop_blending = 0;
    arguments.boot_mode = -1;
    arguments.heartbeat = -1;
    arguments.cropping = -1;
    arguments.cropping_coord = NULL;
    arguments.monitoring_info = 0;

    /* Parse arguments; every option seen by parse_opt will
       be reflected in arguments. */
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    /* Select camera for display */
    if ((arguments.camera_id >= 0) && arguments.display) {
        eviewitf_init();
        ret = eviewitf_display_select_camera(arguments.camera_id);
        if (ret >= 0) {
            fprintf(stdout, "Camera %d selected for display\n", arguments.camera_id);
        } else {
            fprintf(stdout, "Failed to select camera %d for display\n", arguments.camera_id);
        }
        eviewitf_deinit();
    }
    /* Select streamer for display */
    if ((arguments.streamer_id >= 0) && arguments.display) {
        eviewitf_init();
        ret = eviewitf_display_select_streamer(arguments.streamer_id);
        if (ret >= 0) {
            fprintf(stdout, "Streamer %d selected for display\n", arguments.streamer_id);
        } else {
            fprintf(stdout, "Failed to select streamer %d for display\n", arguments.streamer_id);
        }
        eviewitf_deinit();
    }
    /* Select camera for record */
    if ((arguments.camera_id >= 0) && (arguments.record_duration > 0)) {
        eviewitf_init();
        ret = eviewitf_app_record_cam(arguments.camera_id, arguments.record_duration, arguments.path_frames_dir);
        if (ret >= 0) {
            fprintf(stdout, "Recorded %d s from camera %d\n", arguments.record_duration, arguments.camera_id);
        } else {
            fprintf(stdout, "Fail to record stream from camera %d\n", arguments.camera_id);
        }
        eviewitf_deinit();
    }

    /* Set camera register value */
    if ((arguments.camera_id >= 0) && arguments.reg && arguments.val && arguments.write) {
        ret = eviewitf_camera_set_parameter(arguments.camera_id, arguments.reg_address, arguments.reg_value);
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
    if ((arguments.camera_id >= 0) && arguments.reg && arguments.read) {
        ret = eviewitf_camera_get_parameter(arguments.camera_id, arguments.reg_address, &register_value);
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
    if ((arguments.camera_id >= 0) && arguments.reboot) {
        ret = eviewitf_app_reset_camera(arguments.camera_id);

        if (ret >= EVIEWITF_OK) {
            fprintf(stdout, "Camera %d rebooted \n", arguments.camera_id);
        } else if (ret == EVIEWITF_INVALID_PARAM) {
            fprintf(stdout, "You send a wrong camera Id\n");
        } else {
            fprintf(stdout, "Fail to reboot camera %d  \n", arguments.camera_id);
        }
    }

    /* Playback on streamer */
    if ((arguments.streamer_id >= 0) && arguments.play) {
        eviewitf_init();
        if (arguments.fps_value > 0) {
            ret = eviewitf_app_streamer_play(arguments.streamer_id, arguments.fps_value, arguments.path_frames_dir);
        } else {
            ret = eviewitf_app_streamer_play(arguments.streamer_id, FPS_DEFAULT_VALUE, arguments.path_frames_dir);
        }
        if (ret >= EVIEWITF_OK) {
            fprintf(stdout, "Recording played on camera %d\n", arguments.streamer_id);
        } else if (ret == EVIEWITF_INVALID_PARAM) {
            fprintf(stdout, "You sent a wrong parameter\n");
        } else {
            fprintf(stdout, "Fail\n");
        }
        eviewitf_deinit();
    }

    /* Set a blending frame */
    if (arguments.blender_id >= 0) {
        eviewitf_init();
        ret = eviewitf_display_select_blender(arguments.blender_id);
        if (ret >= EVIEWITF_OK) {
            ret = eviewitf_app_set_blending_from_file(arguments.blender_id, arguments.path_blend_frame);
            if (ret >= EVIEWITF_OK) {
                fprintf(stdout, "Blending applied\n");
            } else if (ret == EVIEWITF_INVALID_PARAM) {
                fprintf(stdout, "You sent a wrong parameter\n");
            } else {
                fprintf(stdout, "Fail to set blending\n");
            }
        } else if (ret == EVIEWITF_INVALID_PARAM) {
            fprintf(stdout, "You sent a wrong parameter to Start blending\n");
        } else {
            fprintf(stdout, "Start blending Fail\n");
        }

        eviewitf_deinit();
    }

    /* Stop the blending */
    if (arguments.stop_blending) {
        ret = eviewitf_display_select_blender(-1);
        if (ret >= EVIEWITF_OK) {
            fprintf(stdout, "Blending stopped\n");
        } else if (ret == EVIEWITF_INVALID_PARAM) {
            fprintf(stdout, "An error occurred\n");
        } else {
            fprintf(stdout, "Fail\n");
        }
    }

    /* Print monitoring info */
    if (arguments.monitoring_info) {
        ret = eviewitf_app_print_monitoring_info();
        if (ret >= EVIEWITF_OK) {
            fprintf(stdout, "\n");
        } else if (ret == EVIEWITF_INVALID_PARAM) {
            fprintf(stdout, "An error occurred\n");
        } else {
            fprintf(stdout, "Fail\n");
        }
    }

    /* R7 heartbeat */
    if (arguments.heartbeat >= 0) {
        ret = eviewitf_set_R7_heartbeat_mode((uint32_t)arguments.heartbeat);
        if (ret >= EVIEWITF_OK) {
            fprintf(stdout, "R7 heartbeat mode changed\n");
        } else if (ret == EVIEWITF_INVALID_PARAM) {
            fprintf(stdout, "Set R7 heartbeat mode error\n");
        } else {
            fprintf(stdout, "Set R7 heartbeat mode failure\n");
        }
    }

    /* R7 boot mode */
    if (arguments.boot_mode >= 0) {
        ret = eviewitf_set_R7_heartbeat_mode((uint32_t)arguments.boot_mode);
        if (ret >= EVIEWITF_OK) {
            fprintf(stdout, "R7 boot mode changed\n");
        } else if (ret == EVIEWITF_INVALID_PARAM) {
            fprintf(stdout, "Set R7 boot mode error\n");
        } else {
            fprintf(stdout, "Set R7 boot mode failure\n");
        }
    }

    /* start cropping */
    if (arguments.cropping == 1) {
        cropping_args = strtok(arguments.cropping_coord, ":");
        if ((ret >= EVIEWITF_OK) && (cropping_args != NULL)) {
            cropp_x1 = (uint32_t)atoi(cropping_args);
        } else {
            fprintf(stdout, "Start cropping, seems you forget to set x1 parameter, aborting \n");
            ret = EVIEWITF_INVALID_PARAM;
        }
        if (ret >= EVIEWITF_OK) {
            cropping_args = strtok(NULL, ":");
            if (cropping_args != NULL) {
                cropp_y1 = (uint32_t)atoi(cropping_args);
            } else {
                fprintf(stdout, "Start cropping, seems you forget to set y1 parameter, aborting \n");
                ret = EVIEWITF_INVALID_PARAM;
            }
        }

        if (ret >= EVIEWITF_OK) {
            cropping_args = strtok(NULL, ":");
            if (cropping_args != NULL) {
                cropp_x2 = (uint32_t)atoi(cropping_args);
            } else {
                fprintf(stdout, "Start cropping, seems you forget to set x2 parameter, aborting \n");
                ret = EVIEWITF_INVALID_PARAM;
            }
        }

        if (ret >= EVIEWITF_OK) {
            cropping_args = strtok(NULL, ":");
            if (cropping_args != NULL) {
                cropp_y2 = (uint32_t)atoi(cropping_args);
            } else {
                fprintf(stdout, "Start cropping, seems you forget to set y2 parameter, aborting \n");
                ret = EVIEWITF_INVALID_PARAM;
            }
        }
        if (ret >= EVIEWITF_OK) {
            ret = eviewitf_display_select_cropping(cropp_x1, cropp_y1, cropp_x2, cropp_y2);
            if (ret >= EVIEWITF_OK) {
                fprintf(stdout, "Cropping set\n");
            } else if (ret == EVIEWITF_INVALID_PARAM) {
                fprintf(stdout, "Cropping set error\n");
            } else {
                fprintf(stdout, "Cropping set failure\n");
            }
        }
    }

    /* stop cropping  */
    if (arguments.cropping == 0) {
        ret = eviewitf_display_select_cropping(0, 0, 0, 0);
        if (ret >= EVIEWITF_OK) {
            fprintf(stdout, "Cropping stopped\n");
        } else if (ret == EVIEWITF_INVALID_PARAM) {
            fprintf(stdout, "Cropping stopped error\n");
        } else {
            fprintf(stdout, "Cropping stopped failure\n");
        }
    }
    exit(-ret);
}

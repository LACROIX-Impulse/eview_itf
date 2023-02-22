/**
 * \file
 * \brief Module camera
 * \author LACROIX Impulse
 *
 * The module Camera handles operations that relate to streams and cameras
 *
 */
#include "camera.h"

#include <argp.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "eviewitf.h"
#include "eviewitf-priv.h"

/* Used by main to communicate with parse_opt. */
typedef struct camera_arguments {
    int camera_id;
    int record;
    int record_duration;
    int reg;
    uint32_t reg_address;
    int val;
    int reg_value;
    int read;
    int write;
    int start;
    int stop;
    int reboot;
    int fps_value;
    int heartbeat;
    int monitoring_info;
    int exposure;
    int gain;
    int x_offset;
    int y_offset;
    int cmd_pattern; /* Pattern command activated */
    uint8_t pattern; /* Selected pattern  */
} camera_arguments_t;

/* Possible patterns */
typedef struct camera_pattern_mode {
    uint8_t tp;
    const char *name;
} camera_pattern_mode_t;

/* Program documentation */
static char camera_doc[] =
    "eviewitf -- Program for communication between A53 and R7 CPUs"
    "\v"
    "Available camera patterns:\n"
    " none, solid-red, solid-green, solid-blue, solid-vbar, solid-vbar-faded,\n"
    " custom0, custom1, custom2, custom3, custom4\n";

/* Arguments description */
static char camera_args_doc[] =
    "module:          [camera(default)|pipeline|video]\n"
    "record:          -c[0-7] -r[???] (-p[PATH])\n"
    "play recordings: -s[0-7] -f[2-60] -p[PATH]\n"
    "write register:  -c[0-7] -Wa[0x????] -v[0x??]\n"
    "read register:   -c[0-7] -Ra[0x????]\n"
    "start a camera:  -c[0-7] -s\n"
    "stop a camera:   -c[0-7] -S\n"
    "reboot a camera: -c[0-7] -x\n"
    "monitoring info: -m\n"
    "set exposure:    -c[0-7] -e[???] -g[???]\n"
    "get exposure:    -c[0-7] -E\n"
    "set offset:      -c[0-7] -jx:[X] -jy:[Y]\n"
    "get offset:      -c[0-7] -J\n"
    "set pattern:     -c[0-7] -t[pattern]\n"
    "get pattern:     -c[0-7] -T\n"
    "set frame rate:  -c[0-7] -f[2-60]\n"
    "get frame rate:  -c[0-7] -F";

/* Program options */
static argp_option_t camera_options[] = {
    {"camera", 'c', "ID", 0, "Select camera on which command occurs", 0},
    {"record", 'r', "DURATION", 0, "Record camera ID stream on SSD for DURATION (s)", 0},
    {"address", 'a', "ADDRESS", 0, "Register ADDRESS on which read or write", 0},
    {"value", 'v', "VALUE", 0, "VALUE to write in the register", 0},
    {"read", 'R', 0, 0, "Read register", 0},
    {"write", 'W', 0, 0, "Write register", 0},
    {"start", 's', 0, 0, "Software start camera", 0},
    {"stop", 'S', 0, 0, "Software stop camera", 0},
    {"reboot", 'x', 0, 0, "Software reboot camera", 0},
    {"fps", 'f', "FPS", 0, "Set frame rate", 0},
    {"fps", 'F', 0, 0, "Get frame rate", 0},
    {"raw monitoring info", 'm', 0, 0, "Get monitoring info in RAW format", 0},
    {"exposure", 'E', 0, 0, "Get camera exposure value", 0},
    {"exposure", 'e', "EXPOSURE", 0, "Set camera exposure delay", 0},
    {"gain", 'g', "GAIN", 0, "Set camera gain", 0},
    {"offset", 'j', "OFFSET", 0, "Set camera frame offset", 0},
    {"offset", 'J', 0, 0, "Get camera frame offset", 0},
    {"pattern", 't', "PATTERN", 0, "Set camera test pattern", 0},
    {"pattern", 'T', 0, 0, "Get camera test pattern", 0},
    {0},
};

/* clang-format off */
static camera_pattern_mode_t patterns[] = {
        { EVIEWITF_TEST_PATTERN_UNKNOWN, "unknown" },                    /* Unknown pattern */
        { EVIEWITF_TEST_PATTERN_NONE, "none", },                         /* No test pattern */
        { EVIEWITF_TEST_PATTERN_SOLID_RED, "solid-red", },               /* Solid color - red */
        { EVIEWITF_TEST_PATTERN_SOLID_GREEN, "solid-green", },           /* Solid color - green */
        { EVIEWITF_TEST_PATTERN_SOLID_BLUE, "solid-blue", },             /* Solid color - blue */
        { EVIEWITF_TEST_PATTERN_SOLID_VBAR, "solid-vbar", },             /* Vertical bars */
        { EVIEWITF_TEST_PATTERN_SOLID_VBAR_FADED, "solid-vbar-faded", }, /* Vertical bars faded */
        { EVIEWITF_TEST_PATTERN_CUSTOM0, "custom0", },                   /* Custom pattern 0 */
        { EVIEWITF_TEST_PATTERN_CUSTOM1, "custom1", },                   /* Custom pattern 1 */
        { EVIEWITF_TEST_PATTERN_CUSTOM2, "custom2", },                   /* Custom pattern 2 */
        { EVIEWITF_TEST_PATTERN_CUSTOM3, "custom3", },                   /* Custom pattern 3 */
        { EVIEWITF_TEST_PATTERN_CUSTOM4, "custom4", },                   /* Custom pattern 4 */
        { 0, NULL },
};
/* clang-format on */

/* Gets the pattern value related to the given string */
static int str2pattern(const char *pattern) {
    if (!pattern) return -1;
    for (int n = 0; patterns[n].name; n++) {
        if (strcmp(pattern, patterns[n].name) == 0) return patterns[n].tp;
    }
    return -1;
}

/* Gets the pattern value related to the given string */
static const char *pattern2str(uint8_t tp) {
    for (int n = 0; patterns[n].name; n++) {
        if (tp == patterns[n].tp) return patterns[n].name;
    }
    return patterns[0].name;
}

/* Parse a single option. */
static error_t camera_parse_opt(int key, char *arg, argp_state_t *state) {
    /* Get the input argument from argp_parse */
    camera_arguments_t *arguments = state->input;

    switch (key) {
        case 'a':
            arguments->reg = 1;
            arguments->reg_address = (int)strtol(arg, NULL, 16);
            break;
        case 'c':
            arguments->camera_id = atoi(arg);
            if ((arguments->camera_id < 0) || (arguments->camera_id >= EVIEWITF_MAX_CAMERA)) {
                argp_usage(state);
            }
            break;
        case 'E':
            arguments->exposure = -2;
            break;
        case 'e':
            arguments->exposure = atoi(arg);
            if (arguments->exposure < 0) {
                argp_usage(state);
            }
            break;
        case 'J':
            arguments->x_offset = -2;
            arguments->y_offset = -2;
            break;
        case 'j': {
            char *v = strchr(arg, ':');
            if (v == NULL) {
                argp_usage(state);
                break;
            }
            *v = '\0';
            v++;
            if (strlen(arg) != 1) {
                argp_usage(state);
                break;
            }
            if (*arg == 'x') arguments->x_offset = atoi(v);
            if (*arg == 'y') arguments->y_offset = atoi(v);
            break;
        }
        case 'f':
            arguments->fps_value = atoi(arg);
            if ((arguments->fps_value < FPS_MIN_VALUE) || (arguments->fps_value > FPS_MAX_VALUE)) {
                argp_usage(state);
            }
            break;
        case 'F':
            arguments->fps_value = -2;
            break;
        case 'g':
            arguments->gain = atoi(arg);
            if (arguments->gain < 0) {
                argp_usage(state);
            }
            break;
        case 'm':
            arguments->monitoring_info = 1;
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
        case 't': {
            int pattern = str2pattern(arg);
            if (pattern < 0) {
                fprintf(stdout, "Invalid test pattern %s\n", arg);
                exit(EINVAL);
            }
            arguments->cmd_pattern = 1;
            arguments->pattern = (uint8_t)pattern;
            break;
        }
        case 'T':
            arguments->cmd_pattern = 0;
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
        case 's':
            arguments->start = 1;
            break;
        case 'S':
            arguments->stop = 1;
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
static argp_t camera_argp = {camera_options, camera_parse_opt, camera_args_doc, camera_doc, NULL, NULL, NULL};

/**
 * @brief Parse the parameters and execute the  function
 * @param[in] argc arguments count
 * @param[in] argv arguments
 * @return
 */
eviewitf_ret_t camera_parse(int argc, char **argv) {
    eviewitf_ret_t ret = EVIEWITF_OK;
    camera_arguments_t arguments;
    uint32_t register_value = 0;

    /* Default values. */
    arguments.camera_id = -1;
    arguments.record_duration = -1;
    arguments.reg = 0;
    arguments.reg_address = 0;
    arguments.val = 0;
    arguments.reg_value = 0;
    arguments.read = 0;
    arguments.write = 0;
    arguments.start = 0;
    arguments.stop = 0;
    arguments.reboot = 0;
    arguments.fps_value = -1;
    arguments.monitoring_info = 0;
    arguments.exposure = -1;
    arguments.gain = -1;
    arguments.x_offset = -1;
    arguments.y_offset = -1;
    arguments.cmd_pattern = -1;

    /* Parse arguments; every option seen by parse_opt will
          be reflected in arguments. */
    argp_parse(&camera_argp, argc, argv, 0, 0, &arguments);

    /* Set camera register value */
    if ((arguments.camera_id >= 0) && arguments.reg && arguments.val && arguments.write) {
        ret = eviewitf_camera_set_parameter(arguments.camera_id, arguments.reg_address, arguments.reg_value);
        if (ret >= EVIEWITF_OK) {
            fprintf(stdout, "0x%X written in register 0x%X of camera id %d \n", arguments.reg_value,
                    arguments.reg_address, arguments.camera_id);
        } else if (ret == EVIEWITF_BLOCKED) {
            fprintf(stdout, "You are not allowed to modify register 0x%X \n", arguments.reg_address);
        } else {
            fprintf(stdout, "Fail to set value 0x%X, of camera id %d in register 0x%X\n", arguments.reg_value,
                    arguments.camera_id, arguments.reg_address);
        }
    }
    /* Get camera register value*/
    if ((arguments.camera_id >= 0) && arguments.reg && arguments.read) {
        ret = eviewitf_camera_get_parameter(arguments.camera_id, arguments.reg_address, &register_value);
        if (ret >= EVIEWITF_OK) {
            fprintf(stdout, "Register 0x%X Value: 0x%X, of camera id %d \n", arguments.reg_address, register_value,
                    arguments.camera_id);
        } else if (ret == EVIEWITF_BLOCKED) {
            fprintf(stdout, "You are not allowed to read register 0X%X \n", arguments.reg_address);
        } else {
            fprintf(stdout, "Fail to get register 0x%X value, of camera id %d  \n", arguments.reg_address,
                    arguments.camera_id);
        }
    }

    /* start a camera */
    if ((arguments.camera_id >= 0) && (arguments.start)) {
        ret = eviewitf_camera_start(arguments.camera_id);

        if (ret >= EVIEWITF_OK) {
            fprintf(stdout, "Camera %d started \n", arguments.camera_id);
        } else {
            fprintf(stdout, "Fail to start camera %d  \n", arguments.camera_id);
        }
    }

    /* stop a camera */
    if ((arguments.camera_id >= 0) && (arguments.stop)) {
        ret = eviewitf_camera_stop(arguments.camera_id);

        if (ret >= EVIEWITF_OK) {
            fprintf(stdout, "Camera %d stopped \n", arguments.camera_id);
        } else {
            fprintf(stdout, "Fail to stop camera %d  \n", arguments.camera_id);
        }
    }

    /* reboot a camera */
    if ((arguments.camera_id >= 0) && arguments.reboot) {
        ret = eviewitf_app_reset_camera(arguments.camera_id);

        if (ret >= EVIEWITF_OK) {
            fprintf(stdout, "Camera %d rebooted \n", arguments.camera_id);
        } else {
            fprintf(stdout, "Fail to reboot camera %d  \n", arguments.camera_id);
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

    /* Set camera exposure*/
    if ((arguments.camera_id >= 0) && (arguments.exposure >= 0) && (arguments.gain >= 0)) {
        ret = eviewitf_camera_set_exposure(arguments.camera_id, arguments.exposure, arguments.gain);
        if (ret >= EVIEWITF_OK) {
            fprintf(stdout, "Exposure set to %d us and gain to %d on camera id %d \n", arguments.exposure,
                    arguments.gain, arguments.camera_id);
        } else if (ret == EVIEWITF_BLOCKED) {
            fprintf(stdout, "Not possible to set exposure\n");
        } else {
            fprintf(stdout, "Fail to set exposure on camera id %d  \n", arguments.camera_id);
        }
    }
    /* Get camera exposure*/
    if ((arguments.camera_id >= 0) && (arguments.exposure == -2)) {
        uint32_t exposure;
        uint32_t gain;
        ret = eviewitf_camera_get_exposure(arguments.camera_id, &exposure, &gain);
        if (ret >= EVIEWITF_OK) {
            fprintf(stdout, "Exposure is %d us and gain %d on camera id %d \n", exposure, gain, arguments.camera_id);
        } else if (ret == EVIEWITF_BLOCKED) {
            fprintf(stdout, "Not possible to get exposure\n");
        } else {
            fprintf(stdout, "Fail to get exposure on camera id %d  \n", arguments.camera_id);
        }
        ret = eviewitf_camera_get_min_exposure(arguments.camera_id, &exposure, &gain);
        if (ret >= EVIEWITF_OK) {
            fprintf(stdout, "Min exposure is %d us and min gain %d on camera id %d \n", exposure, gain,
                    arguments.camera_id);
        } else if (ret == EVIEWITF_BLOCKED) {
            fprintf(stdout, "Not possible to get min exposure\n");
        } else {
            fprintf(stdout, "Fail to get min exposure on camera id %d  \n", arguments.camera_id);
        }
        ret = eviewitf_camera_get_max_exposure(arguments.camera_id, &exposure, &gain);
        if (ret >= EVIEWITF_OK) {
            fprintf(stdout, "Max exposure is %d us and max gain %d on camera id %d \n", exposure, gain,
                    arguments.camera_id);
        } else if (ret == EVIEWITF_BLOCKED) {
            fprintf(stdout, "Not possible to get max exposure\n");
        } else {
            fprintf(stdout, "Fail to get max exposure on camera id %d  \n", arguments.camera_id);
        }
    }

    /* Set camera frame rate */
    if ((arguments.camera_id >= 0) && (arguments.fps_value >= FPS_MIN_VALUE)) {
        ret = eviewitf_camera_set_frame_rate(arguments.camera_id, arguments.fps_value);
        if (ret >= EVIEWITF_OK) {
            fprintf(stdout, "Camera frame rate set to %d fps on camera id %d\n", arguments.fps_value,
                    arguments.camera_id);
        } else if (ret == EVIEWITF_BLOCKED) {
            fprintf(stdout, "Not possible to set camera frame rate on camera id %d\n", arguments.camera_id);
        } else {
            fprintf(stdout, "Fail to set camera frame rate on camera id %d\n", arguments.camera_id);
        }
    }
    /* Get camera frame rate */
    if ((arguments.camera_id >= 0) && (arguments.fps_value == -2)) {
        uint16_t fps;
        ret = eviewitf_camera_get_frame_rate(arguments.camera_id, &fps);
        if (ret >= EVIEWITF_OK) {
            fprintf(stdout, "Camera frame rate is %d fps on camera id %d\n", fps, arguments.camera_id);
        } else if (ret == EVIEWITF_BLOCKED) {
            fprintf(stdout, "Not possible to get camera frame rate on camera id %d\n", arguments.camera_id);
        } else {
            fprintf(stdout, "Fail to get camera frame rate on camera id %d\n", arguments.camera_id);
        }
    }

    /* Set camera offset */
    if ((arguments.camera_id >= 0) && (arguments.x_offset >= 0) && (arguments.y_offset >= 0)) {
        ret = eviewitf_camera_set_frame_offset(arguments.camera_id, arguments.x_offset, arguments.y_offset);
        if (ret >= EVIEWITF_OK) {
            fprintf(stdout, "Offset set to (%d,%d) camera id %d \n", arguments.x_offset, arguments.y_offset,
                    arguments.camera_id);
        } else if (ret == EVIEWITF_BLOCKED) {
            fprintf(stdout, "Not possible to set offset\n");
        } else {
            fprintf(stdout, "Fail to set offset on camera id %d  \n", arguments.camera_id);
        }
    }

    /* Get camera offset */
    if ((arguments.camera_id >= 0) && (arguments.x_offset == -2)) {
        uint32_t x_offset, y_offset;

        ret = eviewitf_camera_get_frame_offset(arguments.camera_id, &x_offset, &y_offset);
        if (ret >= EVIEWITF_OK) {
            fprintf(stdout, "Offset is (%u, %u) on camera id %d \n", x_offset, y_offset, arguments.camera_id);
        } else if (ret == EVIEWITF_BLOCKED) {
            fprintf(stdout, "Not possible to get offset\n");
        } else {
            fprintf(stdout, "Fail to get offset on camera id %d  \n", arguments.camera_id);
        }
    }

    /* Set camera test pattern */
    if ((arguments.camera_id >= 0) && (arguments.cmd_pattern == 1)) {
        ret = eviewitf_camera_set_test_pattern(arguments.camera_id, arguments.pattern);
        if (ret >= EVIEWITF_OK) {
            fprintf(stdout, "Test pattern set to %s on camera id %d \n", pattern2str(arguments.pattern),
                    arguments.camera_id);
        } else if (ret == EVIEWITF_BLOCKED) {
            fprintf(stdout, "Not possible to set test pattern\n");
        } else {
            fprintf(stdout, "Fail to set test pattern %s on camera id %d\n", pattern2str(arguments.pattern),
                    arguments.camera_id);
        }
    }

    /* Get camera test pattern */
    if ((arguments.camera_id >= 0) && (arguments.cmd_pattern == 0)) {
        ret = eviewitf_camera_get_test_pattern(arguments.camera_id, &arguments.pattern);
        if (ret >= EVIEWITF_OK) {
            fprintf(stdout, "Test pattern set to %s on camera id %d\n", pattern2str(arguments.pattern),
                    arguments.camera_id);
        } else if (ret == EVIEWITF_BLOCKED) {
            fprintf(stdout, "Not possible to get test pattern\n");
        } else {
            fprintf(stdout, "Fail to get test pattern on camera id %d\n", arguments.camera_id);
        }
    }
    return ret;
}

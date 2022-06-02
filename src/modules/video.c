/**
 * \file
 * \brief Module video
 * \author LACROIX Impulse
 *
 * The module Video handles operations that relate to video display
 *
 */
#include "video.h"

#include <argp.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "eviewitf.h"
#include "eviewitf-priv.h"
#include "eviewitf/eviewitf-video.h"

enum video_action {
    VIDEO_ACTION_NC = 0,
    VIDEO_ACTION_RESUME,
    VIDEO_ACTION_SUSPEND,
};

/* Used by main to communicate with parse_opt. */
struct video_arguments {
    int camera_id;
    enum video_action action;
};


/* Program documentation */
static char video_doc[] =
    "eviewitf -- Program for communication between A53 and R7 CPUs"
    "\n";

/* Arguments description */
static char video_args_doc[] =
    "module:          [camera(default)|pipeline|video]\n"
    "suspend:         -c[0-7] -s\n"
    "resume:          -c[0-7] -r\n";

/* Program options */
static struct argp_option video_options[] = {
    {"camera", 'c', "ID", 0, "Select camera on which command occurs", 0},
    {"suspend", 's', 0, 0, "Suspend video display", 0},
    {"resume", 'r', 0, 0, "Resume video display", 0},
    {0},
};


/* Parse a single option. */
static error_t video_parse_opt(int key, char *arg, struct argp_state *state) {
    /* Get the input argument from argp_parse */
    struct video_arguments *arguments = state->input;

    switch (key) {
        case 'c':
            arguments->camera_id = atoi(arg);
            if ((arguments->camera_id < 0) || (arguments->camera_id >= EVIEWITF_MAX_CAMERA)) {
                argp_usage(state);
            }
            break;
        case 's':
            arguments->action = VIDEO_ACTION_SUSPEND;
            break;
        case 'r':
            arguments->action = VIDEO_ACTION_RESUME;
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
static struct argp video_argp = {video_options, video_parse_opt, video_args_doc, video_doc, NULL, NULL, NULL};

/**
 * @brief Parse the parameters and execute the  function
 * @param[in] argc arguments count
 * @param[in] argv arguments
 * @return
 */
int video_parse(int argc, char **argv) {
    int ret = EVIEWITF_OK;
    struct video_arguments arguments;

    /* Default values. */
    arguments.camera_id = -1;
    arguments.action = VIDEO_ACTION_NC;

    /* Parse arguments; every option seen by parse_opt will
          be reflected in arguments. */
    argp_parse(&video_argp, argc, argv, 0, 0, &arguments);

    /* Resume video for a camera */
    if ((arguments.camera_id >= 0) && (arguments.action == VIDEO_ACTION_RESUME)) {

        ret = eviewitf_video_resume(arguments.camera_id);

        if (ret >= EVIEWITF_OK) {
            fprintf(stdout, "Video for camera %d resumed \n", arguments.camera_id);
        } else {
            fprintf(stdout, "Fail to resume video for camera %d  \n", arguments.camera_id);
        }
    }

    /* Suspend video for a camera */
    if ((arguments.camera_id >= 0) && (arguments.action == VIDEO_ACTION_SUSPEND)) {

        ret = eviewitf_video_suspend(arguments.camera_id);

        if (ret >= EVIEWITF_OK) {
            fprintf(stdout, "Video for camera %d suspended \n", arguments.camera_id);
        } else {
            fprintf(stdout, "Fail to suspend video for camera %d  \n", arguments.camera_id);
        }
    }

    return ret;
}

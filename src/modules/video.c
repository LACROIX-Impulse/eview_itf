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

/**
 * \enum video_action
 * \brief Implemented action for video module
 */
typedef enum video_action {
    VIDEO_ACTION_NC = 0,
    VIDEO_ACTION_RESUME,
    VIDEO_ACTION_SUSPEND,
    VIDEO_ACTION_STATE,
} video_action_t;

/* Used by main to communicate with parse_opt. */
typedef struct video_arguments {
    int camera_id;
    video_action_t action;
} video_arguments_t;

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
static argp_option_t video_options[] = {
    {"camera", 'c', "ID", 0, "Select camera on which command occurs", 0},
    {"suspend", 's', 0, 0, "Suspend video display", 0},
    {"resume", 'r', 0, 0, "Resume video display", 0},
    {"state", 'S', 0, 0, "Gets the video state", 0},
    {0},
};

/* Parse a single option. */
static error_t video_parse_opt(int key, char *arg, argp_state_t *state) {
    /* Get the input argument from argp_parse */
    video_arguments_t *arguments = state->input;

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
        case 'S':
            arguments->action = VIDEO_ACTION_STATE;
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
static argp_t video_argp = {video_options, video_parse_opt, video_args_doc, video_doc, NULL, NULL, NULL};

/**
 * @brief Parse the parameters and execute the  function
 * @param[in] argc arguments count
 * @param[in] argv arguments
 * @return
 */
int video_parse(int argc, char **argv) {
    int ret = EVIEWITF_OK;
    video_arguments_t arguments;

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

    /* Gets the video state for a camera */
    if ((arguments.camera_id >= 0) && (arguments.action == VIDEO_ACTION_STATE)) {
        char sstate[16] = "unknown";
        uint32_t state;

        ret = eviewitf_video_get_state(arguments.camera_id, &state);
        if (ret >= EVIEWITF_OK) {
            if (state == EVIEWITF_VIDEO_STATE_RUNNING) snprintf(sstate, 16, "running");
            if (state == EVIEWITF_VIDEO_STATE_SUSPENDED) snprintf(sstate, 16, "suspended");
            fprintf(stdout, "Video for camera %d is %s \n", arguments.camera_id, sstate);
        } else {
            fprintf(stdout, "Fail to get the video state for camera %d  \n", arguments.camera_id);
        }
    }

    return ret;
}

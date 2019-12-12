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

const char *argp_program_version = "eviewitf-" VERSION;
const char *argp_program_bug_address = "<support-ecube@esoftthings.com>";

/* Program documentation */
static char doc[] = "eviewitf -- Program for communication between A53 and R7 CPUs";

/* Arguments description */
static char args_doc[] = "-c [0-7] -r [0-7] delay(s)";

/* Program options */
static struct argp_option options[] = {
    {"camera", 'c', "ID", 0, "Select camera on which command occurs"},
    {"display", 'd', 0, 0, "Select camera as display"},
    {"record", 'r', "DURATION", 0, "Record camera ID stream on SSD for DURATION (in seconds)"},
    {0},
};

/* Used by main to communicate with parse_opt. */
struct arguments {
    int camera;
    int camera_id;
    int display;
    int record;
    int record_duration;
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

    /* Default values. */
    arguments.camera = 0;
    arguments.camera_id = 0;
    arguments.display = 0;
    arguments.record = 0;
    arguments.record_duration = 0;

    /* Parse arguments; every option seen by parse_opt will
       be reflected in arguments. */
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    /* Select camera for display */
    if (arguments.camera && arguments.display) {
        if (eviewitf_set_display_cam(arguments.camera_id) >= 0) {
            fprintf(stdout, "Camera %d selected for display\n", arguments.camera_id);
        }
    }
    /* Select camera for display */
    if (arguments.camera && arguments.record) {
        eviewitf_init_api();
        if (eviewitf_record_cam(arguments.camera_id, arguments.record_duration) >= 0) {
            fprintf(stdout, "Recorded %d s from camera %d\n", arguments.record_duration, arguments.camera_id);
        } else {
            fprintf(stdout, "Fail to record stream from camera %d\n", arguments.camera_id);
        }
        eviewitf_deinit_api();
    }

    exit(0);
}
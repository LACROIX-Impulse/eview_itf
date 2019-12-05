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
static char args_doc[] = "-c [1-8]";

/* Program options */
static struct argp_option options[] = {
    {"camera", 'c', "ID", 0,  "Select camera ID for display"},
    { 0 }
};

/* Used by main to communicate with parse_opt. */
struct arguments
{
    int camera;
    int camera_id;
};

/* Parse a single option. */
static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
    /* Get the input argument from argp_parse */
    struct arguments *arguments = state->input;

    switch (key)
    {
        case 'c':
            arguments->camera = 1;
            arguments->camera_id = atoi (arg);
            if ((arguments->camera_id < 1) || (arguments->camera_id > EVIEWITF_MAX_CAMERA)) {
                argp_usage (state);
            }
            break;
        case ARGP_KEY_ARG:
            if (state->arg_num >= 0) {
                /* Too many arguments. */
                argp_usage (state);
             }
              break;
        case ARGP_KEY_END:
            if (state->arg_num < 0) {
                /* Not enough arguments. */
                argp_usage (state);
            }
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

/* argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };

int main (int argc, char **argv)
{
    struct arguments arguments;

    /* Default values. */
    arguments.camera = 0;
    arguments.camera_id = 0;

    /* Parse arguments; every option seen by parse_opt will
       be reflected in arguments. */
    argp_parse (&argp, argc, argv, 0, 0, &arguments);

    /* Select camera for display */
    if (arguments.camera) {
        if (eviewitf_set_display_cam(arguments.camera_id) > 0) {
            fprintf(stdout, "Camera %d selected for display\n", arguments.camera_id);
        }
    }

    exit (0);
}
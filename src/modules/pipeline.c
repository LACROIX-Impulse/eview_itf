/**
 * \file pipeline.c
 * \brief Module pipeline
 * \author LACROIX Impulse
 *
 * The module Pipeline handles operations that relate to pipeline
 *
 */
#include "pipeline.h"

#include <argp.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "eviewitf.h"
#include "eviewitf_priv.h"

/* Used by main to communicate with parse_opt. */
struct pipeline_arguments {
    int pipeline_id;
    int state;
    int configure;
    int start;
    int stop;
    uint32_t width;
    uint32_t height;
};

/* Arguments description */
static char pipeline_args_doc[] =
    "configure:      -p[0-1] -c -w[0-4096] -h[0-4096]\n"
    "start:          -p[0-1] -s\n"
    "stop:           -p[0-1] -S";

/* Program options */
static struct argp_option pipeline_options[] = {
    {"pipeline", 'p', "ID", 0, "Select pipeline on which command occurs", 0},
    {"configure", 'c', 0, 0, "Configure the pipeline", 0},
    {"width", 'w', "VALUE", 0, "Set frame width", 0},
    {"height", 'h', "VALUE", 0, "Set frame height", 0},
    {"start", 's', 0, 0, "Start the pipeline", 0},
    {"stop", 'S', 0, 0, "Stop the pipeline", 0},
    {0},
};

/* Parse a single option. */
static error_t pipeline_parse_opt(int key, char *arg, struct argp_state *state) {
    /* Get the input argument from argp_parse */
    struct pipeline_arguments *arguments = state->input;

    switch (key) {
        case 'c':
            arguments->configure = 1;
            break;
        case 'w':
            arguments->width = atoi(arg);
            if (arguments->width > 4096) {
                argp_usage(state);
            }
            break;
        case 'h':
            arguments->height = atoi(arg);
            if (arguments->height > 4096) {
                argp_usage(state);
            }
            break;
        case 's':
            arguments->start = 1;
            break;
        case 'S':
            arguments->stop = 1;
            break;
        case 'p':
            arguments->pipeline_id = atoi(arg);
            if ((arguments->pipeline_id < 0) || (arguments->pipeline_id >= EVIEWITF_MAX_PIPELINE)) {
                argp_usage(state);
            }
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

/* argp parser. */
static struct argp pipeline_argp = {pipeline_options, pipeline_parse_opt, pipeline_args_doc, NULL, NULL, NULL, NULL};

/**
 * @brief Parse the parameters and execute the  function
 * @param[in] argc arguments count
 * @param[in] argv arguments
 * @return
 */
int pipeline_parse(int argc, char **argv) {
    int ret = EVIEWITF_OK;
    struct pipeline_arguments arguments;

    /* Default values. */
    arguments.pipeline_id = -1;
    arguments.start = 0;
    arguments.stop = 0;
    arguments.configure = 0;
    arguments.width = 0;
    arguments.height = 0;

    /* Parse arguments; every option seen by parse_opt will
          be reflected in arguments. */
    argp_parse(&pipeline_argp, argc, argv, 0, 0, &arguments);

    /* Starts the pipeline */
    if (arguments.pipeline_id >= 0 && arguments.start) {
        eviewitf_init();
        ret = eviewitf_pipeline_start(arguments.pipeline_id);
        if (ret >= 0) {
            fprintf(stdout, "Pipeline %d started\n", arguments.pipeline_id);
        } else {
            fprintf(stdout, "Failed to start pipeline %d\n", arguments.pipeline_id);
        }
        eviewitf_deinit();
    }
    /* Stop the pipeline */
    if (arguments.pipeline_id >= 0 && arguments.stop) {
        eviewitf_init();
        ret = eviewitf_pipeline_stop(arguments.pipeline_id);
        if (ret >= 0) {
            fprintf(stdout, "Pipeline %d stoped\n", arguments.pipeline_id);
        } else {
            fprintf(stdout, "Failed to stop pipeline %d\n", arguments.pipeline_id);
        }
        eviewitf_deinit();
    }
    /* Configure the pipeline */
    if (arguments.pipeline_id >= 0 && arguments.configure) {
        eviewitf_init();
        ret = eviewitf_pipeline_configure(arguments.pipeline_id, arguments.width, arguments.height);
        if (ret >= 0) {
            fprintf(stdout, "Pipeline %d configured\n", arguments.pipeline_id);
        } else {
            fprintf(stdout, "Failed to configure pipeline %d\n", arguments.pipeline_id);
        }
        eviewitf_deinit();
    }
    return ret;
}

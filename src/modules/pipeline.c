/**
 * \file
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
#include "eviewitf-priv.h"

/* Used by main to communicate with parse_opt. */
typedef struct pipeline_arguments {
    int pipeline_id;
    int state;
    int configure;
    int reboot;
    int start;
    int stop;
    uint32_t width;
    uint32_t height;
    int led;
    uint8_t led_id;
    uint8_t led_level;
} pipeline_arguments_t;

/* Arguments description */
static char pipeline_args_doc[] =
    "module pipeline: pipeline\n"
    "configure:      -p[0-255] -c -w[0-4096] -h[0-4096]\n"
    "start:          -p[0-255] -s\n"
    "stop:           -p[0-255] -S\n"
    "reboot:         -p[0-255] -R\n"
    "set led:        -p[0-255] -L -i[0-2] -l[0-1]";

/* Program options */
static argp_option_t pipeline_options[] = {
    {"pipeline", 'p', "ID", 0, "Select pipeline on which command occurs", 0},
    {"configure", 'c', 0, 0, "Configure the pipeline", 0},
    {"width", 'w', "VALUE", 0, "Set frame width", 0},
    {"height", 'h', "VALUE", 0, "Set frame height", 0},
    {"start", 's', 0, 0, "Start the pipeline", 0},
    {"reboot", 'R', 0, 0, "Reboot the pipeline R7/A53", 0},
    {"stop", 'S', 0, 0, "Stop the pipeline", 0},
    {"led", 'L', 0, 0, "Set led command", 0},
    {"id", 'i', "VALUE", 0, "Set led identifier", 0},
    {"level", 'l', "VALUE", 0, "Set led level", 0},
    {0},
};

/* Parse a single option. */
static error_t pipeline_parse_opt(int key, char *arg, argp_state_t *state) {
    /* Get the input argument from argp_parse */
    pipeline_arguments_t *arguments = state->input;

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
        case 'i':
            arguments->led_id = atoi(arg);
            if (arguments->led_id > 2) {
                argp_usage(state);
            }
            break;
        case 'l':
            arguments->led_level = atoi(arg);
            if (arguments->led_level > 1) {
                argp_usage(state);
            }
            break;
        case 'L':
            arguments->led = 1;
            break;
        case 's':
            arguments->start = 1;
            break;
        case 'S':
            arguments->stop = 1;
            break;
        case 'R':
            arguments->reboot = 1;
            break;
        case 'p':
            arguments->pipeline_id = atoi(arg);
            if (arguments->pipeline_id < 0 || arguments->pipeline_id > 0xFF) {
                argp_usage(state);
            }
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

/* argp parser. */
static argp_t pipeline_argp = {pipeline_options, pipeline_parse_opt, pipeline_args_doc, NULL, NULL, NULL, NULL};

/**
 * @brief Parse the parameters and execute the  function
 * @param[in] argc arguments count
 * @param[in] argv arguments
 * @return EVIEWITF_OK on success, negative value on failure (see eviewitf_ret_t enum)
 */
int pipeline_parse(int argc, char **argv) {
    int ret = EVIEWITF_OK;
    pipeline_arguments_t arguments;

    /* Default values. */
    arguments.pipeline_id = -1;
    arguments.start = 0;
    arguments.stop = 0;
    arguments.configure = 0;
    arguments.led = 0;
    arguments.led_id = 0;
    arguments.led_level = 0;
    arguments.width = 0;
    arguments.height = 0;

    /* Parse arguments; every option seen by parse_opt will
          be reflected in arguments. */
    argp_parse(&pipeline_argp, argc, argv, 0, 0, &arguments);

    /* Starts the pipeline */
    if (arguments.pipeline_id != -1 && arguments.start) {
        eviewitf_init();
        ret = eviewitf_pipeline_start((uint8_t)arguments.pipeline_id);
        if (ret >= 0) {
            fprintf(stdout, "Pipeline %d started\n", arguments.pipeline_id);
        } else {
            fprintf(stdout, "Failed to start pipeline %d\n", arguments.pipeline_id);
        }
        eviewitf_deinit();
    }
    /* Stop the pipeline */
    if (arguments.pipeline_id != -1 && arguments.stop) {
        eviewitf_init();
        ret = eviewitf_pipeline_stop((uint8_t)arguments.pipeline_id);
        if (ret >= 0) {
            fprintf(stdout, "Pipeline %d stopped\n", arguments.pipeline_id);
        } else {
            fprintf(stdout, "Failed to stop pipeline %d\n", arguments.pipeline_id);
        }
        eviewitf_deinit();
    }
    /* Reboot the pipeline R7/A53 */
    if (arguments.pipeline_id != -1 && arguments.reboot) {
        eviewitf_init();
        ret = eviewitf_pipeline_reboot((uint8_t)arguments.pipeline_id);
        if (ret >= 0) {
            fprintf(stdout, "Pipeline %d rebooted\n", arguments.pipeline_id);
        } else {
            fprintf(stdout, "Failed to reboot pipeline %d\n", arguments.pipeline_id);
        }
        eviewitf_deinit();
    }
    /* Set led level */
    if (arguments.pipeline_id != -1 && arguments.led) {
        eviewitf_init();
        ret = eviewitf_pipeline_set_led((uint8_t)arguments.pipeline_id, arguments.led_id, arguments.led_level);
        if (ret >= 0) {
            fprintf(stdout, "Pipeline %d led (%d) set to %d level\n", arguments.pipeline_id, arguments.led_id,
                    arguments.led_level);
        } else {
            fprintf(stdout, "Failed to set led (%d) to %d level (pipeline %d)\n", arguments.led_id, arguments.led_level,
                    arguments.pipeline_id);
        }
        eviewitf_deinit();
    }
    /* Configure the pipeline */
    if (arguments.pipeline_id != -1 && arguments.configure) {
        eviewitf_init();
        ret = eviewitf_pipeline_configure((uint8_t)arguments.pipeline_id, arguments.width, arguments.height);
        if (ret >= 0) {
            fprintf(stdout, "Pipeline %d configured\n", arguments.pipeline_id);
        } else {
            fprintf(stdout, "Failed to configure pipeline %d\n", arguments.pipeline_id);
        }
        eviewitf_deinit();
    }
    return ret;
}

/**
 * \file main.c
 * \brief Main program entry for communication between A53 and R7 CPUs
 * \author LACROIX Impulse
 *
 * Main program entry to communicate with the R7 CPU from the A53 (Linux).
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include "eviewitf/eviewitf-structs.h"
#include "camera.h"
#include "pipeline.h"
#include "legacy.h"
#include "video.h"
#include <string.h>

const char *argp_program_version = "eviewitf-" VERSION;
const char *argp_program_bug_address = "<support-ecube@lacroix.group>";

int main(int argc, char **argv) {
    eviewitf_ret_t ret = EVIEWITF_OK;

    // if no argv
    if (argc == 1) {
        ret = camera_parse(argc, argv);
        goto out;
    }

    /*  Here : test every modules, and goto out after parse*/
    if (!strcmp("pipeline", argv[1])) {
        argc--;
        argv++;
        ret = pipeline_parse(argc, argv);
        goto out;
    }

    else if (!strcmp("camera", argv[1])) {
        argc--;
        argv++;
        ret = camera_parse(argc, argv);
        goto out;
    } else if (!strcmp("video", argv[1])) {
        argc--;
        argv++;
        ret = video_parse(argc, argv);
        goto out;
    }

    ret = legacy_parse(argc, argv);

out:
    exit(-ret);
}

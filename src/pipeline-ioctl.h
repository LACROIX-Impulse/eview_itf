#ifndef _PIPELINE_IOCTL_H
#define _PIPELINE_IOCTL_H

#include "mfis-ioctl.h"

/**
 * @brief Pipeline geometry
 */
struct pipeline_geometry {
    uint32_t width;
    uint32_t height;
};

/**
 * @brief Pipeline led
 */
struct pipeline_led {
    uint8_t id;
    uint8_t level;
};

/**
 * @brief Pipeline I/O operations
 */
#define IOCSPIPELINECONFIGURE MFIS_IOW(0, sizeof(struct pipeline_geometry)) /* Configures the pipeline */
#define IOCPIPELINESTART      MFIS_IO(1)                                    /* Starts the pipeline */
#define IOCPIPELINESTOP       MFIS_IO(2)                                    /* Stops the pipeline */
#define IOCPIPELINEREBOOT     MFIS_IO(3)                                    /* Reboots the board R7/A53 */
#define IOCSPIPELINESETLED    MFIS_IOW(4, sizeof(struct pipeline_led))      /* Sets the pipeline led */

#endif /* _PIPELINE_IOCTL_H */

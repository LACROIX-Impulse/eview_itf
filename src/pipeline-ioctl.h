#ifndef _PIPEINE_IOCTL_H
#define _PIPEINE_IOCTL_H

#include "mfis-ioctl.h"

/**
 * @brief Pipeline states
 */
#define PIPELINE_STATE_NOT_CONFIGURED (0x00) /* Pipeline not yet configured */
#define PIPELINE_STATE_CONFIGURED     (0x01) /* Pipeline is configured */
#define PIPELINE_STATE_RUNNING        (0x02) /* Pipeline is running */

/**
 * @brief Pipeline geometry
 */
struct pipeline_geometry {
    uint32_t width;
    uint32_t height;
};

/**
 * @brief Pipeline geometry
 */
struct pipeline_configure {
    uint8_t pipeline_id;
    struct pipeline_geometry geometry;
};

/**
 * @brief Pipeline I/O operations
 */
#define IOCGPIPELINESTATE     MFIS_IOR(0, sizeof(uint8_t))                   /* Gets the pipeline state */
#define IOCGPIPELINEGEOMETRY  MFIS_IOR(1, sizeof(struct pipeline_geometry))  /* Gets the pipeline geometry */
#define IOCSPIPELINECONFIGURE MFIS_IOW(2, sizeof(struct pipeline_configure)) /* Configures the pipeline */
#define IOCSPIPELINESTART     MFIS_IOW(3, sizeof(uint8_t))            /* Starts the pipeline */
#define IOCSPIPELINESTOP      MFIS_IOW(4, sizeof(uint8_t))            /* Stops the pipeline */

#endif /* _PIPEINE_IOCTL_H */

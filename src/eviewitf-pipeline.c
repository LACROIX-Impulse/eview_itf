/**
 * \file
 * \brief Communication API between A53 and R7 CPUs for camera devices
 * \author LACROIX Impulse
 *
 * API to communicate with the R7 CPU from the A53 (Linux).
 *
 */
#include <unistd.h>

#include "eviewitf-priv.h"
#include "pipeline-ioctl.h"
#include "mfis-communication.h"

/**
 * \fn eviewitf_pipeline_start(uint8_tv pipeline_id)
 * \brief Starts a pipeline
 *
 * \param[in] pipeline_id id of the pipeline between 0 and EVIEWITF_MAX_PIPELINE
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_pipeline_start(uint8_t pipeline_id) {
    int ret;

    ret = mfis_ioctl_request(MFIS_DEV_PIPELINE, pipeline_id, IOCPIPELINESTART, NULL);

    return ret;
}

/**
 * \fn eviewitf_pipeline_stop(uint8_t pipeline_id)
 * \brief Starts a pipeline
 *
 * \param[in] pipeline_id id of the pipeline between 0 and EVIEWITF_MAX_PIPELINE
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_pipeline_stop(uint8_t pipeline_id) {
    int ret;

    ret = mfis_ioctl_request(MFIS_DEV_PIPELINE, pipeline_id, IOCPIPELINESTOP, NULL);

    return ret;
}

/**
 * \fn  eviewitf_pipeline_configure(uint8_t pipeline_id, uint32_t frame_width, uint32_t frame_height)
 * \brief Configure a pipeline
 *
 * \param[in] pipeline_id id of the pipeline between 0 and EVIEWITF_MAX_PIPELINE
 * \param[in] frame_width frame width between 0 to 4096
 * \param[in] frame_width frame height between 0 to 4096
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_pipeline_configure(uint8_t pipeline_id, uint32_t frame_width, uint32_t frame_height) {
    int ret;

    struct pipeline_geometry geometry = {.height = frame_height, .width = frame_width};
    ret = mfis_ioctl_request(MFIS_DEV_PIPELINE, pipeline_id, IOCSPIPELINECONFIGURE, &geometry);

    return ret;
}

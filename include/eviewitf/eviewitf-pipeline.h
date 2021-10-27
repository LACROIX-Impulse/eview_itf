/**
 * \file
 * \brief Header for eViewItf API regarding cameras
 * \author LACROIX Impulse
 * \copyright Copyright (c) 2019-2021 LACROIX Impulse
 * \ingroup camera
 *
 * Communication API between A53 and R7 CPUs for camera devices
 *
 * \addtogroup camera
 * \{
 */

#ifndef EVIEWITF_PIPELINE_H
#define EVIEWITF_PIPELINE_H

#include <stdint.h>
#include "eviewitf-structs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \def EVIEWITF_MAX_PIPELINE
 * \brief Max number of pipeline devices
 */
#define EVIEWITF_MAX_PIPELINE 2

/**
 * \fn eviewitf_pipeline_start(int pipeline_id)
 * \brief Starts a pipeline
 *
 * \param[in] pipeline_id id of the pipeline between 0 and EVIEWITF_MAX_PIPELINE
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_pipeline_start(int pipeline_id);

/**
 * \fn eviewitf_pipeline_stop(int pipeline_id)
 * \brief Starts a pipeline
 *
 * \param[in] pipeline_id id of the pipeline between 0 and EVIEWITF_MAX_PIPELINE
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_pipeline_stop(int pipeline_id);

/**
 * \fn  eviewitf_pipeline_configure(int pipeline_id, uint32_t frame_width, uint32_t frame_height)
 * \brief Configure a pipeline
 *
 * \param[in] pipeline_id id of the pipeline between 0 and EVIEWITF_MAX_PIPELINE
 * \param[in] frame_width frame width between 0 to 4096
 * \param[in] frame_height frame height between 0 to 4096
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_pipeline_configure(int pipeline_id, uint32_t frame_width, uint32_t frame_height);

#ifdef __cplusplus
}
#endif

#endif /* EVIEWITF_PIPELINE_H */

/*! \} */

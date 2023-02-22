/**
 * \file
 * \brief Header for eViewItf API regarding pipelines
 * \author LACROIX Impulse
 * \copyright Copyright (c) 2019-2022 LACROIX Impulse
 * \ingroup pipeline
 *
 * Communication API between A53 and R7 CPUs for pipelines related operations
 *
 * \addtogroup pipeline
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
 * \fn eviewitf_pipeline_start(uint8_t pipeline_id)
 * \brief Starts a pipeline
 *
 * \param[in] pipeline_id id of the pipeline between 0 and EVIEWITF_MAX_PIPELINE
 * \return return code as specified by the eviewitf_ret_t enumeration.
 */
eviewitf_ret_t eviewitf_pipeline_start(uint8_t pipeline_id);

/**
 * \fn eviewitf_pipeline_stop(uint8_t pipeline_id)
 * \brief Starts a pipeline
 *
 * \param[in] pipeline_id id of the pipeline between 0 and EVIEWITF_MAX_PIPELINE
 * \return return code as specified by the eviewitf_ret_t enumeration.
 */
eviewitf_ret_t eviewitf_pipeline_stop(uint8_t pipeline_id);

/**
 * \fn eviewitf_pipeline_reboot(uint8_t pipeline_id)
 * \brief Reboots a pipeline R7/A53
 *
 * \param[in] pipeline_id id of the pipeline between 0 and EVIEWITF_MAX_PIPELINE
 * \return return code as specified by the eviewitf_ret_t enumeration.
 */
eviewitf_ret_t eviewitf_pipeline_reboot(uint8_t pipeline_id);

/**
 * \fn  eviewitf_pipeline_configure(uint8_t pipeline_id, uint32_t frame_width, uint32_t frame_height)
 * \brief Configure a pipeline
 *
 * \param[in] pipeline_id id of the pipeline between 0 and EVIEWITF_MAX_PIPELINE
 * \param[in] frame_width frame width between 0 to 4096
 * \param[in] frame_height frame height between 0 to 4096
 * \return return code as specified by the eviewitf_ret_t enumeration.
 */
eviewitf_ret_t eviewitf_pipeline_configure(uint8_t pipeline_id, uint32_t frame_width, uint32_t frame_height);

/**
 * \fn  eviewitf_pipeline_set_led(uint8_t pipeline_id, uint8_t led_id, uint8_t led_level)
 * \brief Configure a pipeline
 *
 * \param[in] pipeline_id id of the pipeline between 0 and EVIEWITF_MAX_PIPELINE
 * \param[in] led_id id of the pipeline led between 0 and 2
 * \param[in] led_level level of the pipeline led between 0 and 1
 * \return return code as specified by the eviewitf_ret_t enumeration.
 */
eviewitf_ret_t eviewitf_pipeline_set_led(uint8_t pipeline_id, uint8_t led_id, uint8_t led_level);

#ifdef __cplusplus
}
#endif

#endif /* EVIEWITF_PIPELINE_H */

/*! \} */

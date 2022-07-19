/**
 * \file
 * \brief Header for eViewItf API regarding plots
 * \author LACROIX Impulse
 * \copyright Copyright (c) 2019-2022 LACROIX Impulse
 * \ingroup blender
 *
 * Communication API between A53 and R7 CPUs for plot functions
 *
 * \addtogroup plot
 * \{
 */

#ifndef EVIEWITF_PLOT_H
#define EVIEWITF_PLOT_H

#include <stdint.h>
#include "eviewitf-structs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \fn eviewitf_plot_bounding_box(eviewitf_frame_attributes_t *frame, eviewitf_bounding_box_attributes_t *bounding_box)
 * \brief Plots a bouding box into a frame
 *
 * \param frame: frame pointer where to draw the bounding box
 * \param bounding_box: bounding box pointer to draw
 *
 * \return Return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_plot_bounding_box(eviewitf_frame_attributes_t *frame, eviewitf_bounding_box_attributes_t *bounding_box);

#ifdef __cplusplus
}
#endif

#endif /* EVIEWITF_PLOT_H */

/*! \} */

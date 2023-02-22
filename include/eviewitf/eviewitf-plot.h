/**
 * @file eviewitf-plot.h
 * @brief Header for eViewItf API regarding plots
 * @author LACROIX Impulse
 * @copyright Copyright (c) 2019-2022 LACROIX Impulse
 * @ingroup blender
 *
 * Communication API between A53 and R7 CPUs for plot functions
 *
 * @addtogroup plot
 * @{
 */

#ifndef EVIEWITF_PLOT_H
#define EVIEWITF_PLOT_H

#include <stdint.h>
#include "eviewitf-structs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
/**
 * @fn eviewitf_plot_rectangle(eviewitf_plot_frame_attributes_t *frame, eviewitf_plot_rectangle_attributes_t *rect)
 * @brief Plots a rectangle into a frame
 *
 * @param frame: Frame attributes pointer where to plot the rectangle
 * @param rect: Rectangle attributes pointer to plot
 *
 * @return Return code as specified by the eviewitf_ret_t enumeration.
 */
/* clang-format on */
eviewitf_ret_t eviewitf_plot_rectangle(eviewitf_plot_frame_attributes_t *frame,
                                       eviewitf_plot_rectangle_attributes_t *rect);

/* clang-format off */
/**
 * @fn eviewitf_plot_text(eviewitf_plot_frame_attributes_t *frame, eviewitf_plot_text_attributes_t *text)
 * @brief Plots a text into a frame
 *
 * @param frame: Frame attributes pointer where to plot the text
 * @param text: Text attribtues pointer to plot
 *
 * @return Return code as specified by the eviewitf_ret_t enumeration.
 */
/* clang-format on */
eviewitf_ret_t eviewitf_plot_text(eviewitf_plot_frame_attributes_t *frame, eviewitf_plot_text_attributes_t *text);

#ifdef __cplusplus
}
#endif

#endif /* EVIEWITF_PLOT_H */

/*! \} */

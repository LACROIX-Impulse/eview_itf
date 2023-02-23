/**
 * @file camera.h
 * @brief Module camera
 * @author LACROIX Impulse
 *
 * The module Camera handles operations that relate to streams and cameras
 *
 */
#ifndef _CAMERA_H
#define _CAMERA_H

#include <stdint.h>

#include "eviewitf.h"

/**
 * @fn eviewitf_ret_t camera_parse(int argc, char **argv)
 * @brief Parse the parameters and execute the  function
 * @param[in] argc arguments count
 * @param[in] argv arguments
 * @return return code as specified by the eviewitf_ret_t enumeration.
 */
eviewitf_ret_t camera_parse(int argc, char **argv);

#endif /* _CAMERA_H */

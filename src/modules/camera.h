/**
 * \file
 * \brief Module camera
 * \author LACROIX Impulse
 *
 * The module Camera handles operations that relate to streams and cameras
 *
 */
#ifndef _CAMERA_H
#define _CAMERA_H

#include <stdint.h>

/**
 * @brief Parse the parameters and execute the  function
 * @param[in] argc arguments count
 * @param[in] argv arguments
 * @return
 */
int camera_parse(int argc, char **argv);

#endif /* _CAMERA_H */

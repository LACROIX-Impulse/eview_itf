/**
 * @file legacy.h
 * @brief Module camera
 * @author LACROIX Impulse
 *
 * The module Camera handles operations that relate to streams and cameras
 *
 */
#ifndef _LEGACY_H
#define _LEGACY_H

#include <stdint.h>

#include "eviewitf.h"

/**
 * @brief Parse the parameters and execute the  function
 * @param[in] argc arguments count
 * @param[in] argv arguments
 * @return return code as specified by the eviewitf_ret_t enumeration.
 */
eviewitf_ret_t legacy_parse(int argc, char **argv);

#endif /* _LEGACY_H */

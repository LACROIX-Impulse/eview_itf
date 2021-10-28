/**
 * \file
 * \brief Module pipeline
 * \author LACROIX Impulse
 *
 * The module Pipeline handles operations that relate to pipeline
 *
 */
#ifndef _PIPELINE_H
#define _PIPELINE_H

#include <stdint.h>

/**
 * @brief Parse the parameters and execute the  function
 * @param[in] argc arguments count
 * @param[in] argv arguments
 * @return EVIEWITF_OK on success, negative value on failure (see eviewitf_return_code enum)
 */
int pipeline_parse(int argc, char **argv);

#endif /* _PIPELINE_H */

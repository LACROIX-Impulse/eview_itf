/**
 * @file pipeline.h
 * @brief Module pipeline
 * @author LACROIX Impulse
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
 * @return return code as specified by the eviewitf_ret_t enumeration.
 */
int pipeline_parse(int argc, char **argv);

#endif /* _PIPELINE_H */

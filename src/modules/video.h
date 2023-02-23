/**
 * @file video.h
 * @brief Module video
 * @author LACROIX Impulse
 *
 * The module Video handles operations that relate to video display
 *
 */
#ifndef _VIDEO_H
#define _VIDEO_H

#include <stdint.h>

/**
 * @fn eviewitf_ret_t video_parse(int argc, char **argv)
 * @brief Parse the parameters and execute the  function
 * @param[in] argc arguments count
 * @param[in] argv arguments
 * @return return code as specified by the eviewitf_ret_t enumeration.
 */
int video_parse(int argc, char **argv);

#endif /* _VIDEO_H */

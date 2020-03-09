/**
 * \file eviewitf_ssd.h
 * \brief Header for SSD relative operations
 * \author esoftthings
 */

#ifndef SRC_EVIEWITF_SSD_H_
#define SRC_EVIEWITF_SSD_H_
#include <time.h>
int ssd_get_output_directory(char **storage_directory);
int ssd_save_camera_stream(int camera_id, int duration, char *frames_directory, uint32_t size);
int ssd_set_virtual_camera_stream(int camera_id, uint32_t buffer_size, int fps, char *frames_directory);
int ssd_set_blending(uint32_t buffer_size, char *frame);
struct timespec diff(struct timespec start, struct timespec end);

#endif /* SRC_EVIEWITF_SSD_H_ */

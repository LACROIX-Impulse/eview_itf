/**
 * \file eviewitf_ssd.h
 * \brief Header for SSD relative operations
 * \author esoftthings
 */

#ifndef SRC_EVIEWITF_SSD_H_
#define SRC_EVIEWITF_SSD_H_

int ssd_get_output_directory(char **storage_directory);
int ssd_save_camera_stream(int camera_id, int duration, char *frames_directory);

#endif /* SRC_EVIEWITF_SSD_H_ */

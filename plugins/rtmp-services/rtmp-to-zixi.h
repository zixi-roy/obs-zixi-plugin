#ifndef _RTMP_TO_ZIXI_H_
#define _RTMP_TO_ZIXI_H_
#include <obs-module.h>
#include "../zixi-output/zixi-constants.h"
void add_zixi_fwd_properties(obs_properties_t *ppts);
void update_zixi_settings(struct zixi_settings* zixi,   obs_data_t* settings, const char* rtmp_url, const char* rtmp_key, const char * rtmp_user, const char* rtmp_password);
void zixi_clear_settings(struct zixi_settings* zixi);
void zixi_fill_defaults(  obs_data_t* settings);
#endif

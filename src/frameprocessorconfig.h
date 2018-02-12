#ifndef _FRAMEPROCESSORCONFIG_H_
#define _FRAMEPROCESSORCONFIG_H_

#include <stdbool.h>

typedef struct FrameProcessorConfig {
    int play_area_w;
    int play_area_h;
    int play_area_x;
    int play_area_y;

    int win_cond_w;
    int win_cond_h;
    int win_cond_x;
    int win_cond_y;

    int win_team_w;
    int win_team_h;
    int win_team_x;
    int win_team_y;

    int name_gold_w;
    int name_gold_h;
    int name_gold_x;
    int name_gold_y;

    int name_blue_w;
    int name_blue_h;
    int name_blue_x;
    int name_blue_y;

    char* win_gold_img;
    char* win_blue_img;
    char* win_econ_img;
    char* win_mili_img;
    char* win_snail_img;
} FrameProcessorConfig;

FrameProcessorConfig* frame_processor_config_create();
void frame_processor_config_load(FrameProcessorConfig* cfg, const char* path);
void frame_processor_config_destroy(FrameProcessorConfig* cfg);

#endif  // _FRAMEPROCESSORCONFIG_H_
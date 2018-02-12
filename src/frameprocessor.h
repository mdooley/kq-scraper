#ifndef _FRAMEPROCESSOR_H_
#define _FRAMEPROCESSOR_H_

#include <stdbool.h>
#include <stdint.h>

#include <MagickWand/MagickWand.h>
#include <leptonica/allheaders.h>
#include <tesseract/capi.h>

#include "frameprocessorconfig.h"
#include "frameprocessorresults.h"

typedef struct FrameProcessor {
    FrameProcessorConfig* cfg;
    MagickWand*           win_teams;
    MagickWand*           win_conditions;
    MagickWand*           image;
    TessBaseAPI*          t_api;
    Map                   curr_map;
    Map                   prev_map;
} FrameProcessor;

FrameProcessor* frame_processor_create();
void frame_processor_destroy(FrameProcessor* fp);
void frame_processor_init(FrameProcessor* fp, char* path);
bool frame_processor_process_image(FrameProcessor* fp, uint8_t* img, int img_length, FrameProcessorResults* fpr);
const char* frame_processor_get_map_name(Map m);
const char* frame_processor_get_win_name(Win w);
const char* frame_processor_get_team_name(Team t);

#endif  // _FRAMEPROCESSOR_H_

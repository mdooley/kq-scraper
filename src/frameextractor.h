#ifndef _FRAMEEXTRACTOR_H_
#define _FRAMEEXTRACTOR_H_

#include <stdbool.h>
#include <stdint.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>

typedef struct FrameExtractor {
    AVFormatContext *fcontext;
    AVCodec *vcodec;
    AVCodecContext *vcontext;
    struct SwsContext *swcontext;
    AVFrame *frame;
    AVFrame *rgb;
    uint8_t *rgb_buffer;
    uint8_t *ppm_buffer;
    int ppm_length;
    int vindex;
    int frame_count;
    int fps;
} FrameExtractor;

FrameExtractor* frame_extractor_create();
void frame_extractor_destroy(FrameExtractor* fe);
bool frame_extractor_open(FrameExtractor* fe, char* path);
bool frame_extractor_next_frame(FrameExtractor* fe);

#endif  // _FRAMEEXTRACTOR_H_

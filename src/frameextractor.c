#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "frameextractor.h"

static void frame_extractor_fill_ppm_buffer(FrameExtractor* fe);

FrameExtractor* frame_extractor_create() {
    FrameExtractor* fe = (FrameExtractor*) malloc(sizeof(FrameExtractor));

    fe->fcontext    = NULL;
    fe->vcodec      = NULL;
    fe->vcontext    = NULL;
    fe->swcontext   = NULL;
    fe->frame       = NULL;
    fe->rgb         = NULL;
    fe->rgb_buffer  = NULL;
    fe->ppm_buffer  = NULL;
    fe->ppm_length  = 0;
    fe->vindex      = -1;
    fe->frame_count = 0;
    fe->fps         = 0;

    return fe;
}

void frame_extractor_destroy(FrameExtractor* fe) {
    if (fe->fcontext != NULL)
        avformat_close_input(&fe->fcontext);

    sws_freeContext(fe->swcontext);

    if (fe->rgb != NULL)
        av_frame_free(&fe->rgb);

    if (fe->frame != NULL)
        av_frame_free(&fe->frame);

    free(fe->rgb_buffer);
    free(fe->ppm_buffer);
    free(fe);
}

bool frame_extractor_open(FrameExtractor* fe, char* path) {
    if (avformat_open_input(&fe->fcontext, path, NULL, NULL) < 0) {
        fprintf(stderr, "Can't open input file: %s\n", path);
        return false;
    }

    if (avformat_find_stream_info(fe->fcontext, NULL) < 0) {
        fprintf(stderr, "Can't find stream info for: %s\n", path);
        return false;
    }

    if ((fe->vindex = av_find_best_stream(fe->fcontext, AVMEDIA_TYPE_VIDEO, -1, -1, &fe->vcodec, 0)) < 0) {
        fprintf(stderr, "Can't find best video stream for: %s\n", path);
        return false;
    }

    if (fe->vcodec->id != AV_CODEC_ID_H264 && fe->vcodec->id != AV_CODEC_ID_VP9 && fe->vcodec->id != AV_CODEC_ID_MPEG4) {
        fprintf(stderr, "Not a supported video codec: %s\n", path); 
        return false;
    }

    fe->vcontext = fe->fcontext->streams[fe->vindex]->codec;
    fe->fps = ceil(fe->fcontext->streams[fe->vindex]->avg_frame_rate.num / fe->fcontext->streams[fe->vindex]->avg_frame_rate.den) * 3;

    av_opt_set_int(fe->vcontext, "refcounted_frames", 1, 0);

    if (avcodec_open2(fe->vcontext, fe->vcodec, NULL) < 0) {
        fprintf(stderr, "Can't open video decoder for: %s\n", path);
        return false;
    }

    if ((fe->swcontext = sws_getContext(fe->vcontext->width, fe->vcontext->height, fe->vcontext->pix_fmt,
                                        fe->vcontext->width, fe->vcontext->height, AV_PIX_FMT_RGB24,
                                        SWS_BICUBIC, NULL, NULL, NULL)) == NULL) {
        fprintf(stderr, "Can't create scaling/conversion context\n");
        return false;
    }

    if ((fe->frame = av_frame_alloc()) == NULL)
        return false;

    if ((fe->rgb = av_frame_alloc()) == NULL)
        return false;

    fe->ppm_buffer = (uint8_t*) malloc(7*1024*1024);

    int num_bytes = avpicture_get_size(AV_PIX_FMT_RGB24, fe->vcontext->width, fe->vcontext->height);
    fe->rgb_buffer = (uint8_t*) malloc(num_bytes);
    avpicture_fill((AVPicture *)fe->rgb, fe->rgb_buffer, AV_PIX_FMT_RGB24, fe->vcontext->width, fe->vcontext->height);

    return true;
}

bool frame_extractor_next_frame(FrameExtractor* fe) {
    AVPacket packet;
    int got_frame = 0, read_frames = 0;
    bool done = false, ret_val = false;

    while (!done) {
        // try to read a packet
        if (av_read_frame(fe->fcontext, &packet) >= 0) {
            // only proceed if it's part of the video stream
            if (packet.stream_index == fe->vindex) {
                // try to decode the video frame
                if (avcodec_decode_video2(fe->vcontext, fe->frame, &got_frame, &packet) >= 0) {
                    // decoded a frame, if we've decoded enough for our target interval, fill the ppm buffer
                    if (got_frame && ++read_frames >= fe->fps) {
                        fe->frame->pts = av_frame_get_best_effort_timestamp(fe->frame);
                        sws_scale(fe->swcontext, (const uint8_t* const*)fe->frame->data,
                                  fe->frame->linesize, 0, fe->vcontext->height,
                                  fe->rgb->data, fe->rgb->linesize);
                        frame_extractor_fill_ppm_buffer(fe);
                        ret_val = true;
                        done = true;
                    }
                } else {
                    fprintf(stderr, "Failed decoding video\n");
                    done = true;
                }
            }

            av_frame_unref(fe->frame);
        } else {
            done = true;
        }

        av_packet_unref(&packet);
    }

    return ret_val;
}

static void frame_extractor_fill_ppm_buffer(FrameExtractor* fe) {
    char *ppm = (char *)fe->ppm_buffer;
    fe->ppm_length = 0;

    sprintf(ppm, "P6\n%d %d\n%d\n", fe->vcontext->width, fe->vcontext->height, 255);
    int length = strlen(ppm);
    ppm += length;
    fe->ppm_length += length;

    for (int i = 0; i < fe->vcontext->height; i++) {
        memcpy(ppm, fe->rgb->data[0] + (i*fe->rgb->linesize[0]), fe->vcontext->width*3);
        ppm += fe->vcontext->width*3;
        fe->ppm_length += fe->vcontext->width*3;
    }
}

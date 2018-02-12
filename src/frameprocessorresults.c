#include "frameprocessorresults.h"

#include <stdlib.h>
#include <string.h>

#include <tesseract/capi.h>

FrameProcessorResults* frame_processor_results_create() {
    FrameProcessorResults* fpr = (FrameProcessorResults*) malloc(sizeof(FrameProcessorResults));

    fpr->map           = MAP_UNKNOWN;
    fpr->win_condition = WIN_UNKNOWN;
    fpr->win_team      = TEAM_UNKNOWN;
    fpr->gold_team     = NULL;
    fpr->blue_team     = NULL;

    return fpr;
}

void frame_processor_results_clear(FrameProcessorResults* fpr) {
    fpr->map           = MAP_UNKNOWN;
    fpr->win_condition = WIN_UNKNOWN;
    fpr->win_team      = TEAM_UNKNOWN;

    TessDeleteText(fpr->gold_team);
    fpr->gold_team = NULL;

    TessDeleteText(fpr->blue_team);
    fpr->blue_team = NULL;
}

void frame_processor_results_destroy(FrameProcessorResults* fpr) {
    frame_processor_results_clear(fpr);
    free(fpr);
}

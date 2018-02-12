#include "frameprocessor.h"

#include <string.h>
#include <ctype.h>

#include "gold.h"
#include "blue.h"
#include "economy.h"
#include "military.h"
#include "snail.h"

static const double kMapRGBs[][3] = { { 0.27058, 0.65098, 0.80784 },  // day
                                      { 0.15686, 0.22352, 0.36470 },  // night
                                      { 0.71372, 0.41568, 0.10196 },  // dusk
                                      { 0,       0,       0       }   // win screen
                                    };

static const char* kMapNames[] = { "DAY",
                                   "NIGHT",
                                   "DUSK",
                                   "WIN SCREEN"
                                 };

static const char* kWinNames[] = { "ECONOMY",
                                   "MILITARY",
                                   "SNAIL"
                                 };

static const char* kTeamNames[] = { "GOLD",
                                    "BLUE"
                                  };

static const char kUnknown[] = "N/A";

static Map   frame_processor_determine_map(FrameProcessor* fp);
static Win   frame_processor_determine_win_cond(FrameProcessor* fp);
static Team  frame_processor_determine_win_team(FrameProcessor* fp);
static char* frame_processor_determine_team_name(FrameProcessor* fp, int w, int h, int x, int y);
static void  frame_processor_load_image(MagickWand* wand, char* path, const unsigned char* default_img, int length);
static void  trim(char* str);

FrameProcessor* frame_processor_create() {
    FrameProcessor* fp = (FrameProcessor*) malloc(sizeof(FrameProcessor));

    fp->cfg            = NULL;
    fp->win_teams      = NULL;
    fp->win_conditions = NULL;
    fp->image          = NULL;
    fp->t_api          = NULL;
    fp->curr_map       = MAP_UNKNOWN;
    fp->prev_map       = MAP_UNKNOWN;

    return fp;
}

void frame_processor_init(FrameProcessor* fp, char* path) {
    fp->cfg = frame_processor_config_create();
    if (path != NULL)
        frame_processor_config_load(fp->cfg, path);

    fp->win_teams = NewMagickWand();
    fp->win_conditions = NewMagickWand();
    fp->image = NewMagickWand();

    frame_processor_load_image(fp->win_teams, fp->cfg->win_gold_img, kDefaultGold, sizeof(kDefaultGold));
    frame_processor_load_image(fp->win_teams, fp->cfg->win_blue_img, kDefaultBlue, sizeof(kDefaultBlue));
    frame_processor_load_image(fp->win_conditions, fp->cfg->win_econ_img, kDefaultEconomy,  sizeof(kDefaultEconomy));
    frame_processor_load_image(fp->win_conditions, fp->cfg->win_mili_img, kDefaultMilitary, sizeof(kDefaultMilitary));
    frame_processor_load_image(fp->win_conditions, fp->cfg->win_snail_img, kDefaultSnail,    sizeof(kDefaultSnail));

    fp->t_api = TessBaseAPICreate();
    TessBaseAPIInit3(fp->t_api, NULL, "eng");
    TessBaseAPISetVariable(fp->t_api, "debug_file", "/dev/null");
}

void frame_processor_destroy(FrameProcessor* fp) {
    TessBaseAPIEnd(fp->t_api);
    TessBaseAPIDelete(fp->t_api);
    DestroyMagickWand(fp->image);
    DestroyMagickWand(fp->win_conditions);
    DestroyMagickWand(fp->win_teams);

    free(fp);
}

bool frame_processor_process_image(FrameProcessor* fp, uint8_t* img, int img_length, FrameProcessorResults* fpr) {
    bool results_available = false;

    if (MagickGetNumberImages(fp->image) > 0)
        ClearMagickWand(fp->image);

    MagickReadImageBlob(fp->image, img, img_length);

    if (MagickGetImageWidth(fp->image) == 1280 && MagickGetImageHeight(fp->image) > 720) {
        MagickCropImage(fp->image, 1280, 720, 0, 0);
    } else if (MagickGetImageWidth(fp->image) == 1920 && MagickGetImageHeight(fp->image) >= 1080) {
        MagickCropImage(fp->image, 1920, 1080, 0, 0);
        MagickScaleImage(fp->image, 1280, 720);
    } else if (MagickGetImageWidth(fp->image) != 1280 && MagickGetImageHeight(fp->image) != 720) {
        fprintf(stderr, "Invalid image dimensions: W: %zd H: %zd\n", 
                        MagickGetImageWidth(fp->image), MagickGetImageHeight(fp->image));
        return false;
    }

    Map m = frame_processor_determine_map(fp);
    if (m == WINSCREEN) {
        if (fp->curr_map != MAP_UNKNOWN) {
            Win w = frame_processor_determine_win_cond(fp);
            if (w != WIN_UNKNOWN) {
                fpr->map           = fp->curr_map;
                fpr->win_condition = w;
                fpr->win_team      = frame_processor_determine_win_team(fp);
                fpr->gold_team     = frame_processor_determine_team_name(fp,
                                        fp->cfg->name_gold_w, fp->cfg->name_gold_h,
                                        fp->cfg->name_gold_x, fp->cfg->name_gold_y);
                fpr->blue_team     = frame_processor_determine_team_name(fp,
                                        fp->cfg->name_blue_w, fp->cfg->name_blue_h,
                                        fp->cfg->name_blue_x, fp->cfg->name_blue_y);

                results_available = true;
                fp->curr_map = MAP_UNKNOWN;
            }
        }
    } else if (fp->curr_map == MAP_UNKNOWN || fp->prev_map == m) {
        fp->curr_map = m;
    }

    fp->prev_map = m;

    return results_available;
}

static Map frame_processor_determine_map(FrameProcessor* fp) {
    MagickWand* img = CloneMagickWand(fp->image);
    PixelWand*  rgb = NewPixelWand();
    MagickCropImage(img, fp->cfg->play_area_w, fp->cfg->play_area_h,
                         fp->cfg->play_area_x, fp->cfg->play_area_y);
    MagickScaleImage(img, 1, 1);
    MagickGetImagePixelColor(img, 0, 0, rgb);

    Map index = DAY;
    double min_dist = pow((PixelGetRed(rgb)   - kMapRGBs[index][0]), 2) +
                      pow((PixelGetGreen(rgb) - kMapRGBs[index][1]), 2) +
                      pow((PixelGetBlue(rgb)  - kMapRGBs[index][2]), 2);

    for (Map i = NIGHT; i < MAP_NUM; i++) {
        double dist = pow((PixelGetRed(rgb)   - kMapRGBs[i][0]), 2) +
                      pow((PixelGetGreen(rgb) - kMapRGBs[i][1]), 2) +
                      pow((PixelGetBlue(rgb)  - kMapRGBs[i][2]), 2);
        if (dist < min_dist) {
            min_dist = dist;
            index = i;
        }
    }

    DestroyPixelWand(rgb);
    DestroyMagickWand(img);

    return index;
}

static Win frame_processor_determine_win_cond(FrameProcessor* fp) {
    MagickWand* img = CloneMagickWand(fp->image);
    MagickCropImage(img, fp->cfg->win_cond_w, fp->cfg->win_cond_h,
                         fp->cfg->win_cond_x, fp->cfg->win_cond_y);
    MagickNegateImage(img, MagickFalse);
    MagickThresholdImage(img, QuantumRange * 0.50);
    MagickTrimImage(img, 0);

    PixelWand* color = NewPixelWand();
    PixelSetColor(color, "white");

    MagickWand* base = NewMagickWand();
    MagickNewImage(base, fp->cfg->win_cond_w + 20, 
                         fp->cfg->win_cond_h + 15, color);
    MagickCompositeImageGravity(base, img, OverCompositeOp, CenterGravity);

    double economy_diff, military_diff, snail_diff;
    MagickSetFirstIterator(fp->win_conditions);
    MagickCompareImages(base, fp->win_conditions, AbsoluteErrorMetric, &economy_diff);
    MagickNextImage(fp->win_conditions);
    MagickCompareImages(base, fp->win_conditions, AbsoluteErrorMetric, &military_diff);
    MagickNextImage(fp->win_conditions);
    MagickCompareImages(base, fp->win_conditions, AbsoluteErrorMetric, &snail_diff);

    DestroyPixelWand(color);
    DestroyMagickWand(img);
    DestroyMagickWand(base);

    // printf("E: %f M: %f S: %f\n", economy_diff, military_diff, snail_diff);

    if (economy_diff > 1500 && military_diff > 1500 && snail_diff > 1500)
    	return WIN_UNKNOWN;
    else if (economy_diff - military_diff < -500 && economy_diff - snail_diff < -500)
        return ECONOMY;
    else if (military_diff - economy_diff < -500 && military_diff - snail_diff < -500)
        return MILITARY;
    else if (snail_diff - economy_diff < -500 && snail_diff - military_diff < -500)
        return SNAIL;
    else
        return WIN_UNKNOWN;
}

static Team frame_processor_determine_win_team(FrameProcessor* fp) {
    MagickWand* img = CloneMagickWand(fp->image);
    MagickCropImage(img, fp->cfg->win_team_w, fp->cfg->win_team_h,
                         fp->cfg->win_team_x, fp->cfg->win_team_y);
    MagickNegateImage(img, MagickFalse);
    MagickThresholdImage(img, QuantumRange * 0.50);
    MagickTrimImage(img, 0);

    PixelWand* color = NewPixelWand();
    PixelSetColor(color, "white");

    MagickWand* base = NewMagickWand();
    MagickNewImage(base, fp->cfg->win_team_w + 20,
                         fp->cfg->win_team_h + 10, color);
    MagickCompositeImageGravity(base, img, OverCompositeOp, CenterGravity);

    double gold_diff, blue_diff;
    MagickSetFirstIterator(fp->win_teams);
    MagickCompareImages(base, fp->win_teams, AbsoluteErrorMetric, &gold_diff);
    MagickNextImage(fp->win_teams);
    MagickCompareImages(base, fp->win_teams, AbsoluteErrorMetric, &blue_diff);

    DestroyPixelWand(color);
    DestroyMagickWand(img);
    DestroyMagickWand(base);

    // printf("G: %f B: %f\n", gold_diff, blue_diff);

    if (gold_diff < blue_diff)
        return GOLD;
    else if (blue_diff < gold_diff)
        return BLUE;
    else
        return TEAM_UNKNOWN;
}

static char* frame_processor_determine_team_name(FrameProcessor* fp, int w, int h, int x, int y) {
    PixelWand* color = NewPixelWand();
    PixelSetColor(color, "white");

    MagickWand* img = CloneMagickWand(fp->image);
    MagickCropImage(img, w, h, x, y);
    MagickNegateImage(img, MagickFalse);
    MagickThresholdImage(img, QuantumRange * 0.30);
    MagickTrimImage(img, 0);
    MagickBorderImage(img, color, 20, 10, InCompositeOp);
    MagickSetImageFormat(img, "PNG");

    size_t blob_length = 0;
    unsigned char* blob = MagickGetImageBlob(img, &blob_length);
    struct Pix* png = pixReadMemPng(blob, blob_length);

    TessBaseAPISetImage2(fp->t_api, png);

    char* team = TessBaseAPIGetUTF8Text(fp->t_api);
    trim(team);

    pixDestroy(&png);
    MagickRelinquishMemory(blob);
    DestroyMagickWand(img);
    DestroyPixelWand(color);

    return team;
}

static void frame_processor_load_image(MagickWand* wand, char* path, const unsigned char* default_img, int length) {
    if (path == NULL || !MagickReadImage(wand, path)) {
        if (path != NULL)
            fprintf(stderr, "Can't load image: %s. Using default.\n", path);

        MagickReadImageBlob(wand, default_img, length);
    }
}

const char* frame_processor_get_map_name(Map m) {
    if (m < DAY || m > WINSCREEN)
        return kUnknown;
    else
        return kMapNames[m];
}

const char* frame_processor_get_win_name(Win w) {
    if (w < ECONOMY || w > SNAIL)
        return kUnknown;
    else
        return kWinNames[w];
}

const char* frame_processor_get_team_name(Team t) {
    if (t < GOLD || t > BLUE)
        return kUnknown;
    else
        return kTeamNames[t];
}

static void trim(char* str) {
    char* start;
    char* end;

    for (start = str; *start; start++) {
        if (!isspace((unsigned char)start[0]))
            break;
    }

    for (end = start + strlen(start); end > start+1; end--) {
        if (!isspace((unsigned char)end[-1]))
            break;
    }

    *end = 0;

    if (start > str)
        memmove(str, start, (end - start) + 1);
}

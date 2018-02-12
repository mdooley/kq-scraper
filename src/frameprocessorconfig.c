#include "frameprocessorconfig.h"

#include <stdlib.h>
#include <string.h>

#include "ini.h"

static int match(const char* s, const char* section, const char* n, const char* name);
static int handler(void* user, const char* section, const char* name, const char* value);

FrameProcessorConfig* frame_processor_config_create() {
    FrameProcessorConfig* cfg = (FrameProcessorConfig*) malloc(sizeof(FrameProcessorConfig));

    cfg->play_area_w = 1077;
    cfg->play_area_h = 596;
    cfg->play_area_x = 102;
    cfg->play_area_y = 2;

    cfg->win_cond_w = 273;
    cfg->win_cond_h = 25;
    cfg->win_cond_x = 505;
    cfg->win_cond_y = 446;

    cfg->win_team_w = 260;
    cfg->win_team_h = 48;
    cfg->win_team_x = 515;
    cfg->win_team_y = 113;

    cfg->name_gold_w = 290;
    cfg->name_gold_h = 35;
    cfg->name_gold_x = 310;
    cfg->name_gold_y = 604;

    cfg->name_blue_w = 290;
    cfg->name_blue_h = 35;
    cfg->name_blue_x = 684;
    cfg->name_blue_y = 604;

    cfg->win_gold_img  = NULL;
    cfg->win_blue_img  = NULL;
    cfg->win_econ_img  = NULL;
    cfg->win_mili_img  = NULL;
    cfg->win_snail_img = NULL;

    return cfg;
}

void frame_processor_config_load(FrameProcessorConfig* cfg, const char* path) {
    if (ini_parse(path, handler, cfg) < 0)
        fprintf(stderr, "Can't load configration file: %s. Using defaults.\n", path);
}

void frame_processor_config_destroy(FrameProcessorConfig* cfg) {
    free(cfg->win_blue_img);
    free(cfg->win_gold_img);
    free(cfg->win_econ_img);
    free(cfg->win_mili_img);
    free(cfg->win_snail_img);
    free(cfg);
}

static int match(const char* section, const char* s, const char* name, const char* n) {
    return strcmp(section, s) == 0 && strcmp(name, n) == 0;
}

static int handler(void* user, const char* section, const char* name, const char* value) {
    FrameProcessorConfig* cfg = (FrameProcessorConfig*) user;

    if (match("play_area", section, "width", name)) {
        cfg->play_area_w = atoi(value);
    } else if (match("play_area", section, "height", name)) {
        cfg->play_area_h = atoi(value);
    } else if (match("play_area", section, "x", name)) {
        cfg->play_area_x = atoi(value);
    } else if (match("play_area", section, "y", name)) {
        cfg->play_area_y = atoi(value);
    } else if (match("win_condition_area", section, "width", name)) {
        cfg->win_cond_w = atoi(value);
    } else if (match("win_condition_area", section, "height", name)) {
        cfg->win_cond_h = atoi(value);
    } else if (match("win_condition_area", section, "x", name)) {
        cfg->win_cond_x = atoi(value);
    } else if (match("win_condition_area", section, "y", name)) {
        cfg->win_cond_y = atoi(value);
    } else if (match("win_team_area", section, "width", name)) {
        cfg->win_team_w = atoi(value);
    } else if (match("win_team_area", section, "height", name)) {
        cfg->win_team_h = atoi(value);
    } else if (match("win_team_area", section, "x", name)) {
        cfg->win_team_x = atoi(value);
    } else if (match("win_team_area", section, "y", name)) {
        cfg->win_team_y = atoi(value);
    } else if (match("gold_name_area", section, "width", name)) {
        cfg->name_gold_w = atoi(value);
    } else if (match("gold_name_area", section, "height", name)) {
        cfg->name_gold_h = atoi(value);
    } else if (match("gold_name_area", section, "x", name)) {
        cfg->name_gold_x = atoi(value);
    } else if (match("gold_name_area", section, "y", name)) {
        cfg->name_gold_y = atoi(value);
    } else if (match("blue_name_area", section, "width", name)) {
        cfg->name_blue_w = atoi(value);
    } else if (match("blue_name_area", section, "height", name)) {
        cfg->name_blue_h = atoi(value);
    } else if (match("blue_name_area", section, "x", name)) {
        cfg->name_blue_x = atoi(value);
    } else if (match("blue_name_area", section, "y", name)) {
        cfg->name_blue_y = atoi(value);
    } else if (match("images", section, "gold", name)) {
        cfg->win_gold_img = strdup(value);
    } else if (match("images", section, "blue", name)) {
        cfg->win_blue_img = strdup(value);
    } else if (match("images", section, "economy", name)) {
        cfg->win_econ_img = strdup(value);
    } else if (match("images", section, "military", name)) {
        cfg->win_mili_img = strdup(value);
    } else if (match("images", section, "snail", name)) {
        cfg->win_snail_img = strdup(value);
    } else {
        return 0;
    }

    return 1;
}

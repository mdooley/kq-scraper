#ifndef _FRAMEPROCESSORRESULTS_H_
#define _FRAMEPROCESSORRESULTS_H_

typedef enum Map {
    DAY = 0,
    NIGHT,
    DUSK,
    WINSCREEN,
    MAP_NUM,
    MAP_UNKNOWN
} Map;

typedef enum Win {
    ECONOMY = 0,
    MILITARY,
    SNAIL,
    WIN_NUM,
    WIN_UNKNOWN
} Win;

typedef enum Team {
    GOLD = 0,
    BLUE,
    TEAM_NUM,
    TEAM_UNKNOWN,
} Team;

typedef struct FrameProcessorResults {
    Map   map;
    Win   win_condition;
    Team  win_team;
    char* gold_team;
    char* blue_team;
} FrameProcessorResults; 

FrameProcessorResults* frame_processor_results_create();
void frame_processor_results_clear(FrameProcessorResults* fpr);
void frame_processor_results_destroy(FrameProcessorResults* fpr);

#endif  // _FRAMEPROCESSORRESULTS_H_
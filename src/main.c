#include <stdio.h>
#include <getopt.h> 

#include "frameextractor.h"
#include "frameprocessor.h"

static void print_usage(char* argv[]) {
	printf("Usage: %s [--config config] [--prefix prefix] file\n", argv[0]);
}

int main(int argc, char** argv) {
	int c, index;
	char* prefix = NULL;
	char* config = NULL;

	struct option options[] = {
		{"prefix", required_argument, NULL, 'p'},
		{"config", required_argument, NULL, 'c'},
		{"help",   no_argument,       NULL, 'h'},
		{NULL,     0,                 NULL, 0},
	};

	while ((c = getopt_long(argc, argv, "p:c:h", options, &index)) != -1) {
		switch (c) {
			case 'h':
				print_usage(argv);
				exit(0);
				break;

			case 'p':
				prefix = optarg;
				break;

			case 'c':
				config = optarg;
				break;

			case '?':
				break;

			default:
                printf("getopt returned something weird: 0x%x\n", c);				
		}
	}

	if (optind >= argc) {
		printf("%s: no input file specified\n", argv[0]);
		print_usage(argv);
		exit(1);
	}

	av_register_all();
    MagickWandGenesis();

    FrameProcessor* fp = frame_processor_create();
    frame_processor_init(fp, config);

    while (optind < argc) {
        FrameExtractor* fe = frame_extractor_create();

        if (frame_extractor_open(fe, argv[optind++])) {
            FrameProcessorResults* fpr = frame_processor_results_create();

            while (frame_extractor_next_frame(fe)) {
                frame_processor_results_clear(fpr);

                if (frame_processor_process_image(fp, fe->ppm_buffer, fe->ppm_length, fpr)) {
                    if (prefix != NULL)
                        printf("%s,", prefix);

                    printf("%s,%s,%s,%s,%s\n", 
                        frame_processor_get_map_name(fpr->map),
                        frame_processor_get_win_name(fpr->win_condition),
                        frame_processor_get_team_name(fpr->win_team), 
                        fpr->gold_team, fpr->blue_team);
                }
            }

            frame_processor_results_destroy(fpr);
        }
    
        frame_extractor_destroy(fe);
    }

	frame_processor_destroy(fp);

    MagickWandTerminus();

    return 0;
}

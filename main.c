#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include "midi.h"
#include "pcspkr.h"
#include "timing.h"

int verbose=0; //should be changable from the command line

//declaring global so i can free in signal handler
struct midi_file_t midi_file;

void handle_sigint(int signal) {
	int i;
	printf("Aborting..\n");
	spkr_off();
	if(midi_file.tracks!=NULL)
		midi_free(&midi_file);
	exit(1);
}

int main(int argc, char* argv[]) {
	char* header_buf;
	int i;
	spkr_init();
	if(signal(SIGINT, handle_sigint)==SIG_IGN)
		signal(SIGINT, SIG_IGN);
	if(argc>1) {
		if(midi_import(argv[1], &midi_file)!=0) {
			printf("The specified file is not a valid midi file!\n");
			return 1;
		}
	} else {
		printf("Usage: midi file.mid\n");
		return 1;
	}
	
	printf("Found midi file with %hu tracks, PPQN: %hu\n", midi_file.header.track_count, midi_file.header.ppqn);
	printf("Playing..\n");
	
	float tempo=500.0;
	for(i=0;i<midi_file.header.track_count;i++) {
		printf("Track %i of %hu\n", i+1, midi_file.header.track_count);
		midi_play_track(midi_file.tracks[i], midi_file.header.ppqn, &tempo, &spkr_on, &spkr_off, verbose);
		msleep(500);
	}
	
	spkr_off();
	return 0;
}

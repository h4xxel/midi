#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "midi.h"
#include "timing.h"

uint32_t read_var_len(char** pp) {
	uint32_t value;
	char c, *op;
	op=*pp;
	
	if((c=**pp)&0x80) {
		value=c&0x7F; (*pp)++;
		do {
			value=(value<<7)+((c=**pp) & 0x7F); (*pp)++;
		} while(c&0x80&&(*pp)-op<sizeof(uint32_t));
	} else {
		value=c; (*pp)++;
	}
	return value;
}

uint32_t endian_swap32(uint32_t x) {
    return (x>>24) | 
		((x<<8) & 0x00FF0000) |
		((x>>8) & 0x0000FF00) |
		(x<<24);
}
uint16_t endian_swap16(uint16_t x) {
	return (x>>8)|(x<<8);
}

int midi_import(char* filename, struct midi_file_t* midi_file) {
	FILE* f;
	char buf[32];
	char* header_data;
	void* ptr;
	uint32_t l;
	size_t f_pos;
	int i;
	
	f=fopen(filename, "r");
	if(f==NULL)
		return 1;
	fread(buf, 8, 1, f);
	if(strncmp(buf, "MThd", 4)!=0) {
		fclose(f);
		return 1;
	}
	
	strncpy(midi_file->header.id, buf, 4);
	midi_file->header.size=endian_swap32(*((uint32_t*)(buf+4)));
	header_data=malloc(midi_file->header.size);
	ptr=header_data;
	fread(header_data, midi_file->header.size, 1, f);
	midi_file->header.format=endian_swap16(*(uint16_t*)ptr);
	ptr+=sizeof(uint16_t);
	//make sure we get no more than one track if format = 0
	midi_file->header.track_count=midi_file->header.format==0?1:endian_swap16(*(uint16_t*)ptr);
	ptr+=sizeof(uint16_t);
	midi_file->header.ppqn=endian_swap16(*(uint16_t*)ptr);
	free(header_data);
	//done parsing header
	
	midi_file->tracks=calloc(midi_file->header.track_count, sizeof(void*));
	for(i=0; i<midi_file->header.track_count; i++)
		midi_file->tracks[i]=malloc(sizeof(struct midi_track));
	f_pos=ftell(f);
	
	for(i=0;i<midi_file->header.track_count;i++) {
		fread(buf, 8, 1, f);
		f_pos+=8;
		l=endian_swap32(*((uint32_t*)(buf+4)));
		midi_file->tracks[i]->size=l;
		//ignore propritary chunks
		if(strncmp(buf, "MTrk", 4)!=0) {
			f_pos+=l;
			fseek(f, f_pos, SEEK_SET);
		} else {
			midi_file->tracks[i]->data=malloc(l);
			int j;
			for(j=0; j<l; j++) {
				midi_file->tracks[i]->data[j]=fgetc(f);
				f_pos++;
				fseek(f, f_pos, SEEK_SET);
			}
		}
	}
	fclose(f);
	return 0;
}

void midi_play_track(struct midi_track *track, uint16_t ppqn, float *tempo, void (*noteon_callback)(float), void (*noteoff_callback)(), int verbose) {
	unsigned char note;
	void *ptr, *ptr_end;
	ptr=track->data;
	ptr_end=ptr+track->size;
	while(ptr<ptr_end) {
		uint32_t delta_time;
		unsigned char event;
		delta_time=read_var_len((char**)&ptr);
		msleep((unsigned long)(((float)delta_time)*((*tempo)/((float)ppqn))));
		if((*((char*)ptr))&0x80) {
			//handle running status
			event=*((char*)ptr);
			ptr++;
		}
		switch(event&0xF0) {
			case 0x80:
				ptr+=2;
				noteoff_callback();
				if(verbose) printf("release delta %u\n", delta_time);
			break;
			case 0x90:
				note=*((char*)ptr);
				ptr+=2;
				noteon_callback(midi[note]);
				if(verbose) printf("press delta %u note %i channel %i\n", delta_time, (int)note, (int)(event&0x0F));
			break;
			case 0xA0: ptr+=2; if(verbose) printf("key after-touch\n"); break;
			case 0xB0: ptr+=2; if(verbose) printf("control change\n"); break;
			case 0xC0: ptr+=1; if(verbose) printf("program change\n"); break;
			case 0xD0: ptr+=1; if(verbose) printf("channel after-touch\n"); break;
			case 0xE0: ptr+=2; if(verbose) printf("pitch wheel change\n"); break;
			case 0xF0:
				//those immature meta data tags
				if(event==0xFF) {
					if(verbose) printf("meta\n");
					uint32_t meta_length; int j;
					char meta_event=*((char*)ptr);
					ptr++;
					meta_length=read_var_len((char**)&ptr);
					
					if(meta_event>=1 && meta_event<=5) {
						if(verbose) printf("Text: ");
						for(j=0;j<meta_length;j++) {
							putchar(*((char*)(ptr+j)));
						}
						putchar('\n');
					} else if(meta_event==0x51) {
						*tempo=((float)((*((unsigned char*)ptr))<<16|(*((unsigned char*)ptr+1))<<8|(*((unsigned char*)ptr+2))))/1000;
						if(verbose) printf("Tempo change: %f\n", *tempo);
					}
					
					ptr+=meta_length;
				} else {
					if(verbose) printf("sysex\n");
					ptr+=read_var_len((char**)&ptr)+1;
				}
				
			break;
		}
	}
}

void midi_free(struct midi_file_t* midi_file) {
	int i;
	for(i=0;i<midi_file->header.track_count;i++) {
		free(midi_file->tracks[i]->data);
		free(midi_file->tracks[i]);
	}
	free(midi_file->tracks);
}

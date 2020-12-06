#ifndef _SOUNDER_BLASTER_H
#define _SOUNDER_BLASTER_H

#include "types.h"

#define DSP_MIXER_PORT			0x224
#define DSP_MIXER_DATA_PORT		0x225
#define DSP_RESET				0x226
#define DSP_READ				0x22A
#define DSP_WRITE				0x22C
#define DSP_READ_STATUS			0x22E	// DSP Read Status (Read this port to acknowledge 8-bit interrupt)
#define DSP_ACK					0x22F	// DSP 16-bit Interrupt Acknowledge (Read this port to acknowledge 16-bit interrupt) (DSP Version 4.0+ Only)

// DSP WRITE COMMANDS
#define SET_TIME_CONST			0x40	// Set time constant	8 bit value
#define SET_OUT_SAMP_RATE		0x41	// Set Output Sample Rate	high bit/low bit
#define SPEAKER_ON				0xD1	// Turn speaker on	
#define SPEAKER_OFF				0xD3	// Turn speaker off	
#define STOP_PLAY_8				0xD0	// Stop playing 8 bit channel	
#define RESUME_PLAY_8			0xD4	// Resume playback of 8 bit channel	
#define STOP_PLAY_16			0xD5	// Stop playing 16 bit channel	
#define RESUME_PLAY_16 			0xD6	// Resume playback of 16 bit channel	
#define GET_DSP_VER 			0xE1	// Get DSP version	major version/minor version

// MIXER PORT COMMANDS
#define MASTER_VOL 				0x22	// Master volume	0xLR L=left volume R=right volume min=0x0 max=0xF (default value is 0xCC or 0x11)
#define SET_IRQ 				0x80	// Set IRQ	See below
#define SB_IRQ5 				0x02

#define SB_IRQ 					5

#define WAV_DATA_PG_ADDR        0xC0000  // wav data page at physical/virtual 0xC0000    

#define PHYS_PG_BDY             0x8000  // 64k physical page boundary for sound data buffer 

#define WAV_SAMPLE_RATE_OFFSET  24      // 24 bytes to sample rate
#define WAV_DATA_SIZE_OFFSET    40      // 40 bytes to data size in bytes
#define WAV_DATA_OFFSET         44      // 44 bytes of info before actual sound data

typedef struct wav_head{
	uint32_t chunkID;
	uint32_t chunk_size;
	uint32_t format;
	uint32_t subchunck1ID;
	uint32_t subchunck1size;
	uint16_t audio_format;
	uint16_t num_channels;
	uint32_t sample_rate;
	uint32_t byte_rate;
	uint16_t block_align;
	uint16_t bits_per_sample;
	uint32_t subchunck2ID;
	uint32_t subchunck2size;
} wave_head_t;

void __init_sb__();
void handle_sb_interrupt();

void play_sound();
void reset_dsp();
void set_sb_irq();

void set_irq();
void speaker_on();
void speaker_off();
void resume_playback();
void read_wav_data(char * fname);
void set_master_volume();
void dma_transfer();
void program_sb();
void set_sampling_rate();

#endif /* _SOUNDER_BLASTER_H */

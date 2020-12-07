#include "sound_blaster.h"
#include "i8259.h"
#include "lib.h"
#include "filesystem.h"
#include "paging.h"

wave_head_t curr_wav;
dentry_t wav_dentry;
uint8_t* audio_addr = (uint8_t*)WAV_DATA_PG_ADDR;
uint8_t interrupt_count;
int32_t offset;
int32_t curr_chunk_size;

void __init_sb__(){
	reset_dsp();
	set_sb_irq(); 
	enable_irq(SB_IRQ);
	interrupt_count = 0;
	offset = 0;
	curr_chunk_size = 0;
}

void play_sound(char * fname) {
	reset_dsp();
	if(read_wav_data(fname) == -1){
		return;
	}
	set_master_volume();
	speaker_on();
	dma_transfer();
	program_sb();
}

void handle_sb_interrupt() {
	inb(DSP_READ_STATUS);
	printf("SB got an interrupt lol\n");
	//resume_playback();
	memset((uint8_t*)WAV_DATA_PG_ADDR, 0, MAX_CHUNK_SIZE);	// clear sound data in memory (wava data page)
	if(offset == curr_wav.subchunck2size){
		stop_playback();
		speaker_off();
		offset = 0;
		curr_chunk_size = 0;
	}
	else{
		curr_chunk_size = (curr_wav.subchunck2size - offset > MAX_CHUNK_SIZE) ? MAX_CHUNK_SIZE : curr_wav.subchunck2size - offset;
		offset += read_data(wav_dentry.inode, WAV_DATA_OFFSET + offset, audio_addr, curr_chunk_size);
		resume_playback();
	}
	send_eoi(SB_IRQ);
	sti();
}

void reset_dsp(){
	int i;

	outb(0x1,	DSP_RESET);						// Send 1 to DSP reset port
	
	for(i = 0; i < 20000; i++); 				// software delay

	// Send 0 to DSP reset port
	outb(0x0, 	DSP_RESET);
	while (inb(DSP_READ) != 0xAA);			 // poll read_data port until AA received
}

int read_wav_data(char * fname){
	int num_read;

	// get the dentry information
	if(read_dentry_by_name((uint8_t*)fname, &wav_dentry) == -1){
		return -1;
	}

	read_data(wav_dentry.inode, 0, (uint8_t*)&curr_wav, WAV_DATA_OFFSET);

	curr_chunk_size = (curr_wav.subchunck2size - offset > MAX_CHUNK_SIZE) ? MAX_CHUNK_SIZE : curr_wav.subchunck2size - offset;

	if(-1 == (num_read = read_data(wav_dentry.inode, WAV_DATA_OFFSET + offset, audio_addr, curr_chunk_size))){
		return -1;
	}

	offset += num_read;

	return 0;
}

void set_master_volume() {
	outb(0x22, DSP_MIXER_PORT);		 	// 0x22 to mixer port
	outb(0xCC, DSP_MIXER_DATA_PORT);		// 0x11 (0xLR, L = left volume, R = right volume, min = 0x0, max = 0xF) to Mixer data port
}

void speaker_on(){
	outb(SPEAKER_ON, DSP_WRITE);
}

void dma_transfer() {
	uint8_t length_high_byte = (uint8_t)((curr_chunk_size >> 8) & 0x000000FF);
	uint8_t length_low_byte = (uint8_t)(curr_chunk_size & 0x000000FF);

	if(curr_wav.bits_per_sample == 8){
		// DMA channel 1
		outb(0x05,  0x0A);									// disable channel 1 (number of channel + 0x04)
		outb(1,	 	0x0C);									// flip flop port
		outb(0x59,  0x0B);									// set transfer mode (0x48 for single mode + channel number)
		outb((uint8_t)(WAV_DATA_PG_ADDR & 0x00FF),	0x02);	// send POSITION LOW BIT (WAV DATA POSITION IN MEMORY 0x0C00[00])
		outb((uint8_t)(WAV_DATA_PG_ADDR >> 8),  	0x02);	// send POSITON HIGH BIT (WAV DATA POSITION IN MEMORY 0x0C[00]00)
		outb(1,	 	0x0C);									// debug....................................................................................................
		outb(length_low_byte,  	0x03);						// send low byte of length of data (Example: size of 4MB page 0x04[00] bytes)
		outb(length_high_byte,  0x03);						// send high byte of length of data (Example: size of 4MB page 0x[04]00 bytes) 
		outb((uint8_t)(WAV_DATA_PG_ADDR >> 16),  	0x83);	// send page number to page port of channel 1 (WAV DATA POSITION IN wav_data_page addr 0x[0C]0000)
		outb(1,	 	0x0A);									// enable channel 1
	}
	else if(curr_wav.bits_per_sample == 16){
		// DMA channel 5
		outb(0x05,  0x0A);									// disable channel 1 (number of channel + 0x04)
		outb(1,	 	0xD8);									// flip flop port
		outb(0x59,  0xD6);									// set transfer mode (0x48 for single mode + channel number)
		outb((uint8_t)(WAV_DATA_PG_ADDR & 0x00FF),	0xC4);	// send POSITION LOW BIT (WAV DATA POSITION IN MEMORY 0x0C00[00])
		outb((uint8_t)(WAV_DATA_PG_ADDR >> 8),  	0xC4);	// send POSITON HIGH BIT (WAV DATA POSITION IN MEMORY 0x0C[00]00)
		outb(1,	 	0xD8);									// debug....................................................................................................
		outb(length_low_byte,  	0xC6);						// send low byte of length of data (Example: size of 4MB page 0x04[00] bytes)
		outb(length_high_byte,  0xC6);						// send high byte of length of data (Example: size of 4MB page 0x[04]00 bytes) 
		outb((uint8_t)(WAV_DATA_PG_ADDR >> 16),  	0x8B);	// send page number to page port of channel 1 (WAV DATA POSITION IN wav_data_page addr 0x[0C]0000)
		outb(1,	 	0xD4);									// enable channel 1
	}
 
}

void program_sb() {
	uint8_t count_high_byte = (uint8_t)(((curr_chunk_size - 1) >> 8) & 0x000000FF);
	uint8_t count_low_byte = (uint8_t)((curr_chunk_size - 1) & 0x000000FF);
	uint8_t audio_type = curr_wav.num_channels == 1 ? MONO : STEREO;
	audio_type |= curr_wav.bits_per_sample == 8 ? UNSIGNED : SIGNED;
	uint8_t transfer_mode = curr_wav.bits_per_sample == 8 ? TRANSFER_8 : TRANSFER_16;

	//outb(0x40,	DSP_WRITE);				//  ;set time constant..................................................
	//outb(165,	DSP_WRITE);					//  ;set output sample rate		10989 Hz...........................
	set_sampling_rate();
	outb(transfer_mode,		DSP_WRITE);		// write 8 bit transfer mode
	outb(audio_type,		DSP_WRITE);		// write type of sound data - mono and unsigned sound data
	outb(count_low_byte,	DSP_WRITE);		// COUNT LOW BYTE (Example: wav data length-1 = 0x03[FF])
	outb(count_high_byte,	DSP_WRITE);		// COUNT HIGH BYTE (Example: wav data length-1 = 0x[03]FF)
}

void set_sampling_rate() {
	uint8_t rate_high_byte = (uint8_t)((curr_wav.sample_rate >> 8) & 0x000000FF);
	uint8_t rate_low_byte = (uint8_t)(curr_wav.sample_rate & 0x000000FF);

	outb(0x41,  DSP_WRITE);
	outb(rate_high_byte, DSP_WRITE);
	outb(rate_low_byte, DSP_WRITE);
}



void set_sb_irq(){
	outb(SET_IRQ, DSP_MIXER_PORT);		 	// 0x80 to mixer port
	outb(SB_IRQ5, DSP_MIXER_DATA_PORT);		// 0x02 for IRQ 5 value to Mixer data port
}

void speaker_off(){
	outb(SPEAKER_OFF, DSP_WRITE);
}

void resume_playback(){
	if(curr_wav.bits_per_sample == 8){
		outb(RESUME_PLAY_8, DSP_WRITE);
	}
	else if(curr_wav.bits_per_sample == 16){
		outb(RESUME_PLAY_16, DSP_WRITE);
	}
}

void stop_playback(){
	if(curr_wav.bits_per_sample == 8){
		outb(STOP_PLAY_8, DSP_WRITE);
	}
	else if(curr_wav.bits_per_sample == 16){
		outb(STOP_PLAY_16, DSP_WRITE);
	}
}



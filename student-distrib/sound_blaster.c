#include "sound_blaster.h"
#include "i8259.h"
#include "lib.h"
#include "filesystem.h"
#include "paging.h"

wave_head_t curr_wav;
uint8_t* audio_addr = (uint8_t*)WAV_DATA_PG_ADDR;

void __init_sb__(){
    reset_dsp();
    //set_sb_irq(); 
	enable_irq(SB_IRQ);   
}

void play_sound(char * fname) {
    reset_dsp();
    read_wav_data(fname);
    set_master_volume();
    speaker_on();
    dma_transfer();
    program_sb();
}

void handle_sb_interrupt() {
    //Program DMA controller for next block............................................
    //Program DSP for next block...........................................................
    inb(DSP_READ_STATUS);
    // printf("SB got an interrupt lol\n");
	//resume_playback();
    memset((uint8_t *)WAV_DATA_PG_ADDR, 0, curr_wav.subchunck2size);       // clear sound data in memory (wava data page)
    //speaker_off();
	send_eoi(SB_IRQ);
	sti();
}

void reset_dsp(){
	int i;

	outb(0x1,	DSP_RESET);        			// Send 1 to DSP reset port
	
	for(i = 0; i < 10000; i++);

	// asm volatile(							// Wait 3 microseconds
    //    	"mov $0x86, 		%%ah;"
	// 	"mov $0x0000, 		%%cx;"
	// 	"mov $0xFFFF, 		%%dx;"
	// 	"int $0x15;"
	// 	: 
	// 	:
	// 	:"ah", "cx", "dx"
	// );

	// Send 0 to DSP reset port
	outb(0x0, 	DSP_RESET);

    //while (!(inb(DSP_READ_STATUS) & 0x40));  // poll read-buffer status port until bit-7 set

    while (inb(DSP_READ) != 0xAA);             // poll read_data port until AA received
}

void read_wav_data(char * fname){
	dentry_t dentry;

	// get the dentry information
	read_dentry_by_name((uint8_t*)fname, &dentry);

	read_data(dentry.inode, 0, (uint8_t*)&curr_wav, WAV_DATA_OFFSET);

	uint8_t wav_buf[curr_wav.subchunck2size];

	// read out the file's sound data into the wav data page
	read_data(dentry.inode, WAV_DATA_OFFSET, wav_buf, curr_wav.subchunck2size);

	memcpy((void*)audio_addr, (void*)wav_buf, curr_wav.subchunck2size);

	printf("%d, %d\n", wav_buf[0], ((uint8_t*)audio_addr)[0]);
}

void set_master_volume() {
    outb(0x22, DSP_MIXER_PORT);         	// 0x22 to mixer port
    outb(0xCC, DSP_MIXER_DATA_PORT);    	// 0x11 (0xLR, L = left volume, R = right volume, min = 0x0, max = 0xF) to Mixer data port
}

void speaker_on(){
	outb(SPEAKER_ON, DSP_WRITE);
}

void dma_transfer() {
    uint8_t length_high_byte = (uint8_t)((curr_wav.subchunck2size >> 8) & 0x000000FF);
    uint8_t length_low_byte = (uint8_t)(curr_wav.subchunck2size & 0x000000FF);

    // DMA channel 1
	outb(0x05,  0x0A);									// disable channel 1 (number of channel + 0x04)
	outb(1,     0x0C);									// flip flop port
	outb(0x59,  0x0B);									// set transfer mode (0x48 for single mode + channel number)
	outb((uint8_t)(WAV_DATA_PG_ADDR & 0x00FF),0x02);	// send POSITION LOW BIT (WAV DATA POSITION IN MEMORY 0x0C00[00])
	outb((uint8_t)(WAV_DATA_PG_ADDR >> 8),  0x02);		// send POSITON HIGH BIT (WAV DATA POSITION IN MEMORY 0x0C[00]00)
	outb(1,     0x0C);									// debug....................................................................................................
	outb(length_low_byte,  0x03);						// send low byte of length of data (Example: size of 4MB page 0x04[00] bytes)
	outb(length_high_byte,  0x03);						// send high byte of length of data (Example: size of 4MB page 0x[04]00 bytes) 
	outb((uint8_t)(WAV_DATA_PG_ADDR >> 16),  0x83);		// send page number to page port of channel 1 (WAV DATA POSITION IN wav_data_page addr 0x[0C]0000)
	outb(1,     0x0A);									// enable channel 1
 
}

void program_sb() {
    uint8_t count_high_byte = (uint8_t)(((curr_wav.subchunck2size-1) >> 8) & 0x000000FF);
    uint8_t count_low_byte = (uint8_t)((curr_wav.subchunck2size-1) & 0x000000FF);

    //outb(0x40,	DSP_WRITE);					//  ;set time constant..................................................
	//outb(165,	DSP_WRITE);					//  ;set output sample rate        10989 Hz...........................
    set_sampling_rate();
	outb(0xC2,	DSP_WRITE);					// write 8 bit transfer mode
	outb(0x00,	DSP_WRITE);					// write type of sound data - mono and unsigned sound data
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
    outb(SET_IRQ, DSP_MIXER_PORT);         	// 0x80 to mixer port
    outb(SB_IRQ5, DSP_MIXER_DATA_PORT);    	// 0x02 for IRQ 5 value to Mixer data port
}

void speaker_off(){
	outb(SPEAKER_OFF, DSP_WRITE);
}

void resume_playback(){
	outb(RESUME_PLAY_8, DSP_WRITE);
}

void stop_audio() {
    outb(STOP_PLAY_8, DSP_WRITE);
}



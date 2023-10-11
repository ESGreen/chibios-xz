#pragma once
#ifndef MIC_H
#define MIC_H

//#define NUM_RX_SAMPLES 512

//-0- These were bunnies og values
//#define NUM_RX_SAMPLES 256
//#define NUM_RX_BLOCKS 4

//-0- This is what I'm testing out
#define NUM_RX_SAMPLES 256
#define NUM_RX_BLOCKS 4

#define MIC_SAMPLE_DEPTH 128

#define MAX_CLIPS 6

void analogUpdateTemperature(void);
int32_t analogReadTemperature(void);

void analogUpdateMic(void);

//- Originally the audio sample type was an int but now we going float cause fft!
//- If we good and i mean really good then we use f16
#ifdef AUDIO_AS_INT
typedef int16_t Audio_Sample_Type;
#else
typedef float Audio_Sample_Type;
#endif

//! Get the current audio frame
/*!   \return  Audio sample
 */
Audio_Sample_Type* analogReadMic (void);

void micStart(void);

#define DBLOGLEN 8
extern uint8_t dblog[];
extern uint8_t dblogptr;

//extern int32_t rx_samples[];
extern int32_t rx_samples[];
extern Audio_Sample_Type rx_savebuf[];

extern uint32_t rx_int_count;
extern uint32_t rx_handler_count;
extern uint8_t gen_mic_event;


#endif /* MIC_H */

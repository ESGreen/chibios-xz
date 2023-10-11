#pragma once

#ifndef BTrackAdapter_H
#define BTrackAdapter_H

/* **********************************************************************************************************************
 *   BTrackAdapter   this is a piece that will allow me to make calls to BTrack but not having to worry about c/c++ 
 *                   conversions
 * *********************************************************************************************************************/


//! Initalize the btrack object
/*!   \param[in]  hopSize  the hop size is the amount of unique info in a sample. frame size - hop size = the overlap
 *    \param[in]  frameSize  the size of each frame. Might not be used and just be 1024
 */
int initalizeAudio (void);

//! Process a new frame of data
void processAudioFrame (void);

//! We got the beat
int gotBeat (void);

#endif

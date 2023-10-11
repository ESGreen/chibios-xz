#include "AudioProcessor.h"

#include <float.h>
#include <stdint.h>
#include <stddef.h>

#include "hal.h"
#include "shell.h"
#include "orchard.h"
#include "chprintf.h"
#include "mic.h"
#include "led.h"




//-- These are function so that kiss doesn't need to link against malloc and free cause they dont work so well
void* dummyMalloc (size_t size) { (void) size; return NULL; }
void dummyFree (void* ptr) { (void) ptr; }

//- The declerations of the dummy malloc and dummy free need to happen
//  before the inclusion of kiss fft
#include "kissfft/kiss_fftr.h"

//-- Look this is ugly as heck but I need to have memory of the correct size but there is no room in chibi for
//-- an allocator. Got this by digging through the kiss fft source code confirmed it was the right size just going to live with the mess
// FWIW: 256 Samples = 2836 bytes. Ouch!
#define nfft_ (NUM_RX_SAMPLES >> 1)
#define subsize_ (KISS_FFT_ALIGN_SIZE_UP (296 + sizeof(kiss_fft_cpx) * (nfft_ - 1)))
#define memNeeded_  (16 + subsize_ + sizeof (kiss_fft_cpx) * (nfft_ * 3 / 2))

uint8_t kissMemory[memNeeded_] = {0};
static kiss_fftr_cfg fftMachine_ = NULL;

#define FREQUENCY_COUNT (NUM_RX_SAMPLES/2 + 1)
static kiss_fft_cpx frequencyData[FREQUENCY_COUNT];


//- This is how many magnitude samples that are tracked
#define MAGNITUDE_SAMPLE_COUNT 5

// Struct to track all the data in this file
typedef struct __BeatData {
   float current_;
   float total_;
   float average_;
   float magnitude_[MAGNITUDE_SAMPLE_COUNT];
} BeatData;

// should be 0, but first 2 bands produce too much noise
#define OVERALL_START 2
#define OVERALL_END FREQUENCY_COUNT
#define OVERALL_RANGE (OVERALL_END - OVERALL_START)

#define FIRST_START 2
#define FIRST_END 4
#define FIRST_RANGE (FIRST_END - FIRST_START)
#define SECOND_START 2
#define SECOND_END 6
#define SECOND_RANGE (SECOND_END - SECOND_START)

static BeatData signal_ = { 0.0f, 0.0f, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f, 0.0f} };
static BeatData overall_ = { 0.0f, 0.0f, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f, 0.0f} };
static BeatData firstFrequency_ = { 0.0f, 0.0f, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f, 0.0f} };
static BeatData secondFrequency_ = { 0.0f, 0.0f, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f, 0.0f} };


static systime_t lastBeatTimestamp_ = 0;
static systime_t durationSinceLastBeat_ = 0;

static int beat_ = 0;

//! Process all the sweeps over the data that collects all the stats
static void processSweeps (uint8_t index);

//- This code borrows/copies heavily from https://github.com/Steppschuh/Micro-Beat-Detection
//- There are better beat detection mechanisms but they are all way more time to code up and
//- way more memory than this thing has. This beat detector works by splitting the sound into
//- frequency bins and then looking at the lowest bins. Snapping fingers is a good test if you
//- lack speakers with decent sound output (burning man has plenty of these). 



//! Process the frequency data and put into the beatdata data struct
/*!    \param[in]  data   where you want it
 *     \param[in]  start  the bin to start in the frequency list
 *     \param[in]  end    the last frequency bin to look through
 *     \param[in]  index  the index to throw all the data into on the beat data
 */
static void processFrequencyData (BeatData* data, int start, int end, int index);

//! process history values
/*!   \param[in] 
 */
static void processHistoryValues (BeatData* data, int index);

//! Get the frequency magnitude of a given range.
/*!   \return  the magnituyde over a given range 
 *    \param[in]  data        the data over which to search
 *    \param[in]  startIndex  where in the range you want to start
 *    \param[in]  endIndex    where you want to stop the calculation
 */
static float getFrequencyMagnitude (kiss_fft_cpx* data, int startIndex, int endIndex);



//! Will update the beat probability, a value between 0 and 1 indicating how likely it is that there's a beat right now.
/*!   \return  float  the beat frequency [0-1]
 */
static float updateBeatProbability (void);


//! Will calculate a value in range [0:2] based on the magnitude changes of different frequency bands.
//! Low values are indicating a low beat probability.
static float calculateSignalChangeFactor (void);

//! Will calculate a value in range [0:1] based on the magnitude changes of different frequency bands.
//! Low values are indicating a low beat probability.
static float calculateMagnitudeChangeFactor (systime_t durationSinceLastBeat);

//! Will calculate a value in range [0:1] based on the recency of the last detected beat. Low values
//! are indicating a low beat probability.
static float calculateRecencyFactor (systime_t* durationSinceLastBeat);

//! Make sure a value value in is between min and max
/*!   \return  the constrained value
 *    \param[in]  vlaue  the value to contrain
 *    \param[in]  minimum  the mnimum the vcalue can be 
 *    \param[in]  maximum  the maximum value can be
 */
static float constrain (float value, float minimum, float maximum);





// ***************************************************************************************************
// ***************************************************************************************************
// ***************************************************************************************************





// ***************************************************************************************************
int initalizeAudio (void)
{
   //arm_status status = arm_rfft_fast_init_f32 (&fftMachine_, NUM_RX_SAMPLES);
   //return status == ARM_MATH_SUCCESS;
   size_t sizeOfMemory = memNeeded_;
   fftMachine_ = kiss_fftr_alloc (NUM_RX_SAMPLES, 0, (void*) kissMemory, &sizeOfMemory);

   // @2836 We are allocating ennough memory. But I want to check
   //chprintf (stream, "Kiss Returns Pointer:%d == %d [%d]\r\n", (int) fftMachine_, (int) kissMemory, (int) sizeOfMemory);
   return 0;
}

extern uint32_t radioSample_;

// ***************************************************************************************************
void processAudioFrame ()
{
   static uint8_t sampleIndex_ = 0;
   
   // 0) Not doing nothing if there is no machine to do it
   if (fftMachine_ == NULL)
      return;
   
   // 1) Go get the audio frame
   Audio_Sample_Type* audioFrame = analogReadMic ();

   // 2) Lets process the samples getting some info on the
   float max = -1024.0f;
   float min = 1024.0f;
   float average = 0.0f;
   for (int i = 0; i < NUM_RX_SAMPLES; ++i)
   {
      max = max > audioFrame[i] ? max : audioFrame[i];
      min = min < audioFrame[i] ? min : audioFrame[i];
      average += audioFrame[i];
   }

   average = average / (float) NUM_RX_SAMPLES;
   float signalDelta = max - average;
   // chprintf (stream, "Audio Stream: SAM: 0x%08x   MAX: %.3f  MIN: %.3f   AVERAGE: %.3f  DELTA: %.3f\r\n",
   //           radioSample_, max, min, average, signalDelta);
   signal_.current_ = average + (2*signalDelta);
   signal_.current_ = constrain (signal_.current_, 0, max);

   processHistoryValues (&signal_, sampleIndex_);

   // 3) Now get the fft
   kiss_fftr (fftMachine_, audioFrame, frequencyData);

   // 4) Caculate the abs of the fft
   for (int i = 0; i < FREQUENCY_COUNT; ++i)
      frequencyData[i].r = sqrtf (frequencyData[i].r * frequencyData[i].r + frequencyData[i].i * frequencyData[i].i);

   // 5) now capture all the history 
   processSweeps (sampleIndex_);
   updateBeatProbability ();
   if (durationSinceLastBeat_ < 200)
   {
      int shift = getShiftCeiling () - 3;
      if (shift < 0)
         shift = 0;
      setBeatShift (shift);
   }
   else
      setBeatShift (getShiftCeiling ());

   // 6) Update the index
   // prepare the magnitude sample index for the next update
   sampleIndex_++;
   sampleIndex_ = sampleIndex_ >= MAGNITUDE_SAMPLE_COUNT ? 0 : sampleIndex_;
}

// ***************************************************************************************************
int gotBeat (void)
{
   return beat_;
}






/**
 * Will extract insightful features from the frequency data in order
 * to perform the beat detection.
 */
// ***************************************************************************************************
static void processSweeps (uint8_t index)
{   
   // each of the methods below will:
   //  - get the current frequency magnitude
   //  - add the current magnitude to the history
   //  - update relevant features
   processFrequencyData (&overall_, OVERALL_START, OVERALL_END, index);
   processFrequencyData (&firstFrequency_, FIRST_START, FIRST_END, index);
   processFrequencyData (&secondFrequency_, SECOND_START, SECOND_END, index);
}

// ***************************************************************************************************
static void processFrequencyData (BeatData* data, int start, int end, int index)
{
   data-> current_ = getFrequencyMagnitude (frequencyData, start, end);
   processHistoryValues (data, index);
}

// ***************************************************************************************************
static float getFrequencyMagnitude (kiss_fft_cpx* data, int startIndex, int endIndex)
{
   // 1) Calculate the total over the frequencies
   float total = 0.0f;
  
   for (int i = startIndex; i < endIndex; i++)
      total += data[i].r;

   // 2) Now get the average magnitude
   float average = total / (endIndex - startIndex);
   return average;
}

// ***************************************************************************************************
static void processHistoryValues (BeatData* data, int index)
{
   data-> total_ -= data-> magnitude_[index]; // subtract the oldest history value from the total
   data-> total_ += data-> current_;         // add the current value to the total
   data-> magnitude_[index] = data-> current_; // add the current value to the history
  
   data-> average_ = data-> total_ / (float) MAGNITUDE_SAMPLE_COUNT;
  }

// ***************************************************************************************************
static float constrain (float value, float minimum, float maximum)
{
   if (value < minimum)
      return minimum;
   else if (value > maximum)
      return maximum;
   return value;
}

// ***************************************************************************************************
float updateBeatProbability (void)
{
   //durationSinceLastBeat = chVTTimeElapsedSinceX (lastBeatTimestamp_);
   //chprintf (stream, "Updating Beat Probability\n\r");
   
   float beatProbability = 1.0f;
   static const float beatProbabilityThreshold = 0.5;


   beatProbability *= calculateSignalChangeFactor ();
   beatProbability *= calculateMagnitudeChangeFactor (durationSinceLastBeat_);
   beatProbability *= calculateRecencyFactor (&durationSinceLastBeat_);
  
   if (beatProbability >= beatProbabilityThreshold)
   {
      lastBeatTimestamp_ = chVTGetSystemTime ();
      durationSinceLastBeat_ = 0;
   }

   // chprintf (stream, "Beat Probability: %.3f\n\r", beatProbability);
   return beatProbability;
}

// ***************************************************************************************************
float calculateSignalChangeFactor ()
{
#define averageSignalThreshold_ (75.0f / 512.0f - 1.0f)
#define currentSignalThreshold_ (150.0f / 512.0f - 1.0f)
   float aboveAverageSignalFactor = 0.0f;
   if (signal_.average_ < averageSignalThreshold_ || signal_.current_ < currentSignalThreshold_)
      aboveAverageSignalFactor = 0.0f;
   else
   {
      aboveAverageSignalFactor = signal_.current_ / signal_.average_;
      aboveAverageSignalFactor = constrain (aboveAverageSignalFactor, 0.0f, 2.0f);
   }

   // chprintf (stream, "\tCalculate Signal Change Factor: %.3f\n\r", aboveAverageSignalFactor);
   return aboveAverageSignalFactor;
#undef averageSignalThreshold_
#undef currentSignalThreshold_
}

// ***************************************************************************************************
float calculateMagnitudeChangeFactor (systime_t durationSinceLastBeat)
{
   float changeThresholdFactor = 1.1f;
   if (durationSinceLastBeat < 750)
      // attempt to not miss consecutive beats
      changeThresholdFactor *= 0.95;
   else if (durationSinceLastBeat > 1000) 
      // reduce false-positives
      changeThresholdFactor *= 1.05;
  
   // current overall magnitude is higher than the average, probably 
   // because the signal is mainly noise
   float aboveAverageOverallMagnitudeFactor = overall_.current_ / overall_.average_;
   aboveAverageOverallMagnitudeFactor -= 1.05f;
   aboveAverageOverallMagnitudeFactor *= 10.0f;
   aboveAverageOverallMagnitudeFactor = constrain (aboveAverageOverallMagnitudeFactor, 0.0f, 1.0f);
   aboveAverageOverallMagnitudeFactor -= 0.1f;
   // chprintf (stream, "\tAbove Average Overall Magnitude Factor: %.3f [%.3f/%.3f]\n\r",
   //           aboveAverageOverallMagnitudeFactor,
   //           overall_.current_,
   //           overall_.average_);

   // current magnitude is higher than the average, probably 
   // because the there's a beat right now
   float aboveAverageFirstMagnitudeFactor = firstFrequency_.current_ / firstFrequency_.average_;
   aboveAverageFirstMagnitudeFactor *= 1.5f;
   aboveAverageFirstMagnitudeFactor = powf (aboveAverageFirstMagnitudeFactor, 3);
   aboveAverageFirstMagnitudeFactor /= 3.0f;
   aboveAverageFirstMagnitudeFactor -= 1.25f;
   aboveAverageFirstMagnitudeFactor = constrain (aboveAverageFirstMagnitudeFactor, 0.0f, 1.0f);
   // chprintf (stream, "\tFirst Fq Factor: %.3f [%.3f / %.3f]\n\r",
   //           aboveAverageFirstMagnitudeFactor,
   //           firstFrequency_.current_,
   //           firstFrequency_.average_);
  
   float aboveAverageSecondMagnitudeFactor = secondFrequency_.current_ / secondFrequency_.average_;
   aboveAverageSecondMagnitudeFactor -= 1.01f;
   aboveAverageSecondMagnitudeFactor *= 10.0f;
   aboveAverageSecondMagnitudeFactor = constrain (aboveAverageSecondMagnitudeFactor, 0.0f, 1.0f);
   // chprintf (stream, "\tSecond Fq Factor: %.3f [%.3f / %.3f]\n\r",
   //           aboveAverageSecondMagnitudeFactor,
   //           secondFrequency_.current_,
   //           secondFrequency_.average_);
  
   float magnitudeChangeFactor = aboveAverageFirstMagnitudeFactor;
   if (magnitudeChangeFactor > 0.15)
   {
      magnitudeChangeFactor = aboveAverageFirstMagnitudeFactor > aboveAverageSecondMagnitudeFactor ?
         aboveAverageFirstMagnitudeFactor : aboveAverageSecondMagnitudeFactor;
   }
  
   if (magnitudeChangeFactor < 0.5
       && aboveAverageOverallMagnitudeFactor > 0.5)
   {
      // there's no bass related beat, but the overall magnitude changed significantly
      magnitudeChangeFactor = magnitudeChangeFactor > aboveAverageOverallMagnitudeFactor ?
         magnitudeChangeFactor : aboveAverageOverallMagnitudeFactor;
   }
  
   return magnitudeChangeFactor;
}

// ***************************************************************************************************
#define MAXIMUM_BEATS_PER_MINUTE 200
#define MINIMUM_DELAY_BETWEEN_BEATS (60000L / MAXIMUM_BEATS_PER_MINUTE)
// good value range is [50:150]
#define SINGLE_BEAT_DURATION 100
float calculateRecencyFactor (systime_t* durationSinceLastBeat)
{
   int referenceDuration = MINIMUM_DELAY_BETWEEN_BEATS - SINGLE_BEAT_DURATION;

   float recencyFactor = 1.0f;
   *durationSinceLastBeat = chVTTimeElapsedSinceX (lastBeatTimestamp_);
  
   recencyFactor = 1 - ((float) referenceDuration / (float) *durationSinceLastBeat);
   recencyFactor = constrain (recencyFactor, 0.0f, 1.0f);
  
   // chprintf (stream, "\tRecency Factor: %.3f\n\r", recencyFactor);
  
   return recencyFactor;
}
#undef SINGLE_BEAT_DURATION
#undef MINIMUM_DELAY_BETWEEN_BEATS
#undef MAXIMUM_BEATS_PER_MINUTE








#ifndef __LED_H__
#define __LED_H__

#include "hal.h"

#define sign(x) (( x > 0 ) - ( x < 0 ))

typedef struct Color Color;
struct Color {
  uint8_t g;
  uint8_t r;
  uint8_t b;
};

typedef struct HsvColor {
  uint8_t h;
  uint8_t s;
  uint8_t v;
} HsvColor;

typedef struct RgbColor {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} RgbColor;

// hardware configuration information
// max length is different from actual length because some
// pattens may want to support the option of user-added LED
// strips, whereas others will focus only on UI elements in the
// circle provided on the board itself
typedef struct led_config {
  uint8_t       *fb; // effects frame buffer
  uint8_t       *final_fb;  // merged ui + effects frame buffer
  uint32_t      pixel_count;  // generated pixel length
  uint32_t      max_pixels;   // maximal generation length
  uint8_t       *ui_fb; // frame buffer for UI effects
  uint32_t      ui_pixels;  // number of LEDs on the PCB itself for UI use
} LedConfig;


RgbColor HsvToRgb (HsvColor hsv);
HsvColor RgbToHsv(RgbColor rgb);
uint8_t gray_encode(uint8_t n);
uint8_t gray_decode(uint8_t n);

void ledStart(uint32_t leds, uint8_t *o_fb, uint32_t ui_leds, uint8_t *o_ui_fb);

void effectsStart(void);
uint8_t effectsStop(void);
extern uint8_t ledsOff;;

uint8_t effectsNameLookup(const char *name);
void effectsSetPattern(uint8_t);
uint8_t effectsGetPattern(void);
void bump(uint32_t amount);
void setShiftCeiling (uint8_t s);
uint8_t getShiftCeiling (void);

void setBeatShift (uint8_t beatShift);
uint8_t getBestShift (void);

void effectsNextPattern(int skipstrobe);
void effectsPrevPattern(int skipstrobe);

void uiLedGet(uint8_t index, Color *c);
void uiLedSet(uint8_t index, Color c);

//! Set a single led
/*!   \param[in]  ptr  the frame buffer
 *    \param[in]  x    the index of the led to change
 *    \param[in]  r    red
 *    \param[in]  g    green
 *    \param[in]  b    blue
 *    \param[in]  shift  how much to shift the leds. This makes them dimmer
 */
void ledSetRGB(void *ptr, int x, uint8_t r, uint8_t g, uint8_t b, uint8_t shift);

//! List out the effects
void listEffects(void);

const char *effectsCurName(void);
const char *lightgeneName(void);

void check_lightgene_hack(void);

#define EFFECTS_REDRAW_MS 35

#endif /* __LED_H__ */

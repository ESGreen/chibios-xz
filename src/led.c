#include "ch.h"
#include "hal.h"
#include "led.h"
#include "orchard-effects.h"
#include "gfx.h"

#include "chprintf.h"
#include "stdlib.h"
#include "orchard-math.h"
#include "fixmath.h"

#include "genes.h"
#include "storage.h"

#include "orchard-test.h"
#include "test-audit.h"

#include <string.h>
#include <math.h>

orchard_effects_end();

extern void ledUpdate(uint8_t *fb, uint32_t len);

void ledSetRGB(void *ptr, int x, uint8_t r, uint8_t g, uint8_t b, uint8_t shift);
//static void ledSetColor(void *ptr, int x, Color c, uint8_t shift);
// static void ledSetRGBClipped(void *fb, uint32_t i,
//                       uint8_t r, uint8_t g, uint8_t b, uint8_t shift);
// static Color ledGetColor(void *ptr, int x);


// global effects state
static effects_config fx_config;
static uint8_t fx_index = 0;  // current effect
static uint8_t fx_max = 0;    // max # of effects

static uint32_t bump_amount = 0;
static uint8_t bumped = 0;
static unsigned int bumptime = 0;
//static unsigned long reftime = 0;
//static unsigned long reftime_tau = 0;
//static unsigned long offset = 0;
//static unsigned int waverate = 10;
//static unsigned int waveloop = 0;
static unsigned int patternChanged = 0;
static uint32_t reftime_lg = 0;
static uint8_t sat_offset = 0;

//static int wavesign = -1;

static uint8_t ledExitRequest = 0;
static LedConfig led_config;


uint8_t ledsOff = 1;

genome diploid;   // not static so we can access/debug from other files

uint8_t effectsStop(void) {
  ledExitRequest = 1;
  return ledsOff;
}

/**
 * @brief   Initialize Led Driver
 * @details Initialize the Led Driver based on parameters.
 *
 * @param[in] leds      length of the LED chain controlled by each pin
 * @param[out] o_fb     initialized frame buffer
 *
 */
void ledStart(uint32_t leds, uint8_t *o_fb, uint32_t ui_leds, uint8_t *o_ui_fb)
{
  unsigned int j;
  led_config.max_pixels = leds;
  led_config.pixel_count = leds;
  led_config.ui_pixels = ui_leds;

  led_config.fb = o_fb;
  led_config.ui_fb = o_ui_fb;

  led_config.final_fb = chHeapAlloc( NULL, sizeof(uint8_t) * led_config.max_pixels * 3 );
  
  for (j = 0; j < leds * 3; j++)
    led_config.fb[j] = 0x0;
  for (j = 0; j < ui_leds * 3; j++)
    led_config.ui_fb[j] = 0x0;

  chSysLock();
  ledUpdate(led_config.fb, led_config.max_pixels);
  chSysUnlock();
}

void uiLedGet(uint8_t index, Color *c) {
  if( index >= led_config.ui_pixels )
    index = led_config.ui_pixels - 1;
  
  c->g = led_config.ui_fb[index*3];
  c->r = led_config.ui_fb[index*3+1];
  c->b = led_config.ui_fb[index*3+2];
}

void uiLedSet(uint8_t index, Color c) {
  if( index >= led_config.ui_pixels )
    index = led_config.ui_pixels - 1;
  
  led_config.ui_fb[index*3] = c.g;
  led_config.ui_fb[index*3+1] = c.r;
  led_config.ui_fb[index*3+2] = c.b;
}

// static void ledSetRGBClipped(void *fb, uint32_t i,
//                       uint8_t r, uint8_t g, uint8_t b, uint8_t shift) {
//   if (i >= led_config.pixel_count)
//     return;
//   ledSetRGB(fb, i, r, g, b, shift);
// }

void ledSetRGB(void *ptr, int x, uint8_t r, uint8_t g, uint8_t b, uint8_t shift) {
  uint8_t *buf = ((uint8_t *)ptr) + (3 * x);
  buf[0] = g >> shift;
  buf[1] = r >> shift;
  buf[2] = b >> shift;
}

// static void ledSetColor(void *ptr, int x, Color c, uint8_t shift) {
//   uint8_t *buf = ((uint8_t *)ptr) + (3 * x);
//   buf[0] = c.g >> shift;
//   buf[1] = c.r >> shift;
//   buf[2] = c.b >> shift;
// }

// static Color ledGetColor(void *ptr, int x) {
//   Color c;
//   uint8_t *buf = ((uint8_t *)ptr) + (3 * x);

//   c.g = buf[0];
//   c.r = buf[1];
//   c.b = buf[2];
  
//   return c;
// }

void ledSetCount(uint32_t count) {
  if (count > led_config.max_pixels)
    return;
  led_config.pixel_count = count;
}

static uint8_t shift_ = 3;  // start a little bit dimmer
void setShiftCeiling (uint8_t s) {
    shift_ = s;
}

uint8_t getShiftCeiling (void) {
    return shift_;
}

static uint8_t beatShift_ = 7;
void setBeatShift (uint8_t beatShift) {
   beatShift_ = beatShift;
}
uint8_t getBeatShift (void) {
   return shift_ < beatShift_ ? shift_ : beatShift_;
}

// alpha blend, scale the input color based on a value from 0-255. 255 is full-scale, 0 is black-out.
// uses fixed-point math.
Color alphaPix( Color c, uint8_t alpha ) {
  Color rc;
  uint32_t r, g, b;

  r = c.r * alpha;
  g = c.g * alpha;
  b = c.b * alpha;

  rc.r = (r / 255) & 0xFF;
  rc.g = (g / 255) & 0xFF;
  rc.b = (b / 255) & 0xFF;

  return( rc );  
}

// static Color Wheel(uint8_t wheelPos) {
//   Color c;

//   if (wheelPos < 85) {
//     c.r = wheelPos * 3;
//     c.g = 255 - wheelPos * 3;
//     c.b = 0;
//   }
//   else if (wheelPos < 170) {
//     wheelPos -= 85;
//     c.r = 255 - wheelPos * 3;
//     c.g = 0;
//     c.b = wheelPos * 3;
//   }
//   else {
//     wheelPos -= 170;
//     c.r = 0;
//     c.g = wheelPos * 3;
//     c.b = 255 - wheelPos * 3;
//   }
//   return c;
// }

static void do_lightgene(struct effects_config *config) {
  uint8_t *fb = config->hwconfig->fb;
  uint32_t count = config->count;
  uint32_t loop = config->loop & 0x1FF;
  uint32_t shoot;
  HsvColor hsvC;
  RgbColor rgbC;
  uint32_t i;
  uint32_t tau;
  uint32_t curtime, indextime;
  //float time, space;
  //float twopi;
  //float spacetime;
  fix16_t time, space;
  fix16_t twopi;
  fix16_t spacetime;
  uint8_t overrideHSV = 0;
  uint8_t overshift;
  uint32_t hue_rate;
  uint8_t hue_dir;
  uint32_t hue_temp;
  // diploid is static to this function and set when the lightgene is selected
  uint8_t shift = getBeatShift ();

    

  tau = (uint32_t) map(diploid.cd_rate, 0, 255, 700, 8000);
  curtime = chVTGetSystemTime();
  if( (curtime - reftime_lg) > tau )
    reftime_lg = curtime;
  indextime = reftime_lg - curtime;

  if( bumped ) {
    bumped = 0;
    sat_offset = satadd_8(sat_offset, map(diploid.accel, 0, 255, 0, 24));
  } else {
    if( (loop % 2) == 0 )  // cheesy make the time constant to baseline longer.
      sat_offset = satsub_8(sat_offset, 1);
  }
  for( i = 0; i < count; i++ ) {
    overrideHSV = 0;
    // compute one pixel's color
    // count is the current pixel index
    // loop is the current point in effect cycle, e.g. all effects loop on a 0-511 basis
    // hue chromosome
    hue_rate = (uint32_t) diploid.hue_ratedir & 0xF;
    hue_dir = (((diploid.hue_ratedir >> 4) & 0xF) > 10) ? 1 : 0;
    /*
      refactor: we want the pattern applied from 0-7 to be inversely applied from 8-15
      0 1 2 3 4 5 6 7  7 6 5 4 3 2 1 0
     */
    // 254L cheesily avoids rounding errors.
    if( !hue_dir ) {
      hue_temp = ((128L / (count / 2)) * i + (loop * hue_rate)) - 0L;
      hue_temp &= 0x1FF;
      if( hue_temp <= 0xFF )
	hsvC.h = (uint8_t) hue_temp;
      else {
	hsvC.h = (uint8_t) (511-hue_temp);
      }
    } else {
      hue_temp = ((128L / (count / 2)) * i - (loop * hue_rate)) - 0L;
      hue_temp &= 0x1FF;
      if( hue_temp <= 0xFF )
	hsvC.h = (uint8_t) hue_temp;
      else {
	hsvC.h = (uint8_t) (511-hue_temp);
      }
    }
    hsvC.h = map_16( (int16_t) hsvC.h, 0, 255,
		     (int16_t) diploid.hue_base, (int16_t) diploid.hue_bound );
    
    // chprintf( stream, "%d ", hsvC.h );

    // saturation chromosome
    hsvC.s = satadd_8(diploid.sat, sat_offset);

    // compute the value overlay
    // use cos b/c value is 1.0 when input is 0
    // value = 255 * cos( cd_period * 2pi * (i/(count-1))
    //                    +/- (indextime / tau(cd_rate)) * 2pi )
    // sign of rate is determined by cd_dir

    twopi = fix16_mul( fix16_from_int(2), fix16_pi );
    //twopi = 3.14159265359 * 2.0;
    // space = 2pi * diploid.cd_period * (i / (count-1))
        space = fix16_mul(twopi, fix16_mul( fix16_from_int(diploid.cd_period),
                fix16_div(fix16_from_int(i), fix16_from_int(count-1)) ));
	//space = twopi * diploid.cd_period * ((float)i / ((float)count - 1.0));

    // time = 2pi * (indextime) / tau
    time = fix16_mul(twopi, fix16_div( fix16_from_int(indextime), fix16_from_int(tau) ));
    //time = twopi * (float) indextime / (float) tau;

    //    time = fix16_from_int(0);
    
    // space +/- time based on direction
    if( diploid.cd_dir > 128 ) {
      spacetime = fix16_add( space, time );
      //spacetime = space + time;
    } else {
      spacetime = fix16_sub( space, time );
      //spacetime = space - time;
    }

    // hsv.v = 127 * (1 + cos(spacetime))
    hsvC.v = (uint8_t) fix16_to_int( fix16_mul( fix16_from_int(127),
						fix16_add( fix16_from_int(1),
							   fix16_cos(spacetime))));
    //hsvC.v = (uint8_t) 127.0 * (1.0 + cosf(spacetime));

    if( diploid.nonlin > 127 )
      // add some nonlinearity to gamma-correct brightness
      hsvC.v = (uint8_t) (((uint16_t) hsvC.v * (uint16_t) hsvC.v) >> 8 & 0xFF);

    // now compute lin effect, but only if the threshold is met
    if( diploid.lin < 90 ) {  // rare variant after a summing expression ~3% chance
      shoot = loop % count;
      if( shoot == i ) {
	overrideHSV = 1;
      }
    }

    // now compute strobe effect, but only if the threshold is met
    if( diploid.strobe < 10 ) {
      // for now, do nothing...this one is a pain in the ass to implement and probably not too interesting anyways
    }

    // go from RGB to HSV for a particular pixel
    if( !overrideHSV ) {
      rgbC = HsvToRgb(hsvC);
      ledSetRGB(fb, i, rgbC.r, rgbC.g, rgbC.b, shift);
    } else {
      overshift = shift - 2; // make this effect brighter so it's obvious
      if( overshift > 4 )
	overshift = 4;
      ledSetRGB(fb, i, 255, 255, 255, overshift);
    }
  }
}

static void lg0FB(struct effects_config *config) {
  do_lightgene(config);
}
orchard_effects("Lg0", lg0FB);

static void lg1FB(struct effects_config *config) {
  do_lightgene(config);
}
orchard_effects("Lg1", lg1FB);

static void lg2FB(struct effects_config *config) {
  do_lightgene(config);
}
orchard_effects("Lg2", lg2FB);

static void lg3FB(struct effects_config *config) {
  do_lightgene(config);
}
orchard_effects("Lg3", lg3FB);

static void lg4FB(struct effects_config *config) {
  do_lightgene(config);
}
orchard_effects("Lg4", lg4FB);


static void strobePatternFB(struct effects_config *config) {
  uint8_t *fb = config->hwconfig->fb;
  int count = config->count;
  
  uint16_t i;
  static uint32_t  nexttime = 0;
  static uint8_t   strobemode = 1;
  
  uint8_t shift = 0;

  if( strobemode && (chVTGetSystemTime() > nexttime) ) {
    for( i = 0; i < count; i++ ) {
      if( (rand() % (unsigned int) count) < ((unsigned int) count / 3) )
	ledSetRGB(fb, i, 255, 255, 255, shift);
      else
	ledSetRGB(fb, i, 0, 0, 0, shift);
    }

    nexttime = chVTGetSystemTime() + 30 + (rand() % 25);
    strobemode = 0;
  }

  else if( !strobemode && (chVTGetSystemTime() > nexttime) ) {
    for( i = 0; i < count; i++ ) {
      ledSetRGB(fb, i, 0, 0, 0, shift);
    }
    
    nexttime = chVTGetSystemTime() + 30 + (rand() % 25);
    strobemode = 1;
  }
}
orchard_effects("strobe", strobePatternFB);


static void testPatternFB(struct effects_config *config) {
  uint8_t *fb = config->hwconfig->fb;
  int count = config->count;
  int loop = config->loop;
  
  int i = 0;

// #if 0
//   while (i < count) {
//     /* Black */
//     ledSetRGB(fb, (i + loop) % count, 0, 0, 0, shift);
//     if (++i >= count) break;

//     /* Red */
//     ledSetRGB(fb, (i + loop) % count, 255, 0, 0, shift);
//     if (++i >= count) break;

//     /* Yellow */
//     ledSetRGB(fb, (i + loop) % count, 255, 255, 0, shift);
//     if (++i >= count) break;

//     /* Green */
//     ledSetRGB(fb, (i + loop) % count, 0, 255, 0, shift);
//     if (++i >= count) break;

//     /* Cyan */
//     ledSetRGB(fb, (i + loop) % count, 0, 255, 255, shift);
//     if (++i >= count) break;

//     /* Blue */
//     ledSetRGB(fb, (i + loop) % count, 0, 0, 255, shift);
//     if (++i >= count) break;

//     /* Purple */
//     ledSetRGB(fb, (i + loop) % count, 255, 0, 255, shift);
//     if (++i >= count) break;

//     /* White */
//     ledSetRGB(fb, (i + loop) % count, 255, 255, 255, shift);
//     if (++i >= count) break;
//   }
// #endif
#if 1
  uint8_t shift = getShiftCeiling ();
  while (i < count) {
    if (loop & 1) {
      /* Black */
       ledSetRGB (fb, (i++ + loop) % count, 0, 0, 0, shift);

      /* Black */
       ledSetRGB (fb, (i++ + loop) % count, 0, 0, 0, shift);

      /* Red */
       ledSetRGB (fb, (i++ + loop) % count, 128, 0, 0, shift);
    }
    else {
      /* Red */
       ledSetRGB (fb, (i++ + loop) % count, 128, 0, 0, shift);

      /* Black */
       ledSetRGB (fb, (i++ + loop) % count, 0, 0, 0, shift);

      /* Black */
       ledSetRGB (fb, (i++ + loop) % count, 0, 0, 0, shift);
    }
  }
#endif
// #if 0
//   //  int threshold = (phageAdcGet() * count / 4096);
//   int threshold = (20 * count / 4096);   ////////// NOTE NOTE BODGE
//   for (i = 0; i < count; i++) {
//     if (i > threshold)
//       ledSetRGB(fb, i, 255, 0, 0, shift);
//     else
//       ledSetRGB(fb, i, 0, 0, 0, shift);
//   }
// #endif
 
}
orchard_effects("safetyPattern", testPatternFB);






#define BUMP_DEBOUNCE 300 // 300ms debounce to next bump

void bump(uint32_t amount) {
  bump_amount = amount;
  if( chVTGetSystemTime() - bumptime > BUMP_DEBOUNCE ) {
    bumptime = chVTGetSystemTime();
    bumped = 1;
  }
}

static void draw_pattern(void) {
  const OrchardEffects *curfx;
  
  curfx = orchard_effects_start();
  
  fx_config.loop++;

  if( bump_amount != 0 ) {
    fx_config.loop += bump_amount;
    bump_amount = 0;
  }

  curfx += fx_index;

  curfx->computeEffect(&fx_config);
}

const char *effectsCurName(void) {
  const OrchardEffects *curfx;
  curfx = orchard_effects_start();
  curfx += fx_index;
  
  return (const char *) curfx->name;
}

uint8_t effectsNameLookup(const char *name) {
  uint8_t i;
  const OrchardEffects *curfx;

  curfx = orchard_effects_start();
  if( name == NULL ) {
    return 0;
  }
  
  for( i = 0; i < fx_max; i++ ) {
    if( strcmp(name, curfx->name) == 0 ) {
      return i;
    }
    curfx++;
  }
  
  return 0;  // name not found returns default effect
}

// checks to see if the current effect is one of the lightgenes
// if it is, updates the diploid genome to the current effect
void check_lightgene_hack(void) {
  const struct genes *family;
  uint8_t family_member = 0;
  
  if( strncmp(effectsCurName(), "Lg", 2) == 0 ) {
    family = (const struct genes *) storageGetData(GENE_BLOCK);
    // handle lightgene special case
    family_member = effectsCurName()[2] - '0';
    computeGeneExpression(&(family->haploidM[family_member]),
			  &(family->haploidP[family_member]), &diploid);
  }
}

const char *lightgeneName(void) {
  return diploid.name;
}




void effectsSetPattern(uint8_t index) {
  if(index > fx_max) {
    fx_index = 0;
    return;
  }

  fx_index = index;
  patternChanged = 1;
  check_lightgene_hack();
}

uint8_t effectsGetPattern(void) {
  return fx_index;
}

void effectsNextPattern(int skipstrobe) {
  fx_index = (fx_index + 1) % fx_max;

  if(skipstrobe) {
    if(strncmp(effectsCurName(), "strobe", 6) == 0) {
      fx_index = (fx_index + 1) % fx_max;
    }
  }

  patternChanged = 1;
  check_lightgene_hack();
}

void effectsPrevPattern(int skipstrobe) {
  if( fx_index == 0 ) {
    fx_index = fx_max - 1;
  } else {
    fx_index--;
  }

  if(skipstrobe) {
    if(strncmp(effectsCurName(), "strobe", 6) == 0) {
      if( fx_index == 0 ) {
	fx_index = fx_max - 1;
      } else {
	fx_index--;
      }
    }
  }
  
  patternChanged = 1;
  check_lightgene_hack();
}

static void blendFbs(void) {
  uint8_t i;
  // UI FB + effects FB blend (just do a saturating add)
  for( i = 0; i < led_config.ui_pixels * 3; i ++ ) {
    led_config.final_fb[i] = satadd_8(led_config.fb[i], led_config.ui_fb[i]);
  }

  // copy over the remainder of the effects FB that extends beyond UI FB
  for( i = led_config.ui_pixels * 3; i < led_config.max_pixels * 3; i++ ) {
    led_config.final_fb[i] = led_config.fb[i];
  }
  if( ledExitRequest ) {
    for( i = 0; i < led_config.max_pixels * 3; i++ ) {
      led_config.final_fb[i] = 0; // turn all the LEDs off
    }
  }
}

// static THD_WORKING_AREA(waEffectsThread, 256);
static THD_FUNCTION(effects_thread, arg) {

  (void)arg;
  chRegSetThreadName("effects");

  while (!ledsOff) {
    blendFbs();
    
    // transmit the actual framebuffer to the LED chain
    chSysLock();
    ledUpdate(led_config.final_fb, led_config.pixel_count);
    chSysUnlock();

    // wait until the next update cycle
    chThdYield();
    chThdSleepMilliseconds(EFFECTS_REDRAW_MS);

    // re-render the internal framebuffer animations
    draw_pattern();

    if( ledExitRequest ) {
      // force one full cycle through an update on request to force LEDs off
      blendFbs(); 
      chSysLock();
      ledUpdate(led_config.final_fb, led_config.pixel_count);
      ledsOff = 1;
      chSysUnlock();
      // chThdExitS(MSG_OK);
      return; // this is the same as an exit
    }
  }
  return;
}

void listEffects(void) {
  const OrchardEffects *curfx;

  curfx = orchard_effects_start();
  chprintf(stream, "max effects %d\n\r", fx_max );

  while( curfx->name ) {
    chprintf(stream, "%s\n\r", curfx->name );
    curfx++;
  }
}

void effectsStart(void) {
  const OrchardEffects *curfx;
  
  fx_config.hwconfig = &led_config;
  fx_config.count = led_config.pixel_count;
  fx_config.loop = 0;
  
  strncpy( diploid.name, "err!", GENE_NAMELENGTH ); // in case someone references before init

  curfx = orchard_effects_start();
  fx_max = 0;
  fx_index = 0;
  while( curfx->name ) {
    fx_max++;
    curfx++;
  }
  check_lightgene_hack();

  draw_pattern();
  ledExitRequest = 0;
  ledsOff = 0;

  //  chThdCreateStatic(waEffectsThread, sizeof(waEffectsThread),
  //      NORMALPRIO - 6, effects_thread, &led_config);
  chThdCreateFromHeap(NULL, THD_WORKING_AREA_SIZE(256), "effects", NORMALPRIO - 6, effects_thread, &led_config);
}

#ifdef TESTING_PLEASE
OrchardTestResult test_led(const char *my_name, OrchardTestType test_type) {
   (void) my_name;
  
   OrchardTestResult result = orchardResultPass;
   uint16_t i;
   uint8_t interactive = 0;
  
   switch(test_type) {
   case orchardTestPoweron:
      // the LED is not easily testable as it's "write-only"
      return orchardResultUnsure;
   case orchardTestInteractive:
      interactive = 20;  // 20 seconds to evaluate LED state...should be plenty
   case orchardTestTrivial:
   case orchardTestComprehensive:
      orchardTestPrompt ("Preparing", "LED test", 0);
      while(ledsOff == 0) {
         effectsStop ();
         chThdYield ();
         chThdSleepMilliseconds (100);
      }

      // green pattern
      for( i = 0; i < led_config.pixel_count * 3; i += 3 ) {
         led_config.final_fb[i] = 255;
         led_config.final_fb[i+1] = 0;
         led_config.final_fb[i+2] = 0;
      }
      chSysLock ();
      ledUpdate (led_config.final_fb, led_config.pixel_count);
      chSysUnlock ();
      orchardTestPrompt ("green LED test", "", 0);
      orchardTestPrompt ("press button", "to advance", interactive);
      chThdSleepMilliseconds (200);

      // red pattern
      for( i = 0; i < led_config.pixel_count * 3; i += 3 ) {
         led_config.final_fb[i] = 0;
         led_config.final_fb[i+1] = 255;
         led_config.final_fb[i+2] = 0;
      }
      chSysLock ();
      ledUpdate (led_config.final_fb, led_config.pixel_count);
      chSysUnlock ();
      chThdSleepMilliseconds (200);
      orchardTestPrompt ("red LED test", "", 0);
      orchardTestPrompt ("press button", "to advance", interactive);
      chThdSleepMilliseconds (200);

      // blue pattern
      for( i = 0; i < led_config.pixel_count * 3; i += 3 ) {
         led_config.final_fb[i] = 0;
         led_config.final_fb[i+1] = 0;
         led_config.final_fb[i+2] = 255;
      }
      orchardTestPrompt ("blue LED test", "", 0);
      chSysLock ();
      ledUpdate (led_config.final_fb, led_config.pixel_count);
      chSysUnlock ();
      chThdSleepMilliseconds (200);
      orchardTestPrompt ("press button", "to advance", interactive);
      chThdSleepMilliseconds (200);

      orchardTestPrompt ("LED test", "finished", 0);
      // resume effects
      effectsStart ();
      return result;
   default:
      return orchardResultNoTest;
   }
  
   return orchardResultNoTest;
}
orchard_test("ws2812b", test_led);
#endif

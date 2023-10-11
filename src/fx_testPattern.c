#if defined EGGS_TESTPATTERN
#include <strings.h>
#include <stdlib.h>

#include "hal.h"
#include "shell.h"

#include "chprintf.h"

#include "led.h"
#include "orchard-effects.h"


static int led_ = 0;
static int ledCount_ = -1;

#define NL SHELL_NEWLINE_STR

// Declerations
static void showMyHelp (BaseSequentialStream*);



//! The object of this is to sort out how the terminal thing works and what the id's of all the led's are
//! I want a pretty picture of a heart where I can point to each led and say you are number 6. 
static void fx_testPattern (struct effects_config* config)
{
   uint8_t *fb = config-> hwconfig-> fb;

   //- If the count is -1 then get the count and set the count here so it can be used in the command
   if (ledCount_ == -1)
      ledCount_ = (int) config-> count;

   //- This blanks all the leds
   for (uint32_t i = 0; i < config-> count; ++i) 
      ledSetRGB (fb, i, 0, 0, 0, getShift ());

   //- Now this will set one of them green
   ledSetRGB (fb, led_, 0, 255, 0, getShift ());
}
orchard_effects ("e_test", fx_testPattern);


//! THis is a command to advance the led
void cmd_SetTestPatternLed (BaseSequentialStream* output, int argc, char* argv [])
{
   if (argc <= 0)
   {
      showMyHelp (output);
      return;
   }

   //- Lets get some info
   if (strcasecmp (argv[0], "get") == 0)
   {
      if (argc <= 1)
      {
         chprintf (output, "get needs at least one argument" NL);
         showMyHelp (output);
         return;
      }

      if (strcasecmp (argv[1], "count") == 0)
         chprintf (output, "Led Count: %d" NL , ledCount_);
      else if (strcasecmp (argv[1], "led") == 0)
         chprintf (output, "Current Led: %d" NL , led_);
      return;
   }

   //- Lets try to set the led
   else if (strcasecmp (argv[0], "set") == 0)
   {
      if (argc <= 1)
      {
         chprintf (output, "set needs at least one argument"NL);
         showMyHelp (output);
         return;
      }

      if (ledCount_ == -1)
      {
         chprintf (output, "The led count is not set. Let the e_test pattern run for a sec"NL);
         return;
      }

      int ledToSet = atoi (argv[1]);
      if (ledToSet >= ledCount_
          || ledToSet < 0)
      {
         chprintf (output, "ERROR: Cannot set the led, '%d'. Must be a value between [0-%d)."NL, ledToSet, ledCount_);
         return;
      }

      led_ = ledToSet;
      chprintf (output, "Setting led: %d" NL, led_);
   }
}

//! Show me some help
static void showMyHelp (BaseSequentialStream* output)
{
   chprintf (output,
             "Usage e_test_ctrl [arg]:\r\n" 
             "    get count    get led count\r\n"
             "    get led      get the current led\r\n"
             "    set n        set the nth led\r\n");
   
}

#endif

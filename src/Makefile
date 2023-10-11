##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

# Compiler options here.
ifeq ($(USE_OPT),)  # use O2 for production
  USE_OPT = -O2 # -O2 or -Os
  USE_OPT += -fomit-frame-pointer -falign-functions=16 -nostdlib -DSHELL_CONFIG_FILE -fstack-protector
  USE_OPT += -ggdb
endif

# C specific options here (added to USE_OPT).
ifeq ($(USE_COPT),)
  USE_COPT =
endif

# C++ specific options here (added to USE_OPT).
ifeq ($(USE_CPPOPT),)
  USE_CPPOPT = -fno-rtti
endif

# Enable this if you want the linker to remove unused code and data
ifeq ($(USE_LINK_GC),)
  USE_LINK_GC = yes
endif

# Linker extra options here.
ifeq ($(USE_LDOPT),)
  USE_LDOPT =
endif

# Enable this if you want link time optimizations (LTO)
ifeq ($(USE_LTO),)
  USE_LTO = no   # this will get rid of .cfmprotect / flashcfg, so leave it off
endif

# If enabled, this option allows to compile the application in THUMB mode.
ifeq ($(USE_THUMB),)
  USE_THUMB = yes
endif

# Enable this if you want to see the full log while compiling.
ifeq ($(USE_VERBOSE_COMPILE),)
  USE_VERBOSE_COMPILE = no
endif

# If enabled, this option makes the build process faster by not compiling
# modules not used in the current configuration.
ifeq ($(USE_SMART_BUILD),)
  USE_SMART_BUILD = yes
endif

#
# Build global options
##############################################################################

##############################################################################
# Architecture or project specific options
#

# Stack size to be allocated to the Cortex-M process stack. This stack is
# the stack used by the main() thread.
ifeq ($(USE_PROCESS_STACKSIZE),)
  USE_PROCESS_STACKSIZE = 0x300
endif

# Stack size to the allocated to the Cortex-M main/exceptions stack. This
# stack is used for processing interrupts and exceptions.
ifeq ($(USE_EXCEPTIONS_STACKSIZE),)
  USE_EXCEPTIONS_STACKSIZE = 0x200
endif

# Enables the use of FPU on Cortex-M4 (no, softfp, hard).
ifeq ($(USE_FPU),)
  USE_FPU = hard
#  USE_OPT += -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fsingle-precision-constant
#  DDEFS += -DCORTEX_USE_FPU=TRUE
endif

#
# Architecture or project specific options
##############################################################################

##############################################################################
# Project, sources and paths
#

# Define project name here
PROJECT = bm20

# Imported source files and paths
CHIBIOS = ../ChibiOS
CHIBIOS_CONTRIB = ../ChibiOS-Contrib
XZSRC = ..

# Startup files.
include $(CHIBIOS_CONTRIB)/os/common/startup/ARMCMx/compilers/GCC/mk/startup_k22x.mk
# HAL-OSAL files (optional).
include $(CHIBIOS)/os/hal/hal.mk
include $(CHIBIOS_CONTRIB)/os/hal/ports/KINETIS/K22x/platform.mk
include ../boards/KOSAGI_BM17/board.mk
include $(CHIBIOS)/os/hal/osal/rt/osal.mk
# RTOS files (optional).
include $(CHIBIOS)/os/rt/rt.mk
include $(CHIBIOS)/os/common/ports/ARMCMx/compilers/GCC/mk/port_v7m.mk
# Other files (optional).
include $(CHIBIOS)/test/rt/test.mk
# Shell files
include $(CHIBIOS)/os/hal/lib/streams/streams.mk
include $(CHIBIOS)/os/various/shell/shell.mk

# Define linker script file here
LDSCRIPT= ../ld/MK22FN128.ld

#cruft from CMSIS-DAP openSDA
#OPENSDA = opensda
#include $(OPENSDA)/build.mk

# uGFX includes
GFXLIB =../ugfx
include $(GFXLIB)/gfx.mk
include $(GFXLIB)/boards/base/Kosagi-XZ/board.mk
include $(GFXLIB)/drivers/gdisp/SSD1306/driver.mk

# libfixmath
LIBFIXMATH = ../libfixmath
include $(LIBFIXMATH)/build.mk

# libfixmatrix
#LIBFIXMATRIX = ../libfixmatrix
#include $(LIBFIXMATRIX)/build.mk

#C90TFS Freescale Flash drivers
LIBC90TFS = ../C90TFS
include $(LIBC90TFS)/build.mk

# CM4 DSP Lib
# Some notes on libs. I tried to add the cm4 dsp lib. This is an arm lib that has all the fun dsp tools. Great! cause
# the chip on this thing has a built in dsp and fpu. Problem is this lib takes way way too much memory
#CM4_DSP_LIB = ../cm4-dsp-lib
#DSPBASE = $(CM4_DSP_LIB)
#include $(CM4_DSP_LIB)/dsp.mk

# C sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CSRC = \
       $(STARTUPSRC) \
       $(KERNSRC) \
       $(PORTSRC) \
       $(OSALSRC) \
       $(HALSRC) \
       $(PLATFORMSRC) \
       $(BOARDSRC) \
       $(STREAMSSRC) \
       $(SHELLSRC) \
       $(GFXSRC) \
       main.c \
       orchard-vectors.c \
       orchard-events.c \
       orchard-test.c \
       shellcfg.c \
       gitversion.c \
       oled.c \
       touch.c \
       charger.c \
       led.c \
       mic.c \
       hsvrgb.c \
       orchard-math.c \
       genes.c \
       flash.c \
       storage.c \
       userconfig.c \
       radio.c \
       accel.c \
       hex.c \
       paging.c \
       orchard-app.c \
       orchard-ui.c \
       analog.c \
       $(wildcard cmd-*.c) \
       $(wildcard app-*.c) \
       $(wildcard ui-*.c) \
       $(wildcard test-*.c) \
       $(LIBFIXMATHSRC) \
       $(LIBFIXMATRIXSRC) \
       $(LIBC90TFSSRC) \
       fx_testPattern.c \
       kissfft/kiss_fft.c \
       kissfft/kiss_fftr.c \
       AudioProcessor.c 


# C++ sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CPPSRC = 


# C sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACSRC =

# C++ sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACPPSRC =

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARMand THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCSRC =

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCPPSRC =

# List ASM source files here
ASMSRC = ws2812b_ll.s
ASMXSRC = $(STARTUPASM) $(PORTASM) $(OSALASM)

INCDIR = $(CHIBIOS)/os/license \
         $(STARTUPINC) $(KERNINC) $(PORTINC) $(OSALINC) \
         $(HALINC) $(PLATFORMINC) $(BOARDINC) $(TESTINC) \
         $(STREAMSINC) $(SHELLINC) $(GFXINC) $(LIBFIXMATHINC) $(LIBC90TFSINC) \
         -Isda 

#
# Project, sources and paths
##############################################################################

##############################################################################
# Compiler settings
#

MCU  = cortex-m4

#TRGT = arm-elf-
#TRGT = arm-none-eabi-  # woo ARM native
CC   = $(TRGT)gcc
CPPC = $(TRGT)g++
# Enable loading with g++ only if you need C++ runtime support.
# NOTE: You can use C++ even without C++ support if you are careful. C++
#       runtime support makes code size explode.
LD   = $(TRGT)gcc
#LD   = $(TRGT)g++
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
AR   = $(TRGT)ar
OD   = $(TRGT)objdump
SZ   = $(TRGT)size
HEX  = $(CP) -O ihex
BIN  = $(CP) -O binary
SREC = $(CP) -O srec

# ARM-specific options here
AOPT = 

# THUMB-specific options here
TOPT = -mthumb -DTHUMB

# Define C warning options here
CWARN = -Wall -Wextra -Wundef -Wstrict-prototypes

# Define C++ warning options here
CPPWARN = -Wall -Wextra -Wundef

#
# Compiler settings
##############################################################################

##############################################################################
# Start of user section
#

# List all user C define here, like -D_DEBUG=1  -DTESTER_DEVICE for tester peer
UDEFS = -DTARGET_MK22F -D__FPU_PRESENT=1# -DTESTER_DEVICE ## -DDISABLE_WATCHDOG  comment out -DDISABLE_WATCHDOG for production #
UDEFS +=  -DSCOPE_MODE=0    # Scope mode has two things. One as a screen saver it will show the scope. Also, it has a scope app
                            # These are turned off so that I can create an audio processing thread that runs in to process the audio
                            # data. The scope does that but it gets in my way.

# Debug Test pattern. That allows you to turn ligts on and off individually. See which led is which
#UDEFS += -DEGGS_TESTPATTERN  # This will add a test pattern for testing things

UDEFS += -DARM_MATH_CM4

# Slim mode is a glag that will do everything it can to shrink the code base
UDEFS += -DSLIM_MODE

# Testing is important but tests take up too much space uncomment this to bring back tests
#UDEFS += -DTESTING_PLEASE
UDEFS += -DNDEBUG  # This turns off the debug for kiss fft
UDEFS += -DKISS_FFT_MALLOC=dummyMalloc -DKISS_FFT_FREE=dummyFree

# Define ASM defines here
UADEFS =

# List all user directories here
UINCDIR =

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS = lib/libm.a lib/armv7e-m-fpu-libgcc.a lib/libc-nano-armv7e-m-fpu.a

#ULIBS += $(CM4_DSP_LIB)/libmath_cm4f.a
#ULIBS = lib/libm.a lib/libc_nano.a lib/libg_nano.a lib/libgcc.a 

#
# End of user defines
##############################################################################

RULESPATH = $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC
include $(RULESPATH)/rules.mk

gitversion.c: $(XZSRC)/.git/HEAD $(XZSRC)/.git/index
	echo "const char *gitversion = \"$(shell git rev-parse HEAD)\";" > $@

gitversion.h: $(XZSRC)/.git/HEAD $(XZSRC)/.git/index
	echo "#define GIT_COMMIT_SHA \"$(shell git rev-parse HEAD)\"" > $@


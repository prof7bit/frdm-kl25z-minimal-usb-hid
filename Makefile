########################################
## bare bones makefile for ARM Cortex ##
########################################

NAME      = frdm_kl25z_minimal_usb_hid

MKDIR     = mkdir -p

SRCS      = $(wildcard src/*.c)
SRCS     += $(wildcard src/usb/*.c)
SRCS     += $(wildcard kl25_src/*.c)
SRCS     += $(wildcard kl25_src/*.s)
SRCS     += src/usb/usb_descriptors.c

INCDIRS   = src/
INCDIRS  += src/usb/
INCDIRS  += kl25_src/

DEFINES   = 

LSCRIPT   = kl25_src/MKL25Z32xxx4.ld

BUILDDIR  = build/

CFLAGS    = -ffunction-sections
CFLAGS   += -mlittle-endian
CFLAGS   += -mthumb
CFLAGS   += -mcpu=cortex-m0plus
CFLAGS   += -std=gnu99
CFLAGS   += -ggdb

ifdef DEBUG
	CFLAGS   += -O0
	DEFINES  += -DDEBUG
else
	CFLAGS   += -Os -flto
endif

LFLAGS    = --specs=nano.specs
LFLAGS   += --specs=nosys.specs
LFLAGS   += -nostartfiles
LFLAGS   += -Wl,--gc-sections
LFLAGS   += -T$(LSCRIPT)
LFLAGS   += -lm

WFLAGS    = -Wall
WFLAGS   += -Wextra 
WFLAGS   += -Werror -Wno-error=unused-function -Wno-error=unused-variable
WFLAGS   += -Wfatal-errors 
WFLAGS   += -Warray-bounds 
WFLAGS   += -Wno-unused-parameter

GCCPREFIX = arm-none-eabi-
CC        = $(GCCPREFIX)gcc
OBJCOPY   = $(GCCPREFIX)objcopy
OBJDUMP   = $(GCCPREFIX)objdump
SIZE	  = $(GCCPREFIX)size

OOCD      = openocd
OOCD_CFG  = board/st_nucleo_f401re.cfg

INCLUDE   = $(addprefix -I,$(INCDIRS))
OBJS      = $(addprefix $(BUILDDIR),$(addsuffix .o,$(basename $(SRCS))))


###########
## rules ##
###########

.DELETE_ON_ERROR:

.PHONY: all
all: src/usb/usb_descriptors.c
all: $(SRCS)
all: $(OBJS)
all: $(BUILDDIR)$(NAME).elf
all: $(BUILDDIR)$(NAME).bin
all: $(BUILDDIR)$(NAME).s19
all: $(BUILDDIR)$(NAME).lst
all: print_size


$(SRCS): src/usb/usb_descriptors.h

src/usb/usb_descriptors.h: src/usb/usb_descriptors.py
	py src/usb/usb_descriptors.py

.PHONY: install
install: all
	$(call echo2,upload,$(BUILDDIR)$(NAME).s19)
	$(OOCD) -f $(OOCD_CFG) -c "program $(BUILDDIR)$(NAME).s19 verify reset"

.PHONY: clean
clean: 
	$(RM) -rf $(wildcard $(BUILDDIR)*)
	$(RM) src/usb/usb_descriptors.c
	$(RM) src/usb/usb_descriptors.h

# compiler
$(BUILDDIR)%.o: %.c
	$(MKDIR) $(dir $@)
	$(CC) -MMD -c -o $@ $(INCLUDE) $(DEFINES) $(CFLAGS) $(WFLAGS) $<

# assembler
$(BUILDDIR)%.o: %.s
	$(MKDIR) $(dir $@)
	$(CC) -c -x assembler-with-cpp -o $@ $(INCLUDE) $(DEFINES) $(CFLAGS) $(WFLAGS) $<

# linker
$(BUILDDIR)%.elf: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS)

%.bin: %.elf
	$(OBJCOPY) -O binary -S $< $@ 

%.s19: %.elf
	$(OBJCOPY) -O srec -S $< $@

%.lst: %.elf
	$(OBJDUMP) -D $< > $@ 

.PHONY: print_size
print_size: $(BUILDDIR)$(NAME).elf 
	$(SIZE) $(BUILDDIR)$(NAME).elf
	

#####################
## Advanced Voodoo ##
#####################

# try to include any compiler generated dependency makefile snippet *.d
# that might exist in BUILDDIR (but don't complain if it doesn't yet).
DEPS = $(addprefix $(BUILDDIR),$(patsubst %.c,%.d,$(filter %.c,$(SRCS))))
-include $(DEPS)

# make the object files also depend on the makefile itself
$(OBJS): Makefile


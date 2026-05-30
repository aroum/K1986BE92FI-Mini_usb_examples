######################################################################
# СБОРКА
######################################################################
# $(info    message)
TOP_DIR 	:= $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
HRD_PROBE		= ./dep/probe/jlink4swd.cfg	# JLlink hardware probe

######
# Компилируемый	проект
EXAMPLE		= rtos_blink

include $(TOP_DIR)examples/${EXAMPLE}/Makefile.inc

BINARY		=	main
LDSCRIPT	?= $(TOP_DIR)/dep/gcc/MDR32F9Qx.ld

INC 		=	-I$(TOP_DIR)lib/Config \
				-I$(TOP_DIR)lib/CMSIS/MDR32FxQI/CoreSupport/CM3 \
				-I$(TOP_DIR)lib/CMSIS/MDR32FxQI/DeviceSupport/MDR32F9Q2I/inc \
				-I$(TOP_DIR)lib/CMSIS/MDR32FxQI/DeviceSupport/MDR32F9Q2I/startup \
				-I$(TOP_DIR)lib/SPL/MDR32FxQI/inc \
				-I$(TOP_DIR)lib/SPL/MDR32FxQI/inc/USB_Library \
				-I$(TOP_DIR)lib/SPL/MDR32FxQI/inc/IRQ_Handler_Template \
				-I$(TOP_DIR)lib/Debug \
				-I$(TOP_DIR)lib/LOCM3/inc \
				-I$(TOP_DIR)examples \
				-I$(TOP_DIR)inc \
				-I$(TOP_DIR)examples/${EXAMPLE}

SRCFILES	=	$(TOP_DIR)examples/${EXAMPLE}/main.c \
				$(TOP_DIR)dep/gcc/startup_MDR32F9Qx.S \
				$(TOP_DIR)lib/CMSIS/MDR32FxQI/DeviceSupport/MDR32F9Q2I/startup/system_MDR32F9Q2I.c \
				$(TOP_DIR)lib/Debug/rtt/SEGGER_RTT.c \
				$(TOP_DIR)lib/Debug/rtt/SEGGER_RTT_printf.c

PREFIX		?= arm-none-eabi
DEFS		+= -DUSE_MDR32F9Q2I -D__STARTUP_CLEAR_BSS -D__START=main
FP_FLAGS	?= -msoft-float
ARCH_FLAGS	= -mthumb -mcpu=cortex-m3 $(FP_FLAGS) -mfix-cortex-m3-ldrd
ASFLAGS		= -mthumb -mcpu=cortex-m3

# Добавить ресурсные файлы примера если они есть
ifdef EXAMPLES_SRCFILES
SRCFILES	+=	$(foreach n, $(EXAMPLES_SRCFILES), $(TOP_DIR)examples/$(n) )
endif

ifdef EXAMPLES_LOCAL_SRCFILES
SRCFILES	+=	$(foreach n, $(EXAMPLES_LOCAL_SRCFILES), $(TOP_DIR)examples/$(EXAMPLE)/$(n) )
endif

ifdef LIB_SRCFILES
SRCFILES	+=	$(foreach n, $(LIB_SRCFILES), $(TOP_DIR)lib/$(n) )
endif

ifdef USE_RTOS
include $(TOP_DIR)lib/RTOS/Makefile.inc
INC 		+=	$(foreach n, $(RTOS_INC), -I$(TOP_DIR)lib/RTOS/$(n) )
SRCFILES	+=	$(foreach n, $(RTOS_SRCFILES), $(TOP_DIR)lib/RTOS/$(n) )
DEFS		+=	-DUSE_RTOS
endif

CC			:= $(PREFIX)-gcc
CXX			:= $(PREFIX)-g++
LD			:= $(PREFIX)-gcc
AR			:= $(PREFIX)-ar
AS			:= $(PREFIX)-as
OBJCOPY		:= $(PREFIX)-objcopy
SIZE		:= $(PREFIX)-size
OBJDUMP		:= $(PREFIX)-objdump
GDB			:= $(PREFIX)-gdb

OPT			:= -Os -g
CSTD		?= -std=c99

TEMP1 		= $(patsubst %.c,%.o,$(SRCFILES))
TEMP2 		= $(patsubst %.S,%.o,$(TEMP1))
TEMP3		= $(patsubst %.asm,%.o,$(TEMP2))
OBJS 		= $(patsubst %.cpp,%.o,$(TEMP3))

TGT_CFLAGS		+= $(OPT) $(CSTD)
TGT_CFLAGS		+= $(ARCH_FLAGS)
TGT_CFLAGS		+= -Wextra -Wshadow -Wimplicit-function-declaration
TGT_CFLAGS		+= -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes
TGT_CFLAGS		+= -fno-common -ffunction-sections -fdata-sections
TGT_CFLAGS		+= $(DEFS)
TGT_CFLAGS		+= $(INC)

TGT_CXXFLAGS	+= $(OPT) $(CXXSTD)
TGT_CXXFLAGS	+= $(ARCH_FLAGS)
TGT_CXXFLAGS	+= -Wextra -Wshadow -Wredundant-decls  -Weffc++
TGT_CXXFLAGS	+= -fno-common -ffunction-sections -fdata-sections

TGT_CPPFLAGS	+= -MD
TGT_CPPFLAGS	+= -Wall -Wundef
TGT_CPPFLAGS	+= $(DEFS)
TGT_CPPFLAGS	+= $(INC)

TGT_LDFLAGS		+= --static -nostartfiles
TGT_LDFLAGS		+= -T$(LDSCRIPT)
TGT_LDFLAGS		+= $(ARCH_FLAGS)
TGT_LDFLAGS		+= -Wl,-Map=$(*).map
TGT_LDFLAGS		+= -Wl,--gc-sections
TGT_LDFLAGS		+= -Wl,--print-memory-usage

LDLIBS		+= -specs=nosys.specs
LDLIBS		+= -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group

.SUFFIXES:	.elf .bin .hex .srec .list .map .images
.SECONDEXPANSION:
.SECONDARY:

elf:	$(DEPS) $(BINARY).elf
bin:	$(DEPS) $(BINARY).bin
hex:	$(DEPS) $(BINARY).hex
srec:	$(DEPS) $(BINARY).srec
list:	$(DEPS) $(BINARY).list

print-%:
	@echo $*=$($*)

%.images: %.bin %.hex %.srec %.list %.map
	@#printf "*** $* images generated ***\n"

%.bin: %.elf
	@echo "\033[92mOBJCOPY $(*).bin\033[0m"
	$(OBJCOPY) -Obinary $(*).elf $(*).bin

%.hex: %.elf
	@echo "\033[92mOBJCOPY $(*).hex\033[0m"
	$(OBJCOPY) -Oihex $(*).elf $(*).hex

%.srec: %.elf
	@echo "\033[92mOBJCOPY $(*).srec\033[0m"
	$(OBJCOPY) -Osrec $(*).elf $(*).srec

%.list: %.elf
	@echo "\033[92mOBJDUMP $(*).list\033[0m"
	$(OBJDUMP) -S $(*).elf > $(*).list

%.elf %.map: $(OBJS) $(LDSCRIPT)
	@echo "\033[92mELF $(*).elf\033[0m"
	$(LD) $(TGT_LDFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $(*).elf
	$(SIZE) $(BINARY).elf

%.o: %.c
	@echo "\033[92mCC $(*).c\033[0m"
	$(CC) $(TGT_CFLAGS) $(CFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $(*).o -c $(*).c

%.o: %.S
	@echo "\033[92mAS $(*).S\033[0m"
	$(CC) $(ASFLAGS) $(DEFS) -o $*.o -c $<

%.o: %.cxx
	@echo "\033[92mCXX $(*).cxx\033[0m"
	$(CXX) $(TGT_CXXFLAGS) $(CXXFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $(*).o -c $(*).cxx

%.o: %.cpp
	@echo "\033[92mCXX $(*).cpp\033[0m"
	@#printf "  CXX     $(*).cpp\n"
	$(CXX) $(TGT_CXXFLAGS) $(CXXFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $(*).o -c $(*).cpp

%.o: %.asm
	@echo "\033[92mASM $(*).asm\033[0m"
	$(AS) $(ASFLAGS) -o $*.o -c $<

clean:
	@echo "\033[91mremove build file\033[0m"
	$(RM) *.o *.d generated.* $(OBJS) $(patsubst %.o,%.d,$(OBJS))

clobber: clean
	@echo "\033[91mremove firmware file\033[0m"
	rm -f *.elf *.bin *.hex *.srec *.list *.map $(CLOBBER)

erase:
	@echo "\033[91merase chip\033[0m"
	openocd -f $(HRD_PROBE) -c "init" -c "halt" -c "flash erase_sector 0 0 31" -c "exit"

flash: bin
	@echo "\033[92mwrite flash chip\033[0m"
	openocd -f $(HRD_PROBE) -c "init" -c "halt" -c "program $(BINARY).bin verify reset 0x08000000" -c "exit"

forget: clobber erase

reset:
	openocd -f $(HRD_PROBE) -c "init" -c "reset run" -c "exit"

debugTask: reset flash

.PHONY: images clean elf bin hex srec list all

-include $(OBJS:.o=.d)
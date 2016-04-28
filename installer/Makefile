PATH := $(DEVKITPPC)/bin:$(PATH)
PREFIX ?= powerpc-eabi-
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc
CFLAGS = -std=gnu99 -Os -nostdinc -fno-builtin
ASFLAGS = -mregnames -x assembler-with-cpp
LD = $(PREFIX)ld
LDFLAGS=-Ttext 1800000 --oformat binary -L$(DEVKITPPC)/lib/gcc/powerpc-eabi/4.8.2 -lgcc
OBJDUMP ?= $(PREFIX)objdump
project	:=	.
root	:=	$(CURDIR)
build	:=	 $(root)/bin

sd_loader_elf := ../sd_loader/sd_loader.elf

CFLAGS += -DUSE_SD_LOADER
ASFLAGS += -DUSE_SD_LOADER

all: clean setup main532 main550 main500 main410 main400 main310 main300

sd_loader.h: $(sd_loader_elf)
	xxd -i $< | sed "s/unsigned/static const unsigned/g;s/loader/loader/g;s/build_//g" > $@

$(sd_loader_elf):
	make -C ../sd_loader
	
setup:
	mkdir -p $(root)/bin/

main550:
	make main FIRMWARE=550

main540:
	make main FIRMWARE=532

main532:
	make main FIRMWARE=532

main500:
	make main FIRMWARE=500

main410:
	make main FIRMWARE=410

main400:
	make main FIRMWARE=400

main310:
	make main FIRMWARE=310

main300:
	make main FIRMWARE=300

main210:
	make main FIRMWARE=210

main200:
	make main FIRMWARE=200

main: sd_loader.h
	$(CC) $(CFLAGS) -DVER=$(FIRMWARE) -c $(project)/launcher.c
	$(CC) $(CFLAGS) -DVER=$(FIRMWARE) -c $(project)/kexploit.c
	$(AS) $(ASFLAGS) -DVER=$(FIRMWARE) -c $(project)/kernel_patches.S
	$(AS) $(ASFLAGS) -DVER=$(FIRMWARE) -c $(project)/crt0.S
	cp -r $(root)/*.o $(build)
	rm $(root)/*.o
	$(LD) -s -o $(build)/code$(FIRMWARE).bin $(build)/crt0.o `find $(build) -name "*.o" ! -name "crt0.o"` $(LDFLAGS)

clean:
	rm -rf $(build)
	rm -rf sd_loader.h

print_stats:
	@echo
	@echo "code size : loadiine   =>" `$(OBJDUMP) -h ../loadiine.elf | awk '/.kernel_code|.text|.menu_magic|.loader_magic|.fs_method_calls|.rodata|.data|.sdata|.bss|.sbss|.fs_magic/ { sum+=strtonum("0x"$$3) } END {print sum}'` / 7530312

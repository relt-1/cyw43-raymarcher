# Copyright (c) 2023 CarlosFTM
# This code is licensed under MIT license (see LICENSE.txt for details)


NAME    = boot
CPU     = cortex-m0plus
WIFICPU = cortex-m3
ARMGNU  = "C:\Program Files\Raspberry Pi\Pico SDK v1.5.1\gcc-arm-none-eabi\bin\arm-none-eabi"
AFLAGS  = --warn --fatal-warnings -mcpu=$(CPU)
LDFLAGS = -nostdlib
CFLAGS  = -mcpu=$(CPU) -ffreestanding -nostartfiles -std=c2x -fpic -mthumb -c
WIFICFLAGS  = -mcpu=$(WIFICPU) -ffreestanding -nostartfiles -std=c2x -fpic -mthumb -c
PICOSDK = "C:/Program Files/Raspberry Pi/Pico SDK v1.5.1/pico-sdk"
# -fisolate-erroneous-paths-dereference  REMOVES BOOTLOADER
DONTREMOVEMYBOOTLOADER = -O0
all: $(NAME).uf2

wifi_code_compiled.h : wifi_code.c wifimemmap.ld
	$(ARMGNU)-gcc -Ofast $(WIFICFLAGS) wifi_code.c -o wifi_code_compiled.o
	$(ARMGNU)-ld $(LDFLAGS) -T wifimemmap.ld wifi_code_compiled.o -o wifi_code_compiled.elf
	$(ARMGNU)-objcopy -O binary wifi_code_compiled.elf wifi_code_compiled.bin
	xxd -i wifi_code_compiled.bin > wifi_code_compiled.h

bootloader.bin : bootloader.c memmap_bootloader.ld
	$(ARMGNU)-gcc $(DONTREMOVEMYBOOTLOADER) $(CFLAGS) bootloader.c -o bootloader.o
	$(ARMGNU)-ld $(LDFLAGS) -T memmap_bootloader.ld bootloader.o -o bootloader.elf
	$(ARMGNU)-objdump -m"armv6-m" -Mreg-names-raw,force-thumb -D bootloader.elf > bootloader.list
	$(ARMGNU)-objcopy -O binary bootloader.elf bootloader.bin
	$(ARMGNU)-objdump -b binary -m"armv6-m" -Mreg-names-raw,force-thumb -D bootloader.bin > bootloader.dism

bootloader_patch.o : bootloader.bin
	python $(PICOSDK)/src/rp2_common/boot_stage2/pad_checksum -p 256 -s 0xFFFFFFFF bootloader.bin bootloader_patch.s
	$(ARMGNU)-as $(AFLAGS) bootloader_patch.s -o bootloader_patch.o

image.o : image.raw
	$(ARMGNU)-objcopy -I binary -O elf32-littlearm-fdpic image.raw image.o


$(NAME).o : $(NAME).c wifi_code_compiled.h
	$(ARMGNU)-gcc -Ofast $(CFLAGS) $(NAME).c -o $(NAME).o

wifi_spi.o : wifi_spi.c
	$(ARMGNU)-gcc -Ofast $(CFLAGS) wifi_spi.c -o wifi_spi.o



$(NAME).bin : memmap.ld bootloader_patch.o $(NAME).o image.o wifi_spi.o
	$(ARMGNU)-ld $(LDFLAGS) -T memmap.ld bootloader_patch.o $(NAME).o image.o wifi_spi.o -o $(NAME).elf
	$(ARMGNU)-objdump -m"armv6-m" -Mreg-names-raw,force-thumb -D $(NAME).elf > $(NAME).list
	$(ARMGNU)-objcopy -O binary $(NAME).elf $(NAME).bin
	$(ARMGNU)-objdump -b binary -m"armv6-m" -Mreg-names-raw,force-thumb -D $(NAME).bin > $(NAME).dism

$(NAME).uf2 : $(NAME).bin
	python uf2conv.py $(NAME).bin --base 0x20000000 --family RP2040 --output $(NAME).uf2

clean:
	rm -f *.o *.bin *.elf *.list bootloader_patch.* *.dism
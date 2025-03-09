MAKEFILEPATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENTDIR := $(notdir $(patsubst %/,%,$(dir $(mkfile_path))))

CFILES = $(wildcard ../Code/*/*.c) $(wildcard ../Code/*.c)
OFILES = $(CFILES:.c=.o)
GCCFLAGS = -Wall -O3 -ffreestanding -nostdinc -nostdlib -nostartfiles -mstrict-align -I../Code

all: clean kernel8.img

boot.o: ../Code/boot.S
	aarch64-none-elf-gcc $(GCCFLAGS) -c ../Code/boot.S -o boot.o

%.o: %.c
	aarch64-none-elf-gcc $(GCCFLAGS) -c $< -o $@

font_psf.o: ../Code/fonts/font.psf
	aarch64-none-elf-ld -r -b binary -o $@ $<

font_sfn.o: ../Code/fonts/font.sfn
	aarch64-none-elf-ld -r -b binary -o $@ $<

kernel8.img: boot.o font_psf.o font_sfn.o $(OFILES)
	aarch64-none-elf-ld -nostdlib boot.o font_psf.o font_sfn.o $(OFILES) -T linker.ld -o kernel8.elf
	aarch64-none-elf-objcopy -O binary kernel8.elf kernel8.img

run: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial stdio

clean:
	rm kernel8.elf *.o *.img > /dev/null 2> /dev/null || true
	rm  ../Code/*.o ../Code/klib/*.o ../Code/stdlib/*.o > /dev/null 2> /dev/null || true
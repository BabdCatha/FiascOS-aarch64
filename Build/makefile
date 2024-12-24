CFILES = $(wildcard ../Code/*.c)
OFILES = $(CFILES:.c=.o)
GCCFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles

all: clean kernel8.img

boot.o: ../Code/boot.S
	aarch64-none-elf-gcc $(GCCFLAGS) -c ../Code/boot.S -o boot.o

%.o: %.c
	aarch64-none-elf-gcc $(GCCFLAGS) -c $< -o $@

kernel8.img: boot.o $(OFILES)
	aarch64-none-elf-ld -nostdlib boot.o $(OFILES) -T linker.ld -o kernel8.elf
	aarch64-none-elf-objcopy -O binary kernel8.elf kernel8.img

run: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial stdio

clean:
	rm kernel8.elf *.o *.img > /dev/null 2> /dev/null || true
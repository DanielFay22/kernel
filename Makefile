

OFORMAT = elf32
FILES = main.o scrn.o start.o kernel.bin
CFLAGS = -Wall -O -fstrength-reduce -fomit-frame-pointer \
	-finline-functions -nostdinc -fno-builtin -I ./include -c

CC = /mnt/c/Users/Daniel/CLionProjects/kernel/compiler/bin/i686-elf-gcc
OBJECTS = main.o scrn.o start.o gdt.o idt.o isrs.o irq.o timer.o keyboard.o



all: $(OBJECTS) kernel image build_cleanup

kernel: $(OBJECTS)
	$(CC) -T link.ld -o kernel.bin -ffreestanding -nostdlib $(OBJECTS) -lgcc

image:
	mkdir -p isodir/boot/grub
	cp kernel.bin isodir/boot/kernel.bin
	cp grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o kernel.iso isodir

start.o: start.asm
	nasm -f$(OFORMAT) -o start.o start.asm

main.o: main.c
	$(CC) $(CFLAGS) -o main.o main.c

scrn.o: scrn.c
	$(CC) $(CFLAGS) -o scrn.o scrn.c

gdt.o: gdt.c
	$(CC) $(CFLAGS) -o gdt.o gdt.c

idt.o: idt.c
	$(CC) $(CFLAGS) -o idt.o idt.c

isrs.o: isrs.c
	$(CC) $(CFLAGS) -o isrs.o isrs.c

irq.o: irq.c
	$(CC) $(CFLAGS) -o irq.o irq.c

timer.o: timer.c
	$(CC) $(CFLAGS) -o timer.o timer.c

keyboard.o: keyboard.c
	$(CC) $(CFLAGS) -o keyboard.o keyboard.c


build_cleanup:
	mkdir -p build_output
	mv *.o build_output

clean:
	rm -f $(OBJECTS) kernel.bin



OFORMAT = elf32
FILES = main.o scrn.o start.o kernel.bin
CFLAGS = -Wall -O0 -fstrength-reduce -fomit-frame-pointer \
	-finline-functions -nostdinc -fno-builtin -I ./include -c

DIR = /mnt/c/Users/Daniel/CLionProjects/kernel
CC = $(DIR)/compiler/bin/i686-elf-gcc
LINK = $(DIR)/resources/link.ld
OBJECTS = main.o scrn.o start.o gdt.o idt.o isrs.o irq.o timer.o keyboard.o malloc.o string.o swi.o



all: kernel.iso

kernel.bin: $(OBJECTS)
	$(CC) -T $(LINK) -o kernel.bin -ffreestanding -nostdlib $(OBJECTS) -lgcc

kernel.iso: kernel.bin
	mkdir -p isodir/boot/grub
	cp kernel.bin isodir/boot/kernel.bin
	cp resources/grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o kernel.iso isodir
	rm -r isodir/

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

#memory.o: memory.c
#	$(CC) $(CFLAGS) -o memory.o memory.c

malloc.o: malloc.c
	$(CC) $(CFLAGS) -o malloc.o malloc.c

string.o: string.c
	$(CC) $(CFLAGS) -o string.o string.c

swi.o: swi.c
	$(CC) $(CFLAGS) -o swi.o swi.c

build_cleanup:
	mkdir -p build_output
	mv *.o build_output
	mv *.bin build_output

clean:
	rm -f $(OBJECTS) kernel.bin
	rm -f *.iso
	rm -rf isodir/

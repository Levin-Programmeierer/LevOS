CC = gcc

CFLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector -Wall -Wextra
LDFLAGS = -m elf_i386 -T linker.ld

all: os.iso

src/boot.o: src/boot.S
	$(CC) $(CFLAGS) -c src/boot.S -o src/boot.o

src/kernel.o: src/kernel.c
	$(CC) $(CFLAGS) -c src/kernel.c -o src/kernel.o

src/keyboard.o: src/keyboard.c
	$(CC) $(CFLAGS) -c src/keyboard.c -o src/keyboard.o

src/terminal.o: src/terminal.c
	$(CC) $(CFLAGS) -c src/terminal.c -o src/terminal.o

src/io.o: src/io.c
	$(CC) $(CFLAGS) -c src/io.c -o src/io.o

src/idt.o: src/idt.S
	$(CC) $(CFLAGS) -c src/idt.S -o src/idt.o

src/interrupts.o: src/interrupts.c
	$(CC) $(CFLAGS) -c src/interrupts.c -o src/interrupts.o
src/shell.o: src/shell.c
	$(CC) $(CFLAGS) -c src/shell.c -o src/shell.o

kernel.bin: src/boot.o src/kernel.o src/keyboard.o src/terminal.o src/io.o src/idt.o src/interrupts.o src/shell.o
	ld $(LDFLAGS) -o kernel.bin src/boot.o src/kernel.o src/keyboard.o src/terminal.o src/io.o src/idt.o src/interrupts.o src/shell.o

os.iso: kernel.bin
	mkdir -p iso/boot
	cp kernel.bin iso/boot/kernel.bin
	grub-mkrescue -o os.iso iso

clean:
	rm -f src/boot.o src/kernel.o src/keyboard.o src/terminal.o src/io.o src/idt.o src/interrupts.o
	rm -f kernel.bin os.iso

run: os.iso
	qemu-system-i386 -cdrom os.iso
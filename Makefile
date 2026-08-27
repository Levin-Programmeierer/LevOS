CC = gcc
NASM = nasm

CFLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector -Wall -Wextra -Isrc
NASMFLAGS = -f elf32 -Isrc

LDFLAGS = -m elf_i386 -T linker.ld

OBJS = \
	build/boot.o \
	build/kernel.o \
	build/keyboard.o \
	build/terminal.o \
	build/io.o \
	build/idt.o \
	build/interrupts.o \
	build/shell.o \
	build/gdt.o \
	build/gdt_asm.o

all: os.iso

# NASM assembly
build/boot.o: src/boot/boot.asm
	mkdir -p build
	$(NASM) $(NASMFLAGS) $< -o $@

build/idt.o: src/cpu/idt.asm
	mkdir -p build
	$(NASM) $(NASMFLAGS) $< -o $@

build/gdt_asm.o: src/memory/gdt.asm
	mkdir -p build
	$(NASM) $(NASMFLAGS) $< -o $@

# C files
build/kernel.o: src/kernel.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/keyboard.o: src/drivers/keyboard.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/gdt.o: src/memory/gdt.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/terminal.o: src/drivers/terminal.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/io.o: src/drivers/io.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/interrupts.o: src/cpu/interrupts.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/shell.o: src/shell/shell.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

# Link kernel
build/kernel.bin: $(OBJS)
	ld $(LDFLAGS) -o $@ $(OBJS)

# ISO
os.iso: build/kernel.bin
	mkdir -p iso/boot/grub
	cp build/kernel.bin iso/boot/kernel.bin
	grub-mkrescue -o $@ iso

clean:
	rm -f build/*.o build/kernel.bin os.iso

run: os.iso
	qemu-system-i386 -cdrom os.iso -vga std
CC = gcc

CFLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector -Wall -Wextra -Isrc

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
	build/paging.o \
	build/paging_asm.o \
	build/pmm.o

all: os.iso

build/boot.o: src/boot/boot.S
	$(CC) $(CFLAGS) -c $< -o $@

build/kernel.o: src/kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

build/keyboard.o: src/drivers/keyboard.c
	$(CC) $(CFLAGS) -c $< -o $@

build/terminal.o: src/drivers/terminal.c
	$(CC) $(CFLAGS) -c $< -o $@

build/io.o: src/drivers/io.c
	$(CC) $(CFLAGS) -c $< -o $@

build/idt.o: src/cpu/idt.S
	$(CC) $(CFLAGS) -c $< -o $@

build/interrupts.o: src/cpu/interrupts.c
	$(CC) $(CFLAGS) -c $< -o $@

build/shell.o: src/shell/shell.c
	$(CC) $(CFLAGS) -c $< -o $@

build/paging.o: src/memory/paging.c
	$(CC) $(CFLAGS) -c $< -o $@

build/paging_asm.o: src/memory/paging.S
	$(CC) $(CFLAGS) -c $< -o $@

build/pmm.o: src/memory/pmm.c
	$(CC) $(CFLAGS) -c $< -o $@

build/kernel.bin: $(OBJS)
	ld $(LDFLAGS) -o $@ $(OBJS)

os.iso: build/kernel.bin
	mkdir -p iso/boot
	cp build/kernel.bin iso/boot/kernel.bin
	grub-mkrescue -o $@ iso

clean:
	rm -f build/*.o build/kernel.bin os.iso

run: os.iso
	qemu-system-i386 -cdrom os.iso
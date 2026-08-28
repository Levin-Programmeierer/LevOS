CC = gcc
NASM = nasm

CFLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector -fno-asynchronous-unwind-tables -Wall -Wextra -Isrc
NASMFLAGS = -f elf32 -Isrc
DISK_IMAGE = fat32.img
DISK_SIZE_MB ?= 64

LDFLAGS = -m elf_i386 -T linker.ld

OBJS = \
	build/boot.o \
	build/kernel.o \
	build/keyboard.o \
	build/block.o \
	build/ata.o \
	build/fat32.o \
	build/terminal.o \
	build/io.o \
	build/idt.o \
	build/interrupts.o \
	build/shell.o \
	build/gdt.o \
	build/gdt_asm.o \
	build/cmos.o \
	build/pmm.o \
	build/paging.o \
	build/paging_asm.o \
	build/process.o \
	build/scheduler.o \
	build/timer.o \
	build/syscall.o \
	build/user_test.o

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

build/block.o: src/drivers/block.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/ata.o: src/drivers/ata.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/fat32.o: src/filesystem/fat32.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/gdt.o: src/memory/gdt.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/cmos.o: src/drivers/cmos.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/pmm.o: src/memory/pmm.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/paging.o: src/memory/paging.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/paging_asm.o: src/memory/paging.asm
	mkdir -p build
	$(NASM) $(NASMFLAGS) $< -o $@

build/process.o: src/process/process.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/scheduler.o: src/process/scheduler.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/timer.o: src/cpu/timer.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/syscall.o: src/syscall.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/user_test.o: src/process/user_test.c
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

$(DISK_IMAGE):
	dd if=/dev/zero of=$@ bs=1M count=$(DISK_SIZE_MB) status=none
	mkfs.fat -F 32 -n LEVOS $@

run: os.iso $(DISK_IMAGE)
	qemu-system-i386 -cdrom os.iso \
		-drive file=$(DISK_IMAGE),format=raw,if=ide,index=0,media=disk \
		-boot d -vga std
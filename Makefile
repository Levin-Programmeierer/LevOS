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
	build/paging.o \
	build/paging_asm.o \
	build/pmm.o \
	build/process.o \
	build/syscalls.o \
	build/timer.o \
	build/scheduler.o \
	build/user_binary.o \
	build/font.o \
	build/font_driver.o \
	build/shell.o \
	build/pci.o \
	build/qemu_vga.o

all: os.iso build/user.bin

build/boot.o: src/boot/boot.S
	$(CC) $(CFLAGS) -c $< -o $@

build/user_binary.o: build/user.bin
	objcopy -I binary -O elf32-i386 -B i386 $< $@

build/syscalls.o: src/cpu/syscalls.c
	$(CC) $(CFLAGS) -c $< -o $@

build/process.o: src/process/process.c
	$(CC) $(CFLAGS) -c $< -o $@

build/pci.o: src/drivers/pci.c
	$(CC) $(CFLAGS) -c $< -o $@

build/qemu_vga.o: src/drivers/qemu_vga.c
	$(CC) $(CFLAGS) -c $< -o $@

build/framebuffer_driver.o: src/drivers/framebuffer_driver.c
	$(CC) $(CFLAGS) -c $< -o $@

build/timer.o: src/cpu/timer.c
	$(CC) $(CFLAGS) -c $< -o $@

build/user.bin: src/user/user.S
	$(CC) $(CFLAGS) -c $< -o build/user.o
	ld -m elf_i386 -Ttext 0x00400000 --oformat binary -o $@ build/user.o

build/scheduler.o: src/process/scheduler.c
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

build/font.o: fonts/cp865-8x16.psf
	objcopy -I binary -O elf32-i386 -B i386 $< $@

build/font_driver.o: src/drivers/font.c
	$(CC) $(CFLAGS) -c $< -o $@

build/graphicsshell.o: src/shell/graphicsshell.c
	$(CC) $(CFLAGS) -c $< -o $@

os.iso: build/kernel.bin
	mkdir -p iso/boot/grub
	cp build/kernel.bin iso/boot/kernel.bin
	cp grub.cfg iso/boot/grub/grub.cfg
	grub-mkrescue -o $@ iso

clean:
	rm -f build/*.o build/kernel.bin os.iso

run: os.iso
	qemu-system-i386 -cdrom os.iso -vga std
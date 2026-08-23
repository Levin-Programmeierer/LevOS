#include "terminal.h"
#include "keyboard.h"

extern void load_idt(void); // load idt import from asm idt.S
extern void isr0(void);
extern void load_gdt(void);
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

struct IDTEntry {
    unsigned short offset_low;
    unsigned short selector;
    unsigned char zero;
    unsigned char type_attributes;
    unsigned short offset_high;
} __attribute__((packed));

struct IDTEntry idt[256];

struct IDTR {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed)); // no padding so its 6 bytes

struct IDTR idtr;

struct GDTEntry{ // for 32 bit protected mode
    unsigned short limit_low; // 16 bits of segments limit
    unsigned short base_low; //0-15 of addres
    unsigned char base_middle; // 16-23 of address
    unsigned char access; // tells what segment it is and what to do
    unsigned char granularity; // upper 4 bits of segment limit w/ config flags 32 bit segment normal = 0xCF
    unsigned char base_high; // 24-31 of address
};

struct GDTEntry gdt[3]; // 0 null descriptor 0x00, 1 kernel code 0x08, 2 kernel data 0x10

struct GDTR {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

struct GDTR gdtr;

static const char *exception_names[32] = {
    "Divide Error",
    "Debug",
    "Non-Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
}; // error codes

static void set_idt_gate(int vector, unsigned int address) {
    idt[vector].offset_low = address & 0xFFFF;
    idt[vector].selector = 0x08;
    idt[vector].zero = 0;
    idt[vector].type_attributes = 0x8E;
    idt[vector].offset_high = (address >> 16) & 0xFFFF;
}

void initialise_IDT(void){
    idtr.limit = sizeof(idt) - 1;
    idtr.base = (unsigned int) &idt[0];
    // all idt codes and so
    set_idt_gate(0,  (unsigned int)isr0);
    set_idt_gate(1,  (unsigned int)isr1);
    set_idt_gate(2,  (unsigned int)isr2);
    set_idt_gate(3,  (unsigned int)isr3);
    set_idt_gate(4,  (unsigned int)isr4);
    set_idt_gate(5,  (unsigned int)isr5);
    set_idt_gate(6,  (unsigned int)isr6);
    set_idt_gate(7,  (unsigned int)isr7);
    set_idt_gate(8,  (unsigned int)isr8);
    set_idt_gate(9,  (unsigned int)isr9);
    set_idt_gate(10, (unsigned int)isr10);
    set_idt_gate(11, (unsigned int)isr11);
    set_idt_gate(12, (unsigned int)isr12);
    set_idt_gate(13, (unsigned int)isr13);
    set_idt_gate(14, (unsigned int)isr14);
    set_idt_gate(15, (unsigned int)isr15);
    set_idt_gate(16, (unsigned int)isr16);
    set_idt_gate(17, (unsigned int)isr17);
    set_idt_gate(18, (unsigned int)isr18);
    set_idt_gate(19, (unsigned int)isr19);
    set_idt_gate(20, (unsigned int)isr20);
    set_idt_gate(21, (unsigned int)isr21);
    set_idt_gate(22, (unsigned int)isr22);
    set_idt_gate(23, (unsigned int)isr23);
    set_idt_gate(24, (unsigned int)isr24);
    set_idt_gate(25, (unsigned int)isr25);
    set_idt_gate(26, (unsigned int)isr26);
    set_idt_gate(27, (unsigned int)isr27);
    set_idt_gate(28, (unsigned int)isr28);
    set_idt_gate(29, (unsigned int)isr29);
    set_idt_gate(30, (unsigned int)isr30);
    set_idt_gate(31, (unsigned int)isr31);
    set_idt_gate(32, (unsigned int)irq0);
    set_idt_gate(33, (unsigned int)irq1);
    set_idt_gate(34, (unsigned int)irq2);
    set_idt_gate(35, (unsigned int)irq3);
    set_idt_gate(36, (unsigned int)irq4);
    set_idt_gate(37, (unsigned int)irq5);
    set_idt_gate(38, (unsigned int)irq6);
    set_idt_gate(39, (unsigned int)irq7);
    set_idt_gate(40, (unsigned int)irq8);
    set_idt_gate(41, (unsigned int)irq9);
    set_idt_gate(42, (unsigned int)irq10);
    set_idt_gate(43, (unsigned int)irq11);
    set_idt_gate(44, (unsigned int)irq12);
    set_idt_gate(45, (unsigned int)irq13);
    set_idt_gate(46, (unsigned int)irq14);
    set_idt_gate(47, (unsigned int)irq15);

}

void initialise_GDT(void) {
    print("GDT A\n");
    gdtr.limit = sizeof(gdt) - 1;
    gdtr.base = (unsigned int) &gdt[0];
    print("GDT B\n");

    print("GDT size: ");
    print_hex_byte(sizeof(gdt));
    putchar('\n');

    print("GDTR limit: ");
    print_hex_byte(gdtr.limit & 0xFF);
    putchar(' ');
    print_hex_byte((gdtr.limit >> 8) & 0xFF);
    putchar('\n');
 
    // cuz base = 0x00000000 and cuz granularity = 0xCF it's 4 KiB units so max 0xFFFFF
    
    gdt[1].access = 0x9A;
    gdt[1].limit_low = 0xFFFF;
    gdt[1].granularity = 0xCF;
    gdt[1].base_low = 0;
    gdt[1].base_middle = 0;
    gdt[1].base_high = 0;
    
    gdt[2].access = 0x92;
    gdt[2].limit_low = 0xFFFF;
    gdt[2].granularity = 0xCF;
    gdt[2].base_low = 0;
    gdt[2].base_middle = 0;
    gdt[2].base_high = 0;
    
    unsigned char *bytes = (unsigned char *)&gdt[2];

    for (int i = 0; i < 8; i++) {
        print_hex_byte(bytes[i]);
        putchar(' ');
    }

    putchar('\n');
    clear();

    load_gdt();
}

void print_hex_byte(unsigned char value) {
    const char hex[] = "0123456789ABCDEF";

    putchar(hex[(value >> 4) & 0xF]);
    putchar(hex[value & 0xF]);
}

void print_hex_dword(unsigned int error_code) {
    const char hex[] = "0123456789ABCDEF";
    print("0x\0");
    for(int i=28; i>=0; i = i - 4){
        putchar(hex[(error_code >> i) & 0xF]);
    }
}

void exception_handler(unsigned int exception, unsigned int error_code)
{
    print("EXCEPTION: ");
    print(exception_names[exception]);
    print("\n");

    print("ERROR CODE: ");
    print_hex_dword(error_code);
    print("\n");

    while (1) {
        asm volatile ("hlt");
    }
}

void irq_handler(unsigned int irq){
    if (irq == 1) {
        keyboard_irq();
    }

    if (irq >= 8) {
        outb(0xA0, 0x20);
    }

    outb(0x20, 0x20);
}
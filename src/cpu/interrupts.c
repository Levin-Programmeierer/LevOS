#include <stdint.h>
#include "drivers/terminal.h"
#include "drivers/io.h"
#include "drivers/keyboard.h"

typedef struct {
    uint16_t lower_offset;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attributes;
    uint16_t higher_offset;
}__attribute__((packed)) IDTEntry;

IDTEntry IDT[256];

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) IDTR;

IDTR idtr;

typedef struct {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint32_t vector;
    uint32_t error_code;

    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
} ISRRegisters;

extern void isr_common_handler(ISRRegisters *regs);
extern void (*isr_table[256])(void);
extern void idt_load(const IDTR *idtr);

_Static_assert(sizeof(ISRRegisters) == 52,
               "ISRRegisters has incorrect layout");

typedef enum {
    IDT_GATETYPE_TASKGATE= 0x5, // offset must be 0 when this is applied
    IDT_GATETYPE_INTERRUPTGATE16= 0x6,
    IDT_GATETYPE_TRAPGATE16= 0x7,
    IDT_GATETYPE_INTERRUPTGATE32= 0xE,
    IDT_GATETYPE_TRAPGATE32= 0xF,

    IDT_RING0 = 0x0,
    IDT_RING1 = 0x1,
    IDT_RING2 = 0x2,
    IDT_RING3 = 0x3,

    IDT_PRESENT = 0x1,
    IDT_NOT_PRESENT = 0x0
} IDTFlags;

#define IDT_HIGHER_OFFSET(offset) (((offset) >> 16) & 0xFFFF)
#define IDT_LOWER_OFFSET(offset) ((offset) & 0xFFFF)
#define IDT_SELECTOR(selector) ((selector) & 0xFFFF)

#define IDT_ZERO() 0

#define IDT_TYPE_ATTRIBUTES(gate_type, privilege, present) \
    (((present) << 7) | ((privilege) << 5) | ((gate_type) & 0xF))

static void set_IDT(
    uint8_t vector,
    uint32_t offset,
    uint16_t selector,
    IDTFlags gate_type,
    IDTFlags privilege,
    IDTFlags present
) {
    IDT[vector].lower_offset = (uint16_t)(offset & 0xFFFF);
    IDT[vector].selector = selector;
    IDT[vector].zero = 0;

    IDT[vector].type_attributes =
        (uint8_t)(
            ((present & 0x1) << 7) |
            ((privilege & 0x3) << 5) |
            (gate_type & 0xF)
        );

    IDT[vector].higher_offset =
        (uint16_t)((offset >> 16) & 0xFFFF);
}

void init_IDT(void) {
    __asm__ volatile ("cli");

    for (uint32_t i = 0; i < 256; ++i) {
        uint32_t offset = (uint32_t)isr_table[i];

        IDT[i].lower_offset =
            (uint16_t)(offset & 0xFFFF);

        IDT[i].selector =
            0x08;

        IDT[i].zero =
            0;

        IDT[i].type_attributes =
            IDT_TYPE_ATTRIBUTES(
                IDT_GATETYPE_INTERRUPTGATE32,
                IDT_RING0,
                IDT_PRESENT
            );

        IDT[i].higher_offset =
            (uint16_t)((offset >> 16) & 0xFFFF);
    }

    idtr.limit = sizeof(IDT) - 1;
    idtr.base = (uint32_t)IDT;

    idt_load(&idtr);
    print("IDT successfully loaded!\n", GREEN);
}

void isr_common_handler(ISRRegisters *regs) {
    if (regs->vector == 0x21) {
        keyboard_irq();
        outb(0x20, 0x20);
    }

    outb(0x20, 0x20);
}
#include <stdint.h>
#include "drivers/terminal.h"

uint64_t IDT[256];
uint8_t InterruptVector;

struct {
    uint16_t lower_offset;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attributes;
    uint16_t higher_offset;
}__attribute__((packed)) IDTDescriptor;

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

    IDT_PRESENT = 0x1
};

#define IDT_OFFSET ()

void set_IDT(void){

}
#include "drivers/pci.h"
#include "drivers/terminal.h"
#include <stdint.h>

uint8_t slot = 2; // Current VGA device on QEMU

void readGraphicsCard(void){
    uint16_t command = pciConfigReadWord(0, slot, 0, 0x04);
    uint16_t header = pciConfigReadWord(0, slot, 0, 0x0E);
    // Read all BARs from gpu
    for (uint8_t i = 0; i < 6; i++) {
        uint32_t bar = pciConfigReadDWord(0, slot, 0, 0x10 + i * 4);

        print_hex_dword(bar);
        putchar('\n');
    }

    print_hex_dword(command);
    putchar('\n');
    print_hex_dword(header);
}
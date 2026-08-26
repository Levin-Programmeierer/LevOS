#include "drivers/pci.h"
#include <stdint.h>
#include "drivers/io.h"
#include "drivers/terminal.h"

uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset){
    uint32_t address;
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint16_t tmp = 0;

    // Create configuartion address
    address = (uint32_t)((lbus << 16) | (lslot << 11) | 
    (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    // Write the address
    outl(0xCF8, address);
    // read the data
    tmp = inl(0xCFC);
    return tmp;
}

uint32_t pciConfigReadDWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset){
    uint32_t address;
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint32_t tmp = 0;

    address = (uint32_t)((lbus << 16) | (lslot << 11) | 
    (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    outl(0xCF8, address);

    tmp = (uint32_t)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
    return tmp;
}

void pciConfigWriteWord(uint8_t bus,uint8_t slot,uint8_t func,uint8_t offset,uint32_t value) {
    uint32_t address;

    address =
        ((uint32_t)bus  << 16) |
        ((uint32_t)slot << 11) |
        ((uint32_t)func << 8)  |
        (offset & 0xFC) |
        0x80000000;

    outl(0xCF8, address);
    outl(0xCFC, value);
}

uint16_t pciCheckVendor(uint8_t bus, uint8_t slot){
    uint16_t vendor;

    // Try to read 1st config register, if 0xFFFF = non-existent device
    vendor = pciConfigReadWord(bus, slot, 0, 0);
    if (vendor != 0xFFFF) {
        (void)pciConfigReadWord(bus, slot, 0, 2);
    }

    return vendor;
}

void pciReadAllVendors(void){
    uint16_t vendor, device;
    for(uint8_t bus = 0; bus < 2; bus++){
        for(uint8_t slot = 0; slot < 32; slot++){
            vendor = pciConfigReadWord(bus, slot, 0, 0);
            print_hex_dword(vendor);
            device = pciConfigReadWord(bus, slot, 0, 2);
            putchar(' ');
            print_hex_dword(device);
            putchar(' ');
        }
    }
}

void pciReadVGADevice(uint8_t bus, uint8_t slot){
    uint16_t PCIClass = pciConfigReadWord(bus, slot, 0, 0x08);
    uint16_t Revision = pciConfigReadWord(bus, slot, 0, 0x0A);
    print_hex_dword(PCIClass); // first two are class last two are subclass so 0x03 and 00
    print_hex_dword(Revision);
}
#ifndef PCI_H
#define PCI_H
#include <stdint.h>

uint16_t pciCheckVendor(uint8_t bus, uint8_t slot);
uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void pciReadAllVendors(void);
void pciReadVGADevice(uint8_t bus, uint8_t slot);
void pciConfigWriteWord(uint8_t bus,uint8_t slot,uint8_t func,uint8_t offset,uint32_t value);
uint32_t pciConfigReadDWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);


#endif
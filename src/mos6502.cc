#include "mos6502.hh"
#include "bus.hh"

void MOS6502::connect_bus(Bus *b) {
    return;
}

void MOS6502::write(uint16_t addr, uint8_t data) {
    if (addr >= 0x0000 && addr <= 0xFFFF) {
        return this->bus->write(addr, data);
    }
}

uint8_t MOS6502::read(uint16_t addr) {
    if (addr >= 0x0000 && addr <= 0xFFFF) {
        return this->bus->read(addr);
    }
    return 0x00;
}
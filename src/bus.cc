#include "bus.hh"

Bus::Bus() {
    for (uint8_t &byte: mem) {
        byte = 0x00;
    }

}

void Bus::write(uint16_t addr, uint8_t data) {
    if (addr >= 0x0000 && addr <= 0xFFFF) {
        mem[addr] = data;
    }
}

uint8_t Bus::read(uint16_t addr) {
    if (addr >= 0x0000 && addr <= 0xFFFF) {
        return mem[addr];
    }
    return 0x00;
}
#include "bus.hh"

Bus::Bus() {
    for (uint8_t &byte: this->mem) {
        byte = 0x00;
    }
    this->cpu.connect_bus(this);
} 

Bus::~Bus() {

}

void Bus::write(uint16_t addr, uint8_t data) {
    if (addr >= 0x0000 && addr <= 0xFFFF) {
        this->mem[addr] = data;
    }
}

uint8_t Bus::read(uint16_t addr) {
    if (addr >= 0x0000 && addr <= 0xFFFF) {
        return this->mem[addr];
    }
    return 0x00;
}

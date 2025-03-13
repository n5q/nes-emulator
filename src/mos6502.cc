#include "mos6502.hh"

void MOS6502::connect_bus(Bus *b) {
    this.bus = b;
}
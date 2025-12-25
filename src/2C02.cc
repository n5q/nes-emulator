#include "2C02.hh"
#include "cartridge.hh"
#include <memory>

PPU::PPU() {
  
}

PPU::~PPU() {
  
}

// main bus
uint8_t PPU::cpu_read(uint16_t addr, bool readonly) {
  uint8_t data = 0x00;

  // https://www.nesdev.org/wiki/PPU_programmer_reference#Summary
  switch (addr) {
    case 0x0000: break; // PPUCTRL
    case 0x0001: break; // PPUMASK
    case 0x0002: break; // PPUSTATUS
    case 0x0003: break; // OAMADDR
    case 0x0004: break; // OAMDATA
    case 0x0005: break; // PPUSCROLL
    case 0x0006: break; // PPUADDR
    case 0x0007: break; // PPUDATA
  }
  
  return data;
}
void PPU::cpu_write(uint16_t addr, uint8_t data) {
  
  // https://www.nesdev.org/wiki/PPU_programmer_reference#Summary
  switch (addr) {
    case 0x0000: break; // PPUCTRL
    case 0x0001: break; // PPUMASK
    case 0x0002: break; // PPUSTATUS
    case 0x0003: break; // OAMADDR
    case 0x0004: break; // OAMDATA
    case 0x0005: break; // PPUSCROLL
    case 0x0006: break; // PPUADDR
    case 0x0007: break; // PPUDATA
  }
  return;
}

// ppu bus
uint8_t PPU::ppu_read(uint16_t addr, bool readonly ) {
  uint8_t data = 0x00;
  addr &= 0x3FFF;

  if (cart->ppu_read(addr, data)) {
    // do nothing
  }
  
  return data;
}
void PPU::ppu_write(uint16_t addr, uint8_t data) {
  addr &= 0x3FFF;

  if (cart->ppu_write(addr, data)) {
    // do nothing
  } 
}

void PPU::connect_cartridge(const std::shared_ptr<Cartridge> cartr) {
  this->cart = cartr;
}

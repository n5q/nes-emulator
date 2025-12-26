#ifndef _2C02_HH
#define _2C02_HH

#include "cartridge.hh"
#include <cstdint>
#include <memory>

class PPU {
public:
  PPU();
  ~PPU();

  // main bus
  uint8_t cpu_read(uint16_t addr, bool readonly = false);
  void cpu_write(uint16_t addr, uint8_t data);

  // ppu bus
  uint8_t ppu_read(uint16_t addr, bool readonly = false);
  void ppu_write(uint16_t addr, uint8_t data);

  void connect_cartridge(const std::shared_ptr<Cartridge> cart);
  void clk();

  bool nmi = false;
  
private:
  std::shared_ptr<Cartridge> cart;
  
  // 2kb vram (2 nametables)
  uint8_t nametable[2][1024];
  
  uint8_t palette_ram[32];

  uint16_t vram_addr = 0x0000;
  uint16_t vram_addr_tmp = 0x0000;
  uint8_t fine_x = 0x00;
  uint8_t address_latch = 0x00;
  uint8_t ppu_data_buf = 0x00;

  uint8_t ctrl = 0x00;    // $2000 PPUCTRL
  uint8_t mask = 0x00;    // $2001 PPUMASK
  uint8_t status = 0x00;  // $2002 PPUSTATUS

  int16_t scanline = 0; // 0 to 261
  int16_t cycle = 0;    // 0 to 340
};

#endif

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

private:
  std::shared_ptr<Cartridge> cart;
  
  // 2kb vram (2 nametables)
  uint8_t nametable[2][1024];
  
  uint8_t palette_ram[32];
};

#endif

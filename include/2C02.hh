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


  // BACKGROUND RENDERING

  // 1. background shift registers
  // pattern table for two tiles
  // lower = current tile being drawn
  uint16_t bg_shifter_pattern_lo = 0x0000;
  // high = next tile to be drawn
  uint16_t bg_shifter_pattern_hi = 0x0000;

  // 2. attribute shift registers
  // palette indeces for current and next tile
  // shifted so we know which palette to use for every pixel
  uint16_t bg_shifter_attrib_lo = 0x0000;
  uint16_t bg_shifter_attrib_hi = 0x0000;

  // 3. internal latches (next tile data)
  // during cycle, fetch 4 bytes of data for next tile
  uint8_t bg_next_tile_id = 0x00;
  uint8_t bg_next_tile_attrib = 0x00;
  uint8_t bg_next_tile_lsb = 0x00;
  uint8_t bg_next_tile_msb = 0x00;

  // bg rendering functions
  void load_bg_shifters();
  void update_shifters();
  void inc_scroll_x();
  void inc_scroll_y();
  void transfer_addr_x();
  void transfer_addr_y();
};

#endif

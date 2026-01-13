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

  // signal the cpu that a vblank nmi has occured
  bool nmi = false;
  bool frame_complete = false;
  

  // 256x240 screen buffer (RGBA)
  uint32_t* screen_buffer = new uint32_t[256*240];

  // palette to convert nes colours to rgb
  struct Pixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;
  };

  // generated with https://github.com/Gumball2415/pally
  Pixel palette_lut[0x3F + 1] = {
    {0x62, 0x62, 0x62},
    {0x10, 0x23, 0xb4},
    {0x25, 0x19, 0xca},
    {0x51, 0x18, 0xb4},
    {0x71, 0x12, 0x77},
    {0x7d, 0x0d, 0x1e},
    {0x71, 0x0d, 0x00},
    {0x4f, 0x24, 0x00},
    {0x21, 0x41, 0x00},
    {0x0f, 0x55, 0x00},
    {0x10, 0x5a, 0x00},
    {0x0e, 0x50, 0x1e},
    {0x0d, 0x3b, 0x77},
    {0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00},

    {0xac, 0xac, 0xac},
    {0x25, 0x55, 0xff},
    {0x57, 0x36, 0xff},
    {0x94, 0x2b, 0xff},
    {0xbf, 0x26, 0xc7},
    {0xce, 0x24, 0x54},
    {0xbe, 0x37, 0x00},
    {0x93, 0x5b, 0x00},
    {0x57, 0x80, 0x00},
    {0x28, 0x9a, 0x00},
    {0x25, 0xa1, 0x00},
    {0x22, 0x95, 0x54},
    {0x21, 0x79, 0xc7},
    {0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00},

    {0xff, 0xff, 0xff},
    {0x6d, 0xa3, 0xff},
    {0xa7, 0x7f, 0xff},
    {0xe4, 0x68, 0xff},
    {0xfa, 0x63, 0xff},
    {0xf9, 0x6b, 0xa8},
    {0xf9, 0x87, 0x2c},
    {0xe5, 0xae, 0x00},
    {0xaa, 0xd2, 0x00},
    {0x71, 0xec, 0x00},
    {0x4e, 0xf3, 0x2b},
    {0x44, 0xe7, 0xa8},
    {0x4b, 0xca, 0xff},
    {0x4c, 0x4c, 0x4c},
    {0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00},

    {0xff, 0xff, 0xff},
    {0xc5, 0xda, 0xff},
    {0xdc, 0xcc, 0xff},
    {0xf4, 0xc2, 0xff},
    {0xfc, 0xc0, 0xff},
    {0xfc, 0xc4, 0xdc},
    {0xfc, 0xcf, 0xae},
    {0xf4, 0xde, 0x8b},
    {0xdd, 0xec, 0x7e},
    {0xc5, 0xf6, 0x8b},
    {0xb4, 0xf9, 0xae},
    {0xae, 0xf5, 0xdc},
    {0xb4, 0xea, 0xff},
    {0xb9, 0xb9, 0xb9},
    {0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00},
  };


  // OAM (object attribute memory) (aka sprites)
  // https://www.nesdev.org/wiki/PPU_OAM
  struct OAM {
    // y pos
    uint8_t y;
    uint8_t tile_id;
    // flip, palette, priority
    uint8_t attribute;
    // x pos
    uint8_t x;
  } oam[64];

  uint8_t* oam_p = (uint8_t*) oam;
  
  // internal oam register
  uint8_t oam_addr = 0x00;

private:
  std::shared_ptr<Cartridge> cart;
  
  // 2kb vram (2 nametables) (background)
  uint8_t nametable[2][1024];
  
  uint8_t palette_ram[32];

  uint16_t vram_addr = 0x0000;
  uint16_t vram_addr_tmp = 0x0000;
  uint8_t fine_x = 0x00;
  uint8_t address_latch = 0x00;
  uint8_t ppu_data_buf = 0x00;

  uint8_t ctrl = 0x00;    // $2000 PPUCTRL
  uint8_t mask = 0x00;    // $2001 PPUMASK
  public:
  uint8_t status = 0x00;  // $2002 PPUSTATUS

  // nes screen is 256x240 but the ppu timing covers 361x262
  int16_t scanline = -1; // 0 to 261
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


  // -- FOREGROUND (SPRITE) RENDERING --


  // sprites on current scanline (max 8)
  OAM scanline_sprites[8];
  uint8_t n_sprites;

  // sprite shift registers (8 units, one for each potential visible sprite)
  uint8_t sprite_shifter_lo[8];
  uint8_t sprite_shifter_hi[8];

  bool possible_zerohit = false;
  bool rendering_zerohit = false;
      
  void clear_sprite_shifters();
  inline uint8_t reverse_bits(uint8_t b);
  
};

#endif

#include "2C02.hh"
#include "cartridge.hh"
#include <cstring>
#include <memory>

PPU::PPU() {
  
}

PPU::~PPU() {
  
}

// main bus
uint8_t PPU::cpu_read(uint16_t addr, bool readonly) {
  uint8_t data = 0x00;

  // https://www.nesdev.org/wiki/PPU_registers
  switch (addr) {
    case 0x0000: break; // PPUCTRL
    case 0x0001: break; // PPUMASK
    case 0x0002: break; // PPUSTATUS
    case 0x0003: break; // OAMADDR
    case 0x0004:  // OAMDATA
      // this reg should not be written to in most cases b/c oam should
      // be made only during vblank - use oamdma instaead
      data = oam_p[oam_addr];
      break;
      
    case 0x0005: break; // PPUSCROLL
    case 0x0006: break; // PPUADDR
    case 0x0007:  // PPUDATA
      data = this->ppu_data_buf;
      this->ppu_data_buf = this->ppu_read(this->vram_addr);

      // if addr in range, no delay
      // data returned immediately and buf replaced with mirrored NT
      if (this->vram_addr >= 0x3F00) {
        data = this->ppu_data_buf;
      }
      this->vram_addr += (this->ctrl & 0x04) ? 32 : 1;
  }
  
  return data;
}
void PPU::cpu_write(uint16_t addr, uint8_t data) {
  
  // https://www.nesdev.org/wiki/PPU_registers
  switch (addr) {
    case 0x0000: // PPUCTRL
      ctrl = data;
      // update NT bits 10,11 in t
      vram_addr_tmp = (vram_addr_tmp & 0xF3FF) | ((uint16_t)(data & 0x03) << 10);

      // if nmi and in vblank, trigger nmi
      if ((ctrl & 0x80) && (status & 0x80)) {
        nmi = true;
      }
      break;
      
    case 0x0001: // PPUMASK
      mask = data;
      break;
      
    case 0x0002: // PPUSTATUS
      // only care about top 3 bits, lower 5 are open bus
      data = (status & 0xE0) | (ppu_data_buf & 0x1F);
      // reading status clears vblank flag
      status &= ~0x80;
      address_latch = 0;
      break;
      
    case 0x0003: // OAMADDR
      oam_addr = data;
      break;
      
    case 0x0004: // OAMDATA
      oam_p[oam_addr] = data;
      // inc after every write
      oam_addr++;
      break;
      

    case 0x0005: // PPUSCROLL
      if (!address_latch) {
        // first write: x scroll
        fine_x = data & 0x07;
        // update coarse x in t register
        vram_addr_tmp = (vram_addr_tmp & 0xFFE0) | (data >> 3);
        address_latch = 1;
      }
      // second writeL y scroll
      else {
        // update fine and coarse y in t
        vram_addr_tmp = (vram_addr_tmp & 0x8FFF) | ((uint16_t)(data & 0x07) << 12);
        vram_addr_tmp = (vram_addr_tmp & 0xFC1F) | ((uint16_t)(data >> 3) << 5);
        address_latch = 0;
      }
      break;
      
    case 0x0006: // PPUADDR
      // write high byte first then low byte
      if (!this->address_latch) {
        this->vram_addr_tmp = (this->vram_addr_tmp & 0x00FF) | ((uint16_t)(data & 0x3F) << 8);
      }
      else {
        this->vram_addr_tmp = (this->vram_addr_tmp & 0xFF00) | data;

        this->vram_addr = this->vram_addr_tmp;
        this->address_latch = 0;
      }
      break;
      
    case 0x0007: // PPUDATA
      this->ppu_write(this->vram_addr, data);
      // if ppuctrl bit 2 is 0, increment across (1) otherwise down (32)
      this->vram_addr += (this->ctrl & 0x04) ? 32 : 1;
  }
  return;
}

// ppu bus
uint8_t PPU::ppu_read(uint16_t addr, bool readonly ) {
  uint8_t data = 0x00;
  addr &= 0x3FFF;

  // 1. pattern tables (0x0000 - 0x1FFF)
  // ask the cartridge first, if true then its handled the read
  if (cart->ppu_read(addr, data)) {
    return data;
  }

  // 2. nametables (0x2000 - 0x3EFF)
  // 4 logical NT addresses, mapped based on mirror mode
  if (addr >= 0x2000 && addr <= 0x3EFF) {
    addr &= 0x0FFF;

    if (cart->mirror == Cartridge::VERTICAL) {
      if (addr >= 0x0000 && addr <= 0x03FF) {
        data = nametable[0][addr & 0x03FF];
      }
      if (addr >= 0x0400 && addr <= 0x07FF) {
        data = nametable[1][addr & 0x03FF];
      }
      if (addr >= 0x0800 && addr <= 0x0BFF) {
        data = nametable[0][addr & 0x03FF];
      }
      if (addr >= 0x0C00 && addr <= 0x0FFF) {
        data = nametable[1][addr & 0x03FF];
      }
    }

    else if (cart->mirror == Cartridge::HORIZONTAL) {
      if (addr >= 0x0000 && addr <= 0x03FF) {
        data = nametable[0][addr & 0x03FF];
      }
      if (addr >= 0x0400 && addr <= 0x07FF) {
        data = nametable[0][addr & 0x03FF];
      }
      if (addr >= 0x0800 && addr <= 0x0BFF) {
        data = nametable[1][addr & 0x03FF];
      }
      if (addr >= 0x0C00 && addr <= 0x0FFF) {
        data = nametable[1][addr & 0x03FF];
      }
    }
    
  }

  // 3. palette ram (0x3F00 - 0x3FFF)
  else if (addr >= 0x3F00 && addr <= 0x3FFF) {
    addr &= 0x001F; // mask to 0-31
    // special cases: 3F10, 3F14, 3F18, 3F1C are mirrors of 3F00
    switch (addr) {
      case 0x0010: addr = 0x0000; break;
      case 0x0014: addr = 0x0004; break;
      case 0x0018: addr = 0x0008; break;
      case 0x001C: addr = 0x000C; break;
    }

    data = this->palette_ram[addr];
  }
  
  return data;
}
void PPU::ppu_write(uint16_t addr, uint8_t data) {
  addr &= 0x3FFF;

  // 1. pattern tables (0x0000 - 0x1FFF)
  if (cart->ppu_write(addr, data)) {
    return; // cart handled it
  }

  // 2. nametables (0x2000 - 0x3EFF)
  if (addr >= 0x2000 && addr <= 0x3EFF) {
    addr &= 0x0FFF;
    
    if (cart->mirror == Cartridge::VERTICAL) {
      if (addr >= 0x0000 && addr <= 0x03FF) {
        nametable[0][addr & 0x03FF] = data;
      }
      if (addr >= 0x0400 && addr <= 0x07FF) {
        nametable[1][addr & 0x03FF] = data;
      }
      if (addr >= 0x0800 && addr <= 0x0BFF) {
        nametable[0][addr & 0x03FF] = data;
      }
      if (addr >= 0x0C00 && addr <= 0x0FFF) {
        nametable[1][addr & 0x03FF] = data;
      }
    }

    else if (cart->mirror == Cartridge::HORIZONTAL) {
      if (addr >= 0x0000 && addr <= 0x03FF) {
        nametable[0][addr & 0x03FF] = data;
      }
      if (addr >= 0x0400 && addr <= 0x07FF) {
        nametable[0][addr & 0x03FF] = data;
      }
      if (addr >= 0x0800 && addr <= 0x0BFF) {
        nametable[1][addr & 0x03FF] = data;
      }
      if (addr >= 0x0C00 && addr <= 0x0FFF) {
        nametable[1][addr & 0x03FF] = data;
      }
    }
    
  }
  
  // 3. palette ram (0x3F00 - 0x3FFF)
  else if (addr >= 0x3F00 && addr <= 0x3FFF) {
    addr &= 0x001F; // mask to 0-31
    // special cases: 3F10, 3F14, 3F18, 3F1C are mirrors of 3F00
    switch (addr) {
      case 0x0010: addr = 0x0000; break;
      case 0x0014: addr = 0x0004; break;
      case 0x0018: addr = 0x0008; break;
      case 0x001C: addr = 0x000C; break;
    }

    this->palette_ram[addr] = data;
  }
}

void PPU::connect_cartridge(const std::shared_ptr<Cartridge> cartr) {
  this->cart = cartr;
}

// void PPU::clk() {
//   this->cycle++;

//   if (cycle >= 341) {
//     cycle = 0;
//     scanline++;

//     if (scanline > 262) {
//       scanline = 0;
//       // frame complete
//     }
//   }

//   // 2. handle status flags and nmi
//   if (scanline == 241 && cycle == 1) {
//     status |= 0x80; // set vblank flag (bit 7)

//     // if nmi enabled in ctrl (bit 7), trigger
//     if (ctrl & 0x80) {
//       nmi = true;
//     }
//   }

//   // scanline 261 (or -1): pre render
//   // line before visible frame starts
//   if (scanline == 261 && cycle == 1) {
//     status &= ~0x80; // clear vblank
//     status &= ~0x40; // clear sprite 0 hit
//     status &= ~0x20; // clear sprite overflow
    
//     // TODO: clear internal shifters
//   }
// }

void PPU::clk() {
  // -- RENDERING PHASE --
  // visible scanlines (0-239) and pre render line (261/-1)
  if (scanline >= -1 && scanline < 240) {
    
    // scanline 261/-1 (pre render): clear flags at dot 1
    if (scanline == 0 && cycle == 0) {
      cycle = 1;
    }
    if (scanline == -1 && cycle == 1) {
      status &= ~0x80; // clear vblank
      status &= ~0x40; // clear sprite 0
      status &= ~0x20; // clear overflow
    }
    // reset shifters
    bg_shifter_pattern_lo = 0;
    bg_shifter_pattern_hi = 0;
    bg_shifter_attrib_lo = 0;
    bg_shifter_attrib_hi = 0;

    // cycles 1-256 (visible) and 321-336 (prefetch for next line)
    if ((cycle >= 2 && cycle < 258) || (cycle >= 321 && cycle < 338)) {
      this->update_shifters();

      // -- PIXEL OUTPUT & MUX --
      // 1. bg pixel
      uint8_t bg_pixel = 0x00;   // pixel value (0,1,2,3)
      uint8_t bg_palette = 0x00; // palette index (0,1,2,3)
     
      // only generate pixels during visible window
      if (mask & 0x08) {
        // pixel to draw is at msb shifted by fine x
        uint16_t mux = 0x8000 >> fine_x;
        uint8_t p0 = (bg_shifter_pattern_lo & mux) > 0;
        uint8_t p1 = (bg_shifter_pattern_hi & mux) > 0;
        bg_pixel = (p1 << 1) | p0;

        uint8_t pal0 = (bg_shifter_attrib_lo & mux) > 0;
        uint8_t pal1 = (bg_shifter_attrib_hi & mux) > 0;
        bg_palette = (pal1 << 1) | pal0;
      }

      // 2. fg (sprite) pixel
      uint8_t fg_pixel = 0x00;
      uint8_t fg_palette = 0x00;
      uint8_t fg_priority = 0x00;

      if (mask & 0x10) {
        rendering_zerohit = false;
        // check sprite enable (bit 4)
        for (uint8_t i = 0; i < n_sprites; i++) {
          if (scanline_sprites[i].x == 0) {
            // read msb of sprite shifters
            uint8_t pixel_lo = (sprite_shifter_lo[i] & 0x80) > 0;
            uint8_t pixel_hi = (sprite_shifter_hi[i] & 0x80) > 0;
            fg_pixel = (pixel_hi << 1) | pixel_lo;
            // sprite palettes + 4
            fg_palette = (scanline_sprites[i].attribute & 0x03) + 0x04;
            // 0 == front
            fg_priority = (scanline_sprites[i].attribute & 0x20) == 0;
            // found topmost
            if (fg_pixel) {
              // sprite zero?
              if (!i && possible_zerohit) {
                rendering_zerohit = true;
              }
              break;
            }
          }
        }
      }

      // 3. priority mux (combing fg and bg)
      
      uint8_t pixel = 0x00;
      uint8_t palette = 0x00;

      if (!fg_pixel && !bg_pixel) {
        pixel = 0x00;
        palette = 0x00;
      }

      else if (fg_pixel && !bg_pixel) {
        pixel = fg_pixel;
        palette = fg_palette;
      }

      else if (!fg_pixel && bg_pixel) {
        pixel = bg_pixel;
        palette = bg_palette;
      }

      else if (fg_pixel && bg_pixel){
        if (fg_priority) {
          pixel = fg_pixel;
          palette = fg_palette;
        }
        else {
          pixel = bg_pixel;
          palette = bg_palette;
        }
        // zero hit detection
        if (possible_zerohit && rendering_zerohit) {
          // mask render foreground and render background (bits 4 and 3)
          if ((mask & 0x08) && (mask & 0x10)) {
            // left edge has special flags to fix inconsistencies when scrolling
            // mask bits 1 and 2
            if ( !((mask & 0x02) | (mask & 0x04)) ) {
              if (cycle >= 9 && cycle < 258) {
                // set the sprite zero hit flag
                status |= 0x40;
              }
            }
            else {
              if (cycle >= 1 && cycle < 258) {
                // set the sprite zero hit flag
                status |= 0x40;
              }
            }
          }
        }
      }

              
      // 4. output to screen buf
      // calculate final colour
      // index = 0x3F00 + (palette * 4) + pixel
      // visible screen area
      if (cycle < 257 && scanline >= 0) {
        uint16_t palette_addr = 0x3F00 + (palette * 4) + pixel;
        // if pixel is 0, always points to 0x3F00 (bg colour)
        if (!(pixel & 0x03)) {
          palette_addr = 0x3F00;
        }
        uint8_t colour_index = ppu_read(palette_addr) & 0x3F;
        Pixel colour = palette_lut[colour_index];
        
        this->screen_buffer[(scanline * 256) + (cycle - 1)] = (colour.r << 24)
                                                            | (colour.g << 16)
                                                            | (colour.b << 8)
                                                            | 255;
      }

      // 5. update sprite shifters (dec x or shift)
      if (mask & 0x10) {
        for (int i = 0; i < n_sprites; i++) {
          if (scanline_sprites[i].x > 0) {
            scanline_sprites[i].x--;
          }
          else {
            sprite_shifter_lo[i] <<= 1;
            sprite_shifter_hi[i] <<= 1;
          }
        }
      }

      


      // Cycle logic operates on steps of 8 pixels
      // 1. fetch NT byte (title id)
      // 2. fetch AT byte (attributes)
      // 3. fetch low pattern byte
      // 4. fetch high pattern byte

      switch ((cycle - 1) % 8) {
        case 0:
          this->load_bg_shifters();
          // fetch next title id from NT
          // addr: 0x2000 | (v & 0x0FFF)
          bg_next_tile_id = ppu_read(0x2000 | (vram_addr & 0x0FFF));
          break;
          
        case 2:
          // fetch next title attr from AT
          // logical shortcut - not exact ppu behaviour
          // TODO: improve?
          bg_next_tile_attrib = ppu_read(0x23C0 | (vram_addr & 0x0C00)
                                            | ((vram_addr >> 4) & 0x38)
                                            | ((vram_addr >> 2) & 0x07));
          // shift to get the quadrant we need
          if (vram_addr & 0x0040) {
            bg_next_tile_attrib >>= 4;
          }
          if (vram_addr & 0x0002) {
            bg_next_tile_attrib >>= 2;
          }
          bg_next_tile_attrib &= 0x03;
          break;
          
        case 4:
          // fetch low byte from PT
          // addr: (ctrl4 << 12) + (titleid * 16) + fine_y
          bg_next_tile_lsb = ppu_read(((ctrl & 0x10) << 8)
                                      + ((uint16_t) bg_next_tile_id << 4)
                                      + ((vram_addr >> 12) & 0x07));
          break;
          
        case 6:
          // fetch high byte from PT
          // addr: (ctrl4 << 12) + (titleid * 16) + fine_y + 8
          bg_next_tile_msb = ppu_read(((ctrl & 0x10) << 8)
                                      + ((uint16_t) bg_next_tile_id << 4)
                                      + ((vram_addr >> 12) & 0x07)
                                      + 8);
          break;
          
        case 7:
          inc_scroll_x();
          break;
          
      }
    }

    // end of visible line: increment y
    if (cycle == 256) {
      inc_scroll_y();
    }

    // reset x at start of each line (dot 257)
    if (cycle == 257) {
      load_bg_shifters();
      transfer_addr_x();

      // SPRITE EVALUATION:
      // 1. clear list of visible sprites
      n_sprites = 0;
      possible_zerohit = false;
      for (int i = 0; i < 8; i++) {
        // 0xFF means no sprite
        scanline_sprites[i].tile_id = 0xFF;
        scanline_sprites[i].x = 0xFF;
        scanline_sprites[i].y = 0xFF;
        scanline_sprites[i].attribute = 0xFF;
      }
      clear_sprite_shifters();

      // 2. iterate through OAM to find sporites on this scanline
      uint8_t oam_entries = 0;
      while (oam_entries < 64 && n_sprites < 8) {
        // diff between current scanline and sprite y
        int16_t diff = ((int16_t) scanline - (int16_t) oam[oam_entries].y);
        // check if sprite is visible (diff between 0-8 or 0-16 for 8x16 mode)
        int16_t spr_height = (ctrl & 0x20) ? 16 : 8;
        if (diff >= 0 && diff < spr_height) {
          // sprite visible - copy to secondary oam
          if (n_sprites < 8) {
            // zero hit
            // TODO: flag check logic
            if (oam_entries == 0) {
              possible_zerohit = true;
            }
            memcpy(&scanline_sprites[n_sprites], &oam[oam_entries], sizeof(OAM));
            n_sprites++;
          }
        }
        oam_entries++;
      }

      // 3. set sprite overflow flag (status but 5)
      // real implementation is bugged but here the bug is ignored for simplicity
      // no games make use of the bug
      // https://www.nesdev.org/wiki/PPU_sprite_evaluation#Sprite_overflow_bug
      status &= ~0x20;
      if (n_sprites >= 8) {
        status |= 0x20;
      }
    }
    
    // sprite shifter loading
    if (cycle == 340) {
      // prep shifters for next scanline
      for (uint8_t i = 0; i < n_sprites; i++) {
        uint8_t spr_pattern_bits_lo;
        uint8_t spr_pattern_bits_hi;
        uint16_t spr_pattern_addr;

        // determine address of sprite pattern
        // 8x8 mode
        if (!(ctrl & 0x20)) {
          // bit 3 tells which pattern table
          uint16_t table_addr = (ctrl & 0x08) ? 0x1000 : 0x0000;
          uint16_t cell_addr = (uint16_t) scanline_sprites[i].tile_id << 4;
          // row within title ( handling vflip)
          uint16_t row_addr = (scanline - scanline_sprites[i].y);
          if (scanline_sprites[i].attribute & 0x80) {
            row_addr = 7 - row_addr;
          }
          spr_pattern_addr = table_addr + cell_addr + row_addr;
        }

        // TODO: 8x16 mode
        else {
          spr_pattern_addr = 0;
        }

        // fetch data
        spr_pattern_bits_lo = ppu_read(spr_pattern_addr);
        spr_pattern_bits_hi = ppu_read(spr_pattern_addr + 8);

        // hflip (reverse byte)
        if (scanline_sprites[i].attribute & 0x40) {
          spr_pattern_bits_lo = reverse_bits(spr_pattern_bits_lo);
          spr_pattern_bits_hi = reverse_bits(spr_pattern_bits_hi);
        }

        // load shifters
        sprite_shifter_lo[i] = spr_pattern_bits_lo;
        sprite_shifter_hi[i] = spr_pattern_bits_hi;
      }
    }

    

    // pre render line: reset y at end of pre render (dot 280-304)
    if (scanline == -1 && cycle >= 280 && cycle < 305) {
      transfer_addr_y();
    }
  }

  // -- VBLANK PHASE --
  if (scanline == 241 && cycle == 1) {
    status |= 0x80;
    if (ctrl & 0x80) {
      nmi = true;
    }
  }

  // advance counters
  cycle++;
  if (cycle >= 341) {
    cycle = 0;
    scanline++;
    if (scanline >= 261) {
      scanline = -1;
    }
  }
}


// -- BACKGROUND RENDERING --

// transfers the latched next tile into the upper end of the 16 bit shifters
void PPU::load_bg_shifters() {
  bg_shifter_pattern_lo = (bg_shifter_pattern_lo & 0xFF00) | bg_next_tile_lsb;
  bg_shifter_pattern_hi = (bg_shifter_pattern_hi & 0xFF00) | bg_next_tile_msb;
  // expand 1 bit attrs into 8 bits
  bg_shifter_attrib_lo = (bg_shifter_attrib_lo & 0xFF00) | ((bg_next_tile_attrib & 0b01) ? 0xFF : 0x00);
  bg_shifter_attrib_hi = (bg_shifter_attrib_hi & 0xFF00) | ((bg_next_tile_attrib & 0b10) ? 0xFF : 0x00);
}

void PPU::update_shifters() {
  // only if rendering is enabled
  if (mask & 0x18) {
    bg_shifter_pattern_lo <<= 1;
    bg_shifter_pattern_hi <<= 1;
    bg_shifter_attrib_lo <<= 1;
    bg_shifter_attrib_hi <<= 1;
  }
}

void PPU::inc_scroll_x() {
  // if rendering enabled
  if (mask & 0x18) {
    if ((vram_addr & 0x001F) == 31) {
      vram_addr &= ~0x001F; // reset coarse x
      vram_addr ^= 0x0400;  // switch horizontal NT
    }
    else {
      vram_addr++;
    }
  }
}

void PPU::inc_scroll_y() {
  // if rendering enabled
  if (mask & 0x18) {
    uint8_t fine_y = (vram_addr & 0x7000) >> 12;
    if (fine_y < 7) {
      // inc fine y
      vram_addr += 0x1000;
    }
    else {
      // reset fine y
      vram_addr &= ~0x7000;
      uint8_t coarse_y = (vram_addr & 0x03E0) >> 5;
      if (coarse_y == 29) {
        coarse_y = 0;
        // switch vertical NT
        vram_addr ^= 0x0800;
      }
      else if (coarse_y == 31) {
        coarse_y = 0;
      }
      else {
        coarse_y++;
      }
      vram_addr = (vram_addr & ~0x03E0) | (coarse_y << 5);
    }
  }
}

// reset horizontal scroll bits from temp register
void PPU::transfer_addr_x() {
  // if rendering enabled
  if (mask & 0x18) {
    vram_addr = (vram_addr & ~0x041F) | (vram_addr_tmp & 0x041F);
  }
}

// reset vertical scroll bits from temp register
void PPU::transfer_addr_y() {
  // if rendering enabled
  if (mask & 0x18) {
    vram_addr = (vram_addr & ~0x7BE0) | (vram_addr_tmp & 0x7BE0);
  }
}


// -- FOREGROUND (SPRITE) RENDERING --

void PPU::clear_sprite_shifters() {
  for (int i = 0; i < 8; i++) {
    sprite_shifter_lo[i] = 0;
    sprite_shifter_hi[i] = 0;
  }
}


// https://stackoverflow.com/questions/2602823/in-c-c-whats-the-simplest-way-to-reverse-the-order-of-bits-in-a-byte
inline uint8_t PPU::reverse_bits(uint8_t b) {
  b = ((b & 0xF0) >> 4) | ((b & 0x0F) << 4);
  b = ((b & 0xCC) >> 2) | ((b & 0x33) << 2);
  b = ((b & 0xAA) >> 1) | ((b & 0x55) << 1);
  return b;
}

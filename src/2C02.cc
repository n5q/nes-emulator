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
  
  // https://www.nesdev.org/wiki/PPU_programmer_reference#Summary
  switch (addr) {
    case 0x0000: // PPUCTRL
      ctrl = data;
      this->vram_addr_tmp = (this->vram_addr_tmp & 0xF3FF) | ((data & 0x03) << 10);

      // if nmi and in vblank, trigger nmi
      if ((ctrl & 0x80) && (status & 0x80)) {
        nmi = true;
      }
      break;
      
    case 0x0001: break; // PPUMASK
    case 0x0002: // PPUSTATUS
      // only care about top 3 bits, lower 5 are open bus
      data = (status & 0xE0) | (ppu_data_buf & 0x1F);
      // reading status clears vblank flag
      status &= ~0x80;
      address_latch = 0;
      break;
      
    case 0x0003: break; // OAMADDR
    case 0x0004: break; // OAMDATA
    case 0x0005: break; // PPUSCROLL
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
          bg_next_tile_id = ppu_read(0x23C0 | (vram_addr & 0x0C00)
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


// BACKGROUND RENDERING

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


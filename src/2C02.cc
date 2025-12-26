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

void PPU::clk() {
  this->cycle++;

  if (cycle >= 341) {
    cycle = 0;
    scanline++;

    if (scanline > 262) {
      scanline = 0;
      // frame complete
    }
  }

  // 2. handle status flags and nmi
  if (scanline == 241 && cycle == 1) {
    status |= 0x80; // set vblank flag (bit 7)

    // if nmi enabled in ctrl (bit 7), trigger
    if (ctrl & 0x80) {
      nmi = true;
    }
  }

  // scanline 261 (or -1): pre render
  // line before visible frame starts
  if (scanline == 261 && cycle == 1) {
    status &= ~0x80; // clear vblank
    status &= ~0x40; // clear sprite 0 hit
    status &= ~0x20; // clear sprite overflow
    
    // TODO: clear internal shifters
  }
}

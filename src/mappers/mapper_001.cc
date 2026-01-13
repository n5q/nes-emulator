#include "mappers/mapper_001.hh"
#include "mappers/mapper_template.hh"
#include <cstdint>

Mapper_001::Mapper_001(uint8_t banks_PRG, uint8_t banks_CHR, std::function<void(uint8_t)> cb)  
  : Mapper_Template(banks_PRG, banks_CHR), mirror_callback(cb) {
  prg_ram.resize(8*1024, 0x00);
  reset();
}

Mapper_001::~Mapper_001() {
}

void Mapper_001::reset() {
  shift_register = 0x00;
  shift_count = 0;
  
  // power up state: control = 0x0C (mode 3: fix first switch last)
  control_register = 0x0C;
  
  chr_bank_0 = 0;
  chr_bank_1 = 0;
  prg_bank = 0;
  
  // parse initial control register state
  mirroring = control_register & 0x03;
  prg_bank_mode = (control_register >> 2) & 0x03;
  chr_bank_mode = (control_register >> 4) & 0x01;

  mirroring = control_register & 0x03;
  // notify cart
  if (mirror_callback) {
    mirror_callback(mirroring);
  }
}

bool Mapper_001::cpu_mapread(uint16_t addr, uint32_t &addr_mapped) {
  // PRG-RAM (8kb)
  if (addr >= 0x6000 && addr <= 0x7FFF) {
    // map to 8kb prg ram
    addr_mapped = addr & 0x1FFF;
    return true;
  }

  // PRG-ROM
  if (addr >= 0x8000 && addr <= 0xFFFF) {
    if (prg_bank_mode == 0 || prg_bank_mode == 1) {
      // 32KB mode - switch 32KB at 0x8000
      // ignore lowest bit of index for 32KB alignment
      addr_mapped = ((prg_bank & 0xFE) * 0x4000) + (addr & 0x7FFF);
    }
    else if (prg_bank_mode == 2) {
      // fix first bank at 0x8000, switch 16KB bank at 0xC000
      if (addr < 0xC000) {
        // fixed to bank 0
        addr_mapped = addr & 0x3FFF; 
      }
      else {
        addr_mapped = (prg_bank * 0x4000) + (addr & 0x3FFF);
      }
    }
    else if (prg_bank_mode == 3) {
      // switch 16KB bank at 0x8000, fix last bank at 0xC000
      if (addr < 0xC000) {
        addr_mapped = (prg_bank * 0x4000) + (addr & 0x3FFF);
      }
      else {
        addr_mapped = ((banks_PRG - 1) * 0x4000) + (addr & 0x3FFF);
      }
    }
    return true;
  }
  return false;
}
bool Mapper_001::cpu_mapwrite(uint16_t addr, uint32_t &addr_mapped, uint8_t data) {
  // PRG-RAM (writeable)
  if (addr >= 0x6000 && addr <= 0x7FFF) {
    addr_mapped = addr & 0x1FFF;
    return true;
  }

  // PRG-ROM
  if (addr >= 0x8000 && addr <= 0xFFFF) {
    // 1. reset logic: if bit 7 is set, reset shift register
    if (data & 0x80) {
      shift_register = 0x00;
      shift_count = 0;
      control_register = control_register | 0x0C;
      mirroring = control_register & 0x03;
      if (mirror_callback) {
        mirror_callback(mirroring);
      }
    } 
    // 2. shift logic: LSB of data enters MSB of shift register? 
    else {
      shift_register >>= 1;
      shift_register |= ((data & 0x01) << 4);
      shift_count++;

      // 3. register update: if 5 bits have been written
      if (shift_count == 5) {
        uint8_t target = (addr >> 13) & 0x03;
        uint8_t output = shift_register; 

        // 0x8000 - 0x9FFF: Control Register
        if (target == 0) { 
          control_register = output & 0x1F;
          mirroring = control_register & 0x03;
          if (mirror_callback) {
            mirror_callback(mirroring);
          }
          prg_bank_mode = (control_register >> 2) & 0x03;
          chr_bank_mode = (control_register >> 4) & 0x01;
        } 
        else if (target == 1) { 
          chr_bank_0 = output & 0x1F;
        } 
        else if (target == 2) { 
          chr_bank_1 = output & 0x1F;
        } 
        else if (target == 3) { 
          prg_bank = output & 0x1F; 
        }

        // reset shift register after use
        shift_register = 0x00;
        shift_count = 0;
      }
    }
    return false;
  }
  return false;
}

bool Mapper_001::ppu_mapread(uint16_t addr, uint32_t &addr_mapped) {
  // CHR-RAM
  if (addr >= 0x0000 && addr <= 0x1FFF) {
    if (banks_CHR == 0) {
      addr_mapped = addr;
      return true;
    }

    if (chr_bank_mode == 0) {
      // 8KB mode - switch entire 8KB at once
      addr_mapped = ((chr_bank_0 & 0xFE) * 0x2000) + (addr & 0x1FFF);
    } 
    else {
      // 4KB mode - switch two separate 4KB banks
      if (addr < 0x1000) {
        addr_mapped = (chr_bank_0 * 0x1000) + (addr & 0x0FFF);
      } 
      else {
        addr_mapped = (chr_bank_1 * 0x1000) + (addr & 0x0FFF);
      }
    }
    return true;
  }
  return false;
}

bool Mapper_001::ppu_mapwrite(uint16_t addr, uint32_t &addr_mapped) {
  if (addr >= 0x0000 && addr <= 0x1FFF) {
    if (banks_CHR == 0) {
      addr_mapped = addr;
      return true;
    }
    // Cannot write to CHR-ROM
    return false;
  }
  return false;
}

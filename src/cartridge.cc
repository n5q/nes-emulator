#include "cartridge.hh"
#include "mappers/mapper_000.hh"
#include <memory>



Cartridge::Cartridge(const std::string &romfile) {

  // https://www.nesdev.org/wiki/INES
  typedef struct {
    char header_start[4];  // constant 'N', 'E', 'S', EOF
    uint8_t PRG_ROM_size;  // PRG ROM size in 16kb units
    uint8_t CHR_ROM_size;  // CHR ROM size in 8kb units
    uint8_t flags6;   // Mapper, mirroring, battery, trainer
    uint8_t flags7;   // Mapper, VS/Playchoice, NES 2.0
    uint8_t flags8;   // PRG-RAM size (rarely used extension)
    uint8_t flags9;   // TV system (rarely used extension)
    uint8_t flags10;  // TV system, PRG-RAM presence (unofficial, rarely used extension)
    char unused[5];   // unused - should be zeroed but some rippers put their name across bytes 7-15
  } ines_header_T;

  ines_header_T header;

  std::ifstream ifs;
  ifs.open(romfile, std::ifstream::binary);
  if (ifs.is_open()) {
    ifs.read((char*)&header, sizeof(ines_header_T));

    // dont care about trainer bytes
    if (header.flags6 & 0x04) {
      ifs.seekg(512, std::ios_base::cur);
    }

    mapper_id = ((header.flags6 & 0xF0) >> 4) | (header.flags7 & 0xF0);

    if (header.flags6 & 0x01) {
      mirror = VERTICAL;
    }
    else {
      mirror = HORIZONTAL;
    }

    uint8_t romtype = 1;
    switch (romtype) {
      case 0:
        break;
        
      case 1:
        banks_PRG = header.PRG_ROM_size;
        mem_PRG.resize(banks_PRG*16*1024);
        ifs.read( (char*) mem_PRG.data(), mem_PRG.size());

        banks_CHR = header.CHR_ROM_size;
        if (banks_CHR == 0) {
          // create new 8kb ram for chr
          mem_CHR.resize(8*1024);
        }
        else {
          mem_CHR.resize(banks_CHR*8*1024);
        }
        if (banks_CHR > 0) {
          ifs.read( (char*) mem_CHR.data(), mem_CHR.size());
        }
        break;
        
      case 2:
        break;
    }

    switch (mapper_id) {
      case 0: mapper = std::make_shared<Mapper_000>(banks_PRG, banks_CHR); break;
      case 1: mapper = std::make_shared<Mapper_001>(banks_PRG, banks_CHR, [&](uint8_t mode) {
        switch (mode) {
          case 0: mirror = ONESCREEN_LO; break;
          case 1: mirror = ONESCREEN_HI; break;
          case 2: mirror = VERTICAL; break;
          case 3: mirror = HORIZONTAL; break;
      }});
      break;
    }

    valid = true;
    ifs.close();
  }
}


Cartridge::~Cartridge() {
  
}

bool Cartridge::cpu_read(uint16_t addr, uint8_t &data) {
  uint32_t addr_mapped = 0;
  if (mapper->cpu_mapread(addr, addr_mapped)) {
      // for PRG-RAM read from mapper internal ram
      if (addr >= 0x6000 && addr <= 0x7FFF && mapper_id == 1) {
        auto mapper_001 = std::dynamic_pointer_cast<Mapper_001>(mapper);
        if (mapper_001) {
          data = mapper_001->prg_ram[addr_mapped];
          return true;
        }
    }
    else {
      data = this->mem_PRG[addr_mapped];
      return true;
    }
  }
  return false;
}

bool Cartridge::cpu_write(uint16_t addr, uint8_t data) {
  uint32_t addr_mapped = 0;
  if (mapper->cpu_mapwrite(addr, addr_mapped, data)) {
      // for prg-ram write to mappers internal ram
      if (addr >= 0x6000 && addr <= 0x7FFF && mapper_id == 1) {
        auto mapper_001 = std::dynamic_pointer_cast<Mapper_001>(mapper);
        if (mapper_001) {
          mapper_001->prg_ram[addr_mapped] = data;
          return true;
        }
      }
    else {
      this->mem_PRG[addr_mapped] = data;
      return true;
    }
  }
  return false;
}

bool Cartridge::ppu_read(uint16_t addr, uint8_t &data) {
  uint32_t addr_mapped = 0;
  if (mapper->ppu_mapread(addr, addr_mapped)) {
    data = this->mem_CHR[addr_mapped];
    return true;
  }
  return false;
}

bool Cartridge::ppu_write(uint16_t addr, uint8_t data) {
  uint32_t addr_mapped = 0;
  if (mapper->ppu_mapwrite(addr, addr_mapped)) {
    this->mem_CHR[addr_mapped] = data;
    return true;
  }
  return false;
}


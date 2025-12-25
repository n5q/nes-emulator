#include "mappers/mapper_000.hh"
#include "mappers/mapper_template.hh"
#include <cstdint>

Mapper_000::Mapper_000(uint8_t banks_PRG, uint8_t banks_CHR) : Mapper_Template(banks_PRG, banks_CHR) {
  
}

Mapper_000::~Mapper_000() {
  
}


bool Mapper_000::cpu_mapread(uint16_t addr, uint32_t &addr_mapped) {
  if (addr >= 0x8000 && addr <= 0xFFFF) {
    addr_mapped = addr & (this->banks_PRG > 1 ? 0x7FFF : 0x3FFF);
    return true;
  }
  return false;
}

bool Mapper_000::cpu_mapwrite(uint16_t addr, uint32_t &addr_mapped) {
  if (addr >= 0x8000 && addr <= 0xFFFF) {
    return true;
  }
  return false;
}

bool Mapper_000::ppu_mapread(uint16_t addr, uint32_t &addr_mapped) {
  if (addr >= 0x0000 && addr <= 0x1FFF) {
    return true;
  }
  return false;
}

bool Mapper_000::ppu_mapwrite(uint16_t addr, uint32_t &addr_mapped) {
  return false;
}

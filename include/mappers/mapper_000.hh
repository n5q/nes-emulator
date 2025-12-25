#ifndef MAPPER_000_HH
#define MAPPER_000_HH

#include "mappers/mapper_template.hh"

class Mapper_000 : public Mapper_Template {
public:
  Mapper_000(uint8_t banks_PRG, uint8_t banks_CHR);
  ~Mapper_000();

  
  bool cpu_mapread(uint16_t addr, uint32_t &addr_mapped) override;
  bool cpu_mapwrite(uint16_t addr, uint32_t &addr_mapped) override;
  bool ppu_mapread(uint16_t addr, uint32_t &addr_mapped) override;
  bool ppu_mapwrite(uint16_t addr, uint32_t &addr_mapped) override;
};

#endif

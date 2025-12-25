#ifndef MAPPER_TEMPLATE_HH
#define MAPPER_TEMPLATE_HH

#include <cstdint>

class Mapper_Template {
public:
  Mapper_Template(uint8_t banks_PRG, uint8_t banks_CHR);
  ~Mapper_Template();

  virtual bool cpu_mapread(uint16_t addr, uint32_t &addr_mapped) = 0;
  virtual bool cpu_mapwrite(uint16_t addr, uint32_t &addr_mapped) = 0;
  virtual bool ppu_mapread(uint16_t addr, uint32_t &addr_mapped) = 0;
  virtual bool ppu_mapwrite(uint16_t addr, uint32_t &addr_mapped) = 0;


protected:
  uint8_t banks_PRG = 0;
  uint8_t banks_CHR = 0;
};



#endif

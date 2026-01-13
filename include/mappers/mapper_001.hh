#ifndef MAPPER_001_HH
#define MAPPER_001_HH

#include "mapper_template.hh"
#include <cstdint>
#include <functional>

class Mapper_001 : public Mapper_Template {
private:
  uint8_t shift_register;
  uint8_t shift_count;
  
  // control register (internal register 0)
  uint8_t control_register;
  
  // CHR bank registers (internal register 1 and 2)
  uint8_t chr_bank_0;
  uint8_t chr_bank_1;
  
  // PRG bank register (internal register 3)
  uint8_t prg_bank;
  
  //  from control register
  uint8_t mirroring;
  uint8_t prg_bank_mode;
  uint8_t chr_bank_mode;


  std::function<void(uint8_t)> mirror_callback;

public:
  std::vector<uint8_t> prg_ram;
  
  Mapper_001(uint8_t banks_PRG, uint8_t banks_CHR, std::function<void(uint8_t)> mirror_callback);
  ~Mapper_001();
  
  bool cpu_mapread(uint16_t addr, uint32_t &addr_mapped) override;
  bool cpu_mapwrite(uint16_t addr, uint32_t &addr_mapped, uint8_t data) override;
  bool ppu_mapread(uint16_t addr, uint32_t &addr_mapped) override;
  bool ppu_mapwrite(uint16_t addr, uint32_t &addr_mapped) override;
  
  void reset();
};

#endif

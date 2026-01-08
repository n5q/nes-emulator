#ifndef CARTRIDGE_HH
#define CARTRIDGE_HH

#include <cstdint>
#include <memory>
#include <string>
#include <fstream>
#include <vector>

#include "mappers/mapper_template.hh"
#include "mappers/all_mappers.hh"

class Cartridge {
public:
  Cartridge(const std::string &romfile);
  ~Cartridge();

  bool valid = false;
  uint8_t mapper_id;
  uint8_t banks_PRG;
  uint8_t banks_CHR;

  enum Mirror {
    HORIZONTAL,
    VERTICAL,
    ONESCREEN_LO,
    ONESCREEN_HI,
  } mirror = HORIZONTAL;
  
  std::shared_ptr<Mapper_Template> mapper; 
  
  // main bus
  bool cpu_read(uint16_t addr, uint8_t &data);
  bool cpu_write(uint16_t addr, uint8_t data);

  // ppu bus
  bool ppu_read(uint16_t addr, uint8_t &data);
  bool ppu_write(uint16_t addr, uint8_t data);

private:
   std::vector<uint8_t> mem_PRG;
   std::vector<uint8_t> mem_CHR;
};

#endif

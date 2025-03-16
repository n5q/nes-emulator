#ifndef BUS_HH
#define BUS_HH

#include <cstdint>
#include <array>

#include "mos6502.hh" 

class Bus {
public:
  Bus();
  ~Bus();

  // devices on bus
  MOS6502 cpu;
  std::array<uint8_t, 65536> mem; // 64kb ram

  void write(uint16_t addr, uint8_t data);
  uint8_t read(uint16_t addr);
};

#endif
#ifndef BUS
#define BUS
#endif

#include <cstdint>
#include <array>
#include "6502.hh"

class Bus {
public:
  Bus();
  ~Bus();

  // devices on bus
  6502 cpu;
  std::array<uint8_t, 65536> mem;

  void write(uint16_t addr, uint8_t data);
  uint8_t read(uint16_t addr);
};

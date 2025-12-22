#ifndef BUS_HH
#define BUS_HH

#include <cstdint>
#include <array>

#include "mos6502.hh"
#include "2C02.hh"

class Bus {
public:
  Bus();
  ~Bus();

  // devices on bus
  CPU cpu;
  std::array<uint8_t, 2*1024> cpu_mem; // 2kb ram
  void cpu_write(uint16_t addr, uint8_t data);
  uint8_t cpu_read(uint16_t addr);

  PPU ppu;
};

#endif

#ifndef BUS_HH
#define BUS_HH

#include <cstdint>
#include <array>
#include <memory>

#include "RP2A03.hh"
#include "cartridge.hh"
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
  uint8_t cpu_read(uint16_t addr, bool readonly);

  PPU ppu;
  std::shared_ptr<RP2A03> rp;
  std::shared_ptr<Cartridge> cart;
  std::shared_ptr<APU> apu;

  // interface
  void insert_cartridge(const std::shared_ptr<Cartridge>& cart);
  void reset();
  void clk();

  uint32_t sys_clocks = 0;

  static const int AUDIO_BUF_SIZE = 4096;
  float audio_buf[AUDIO_BUF_SIZE];
  int audio_write_pos = 0;
  int audio_read_pos = 0;

  void push_audio_sample(float sample);
  float pop_audio_sample();
  int get_audio_buf_size();
};

#endif

#ifndef RP2A03_HH
#define RP2A03_HH

#include <cstdint>
#include <memory>

class Bus;
class PPU;

class RP2A03 {
public:
  RP2A03();
  ~RP2A03();

  void connect_bus(std::shared_ptr<Bus> b);
  void connect_ppu(std::shared_ptr<PPU> p);

  // 2A03 interprets writes to $4000 - $4017
  void cpu_write(uint16_t addr, uint8_t data);
  uint8_t cpu_read(uint16_t addr);

  void clk();

  // DMA
  // DMA transfer suspends cpu for 513/514 cycles
  bool dma_transfer = false;
  bool dma_alignment = true;
  uint8_t dma_page = 0x00;
  uint8_t dma_addr = 0x00;
  uint8_t dma_data = 0x00;

  // controller states
  // current state of buttons (snapshot)
  uint8_t controller[2];
  // shift register for serial reading
  uint8_t controller_state[2];

private:
  std::shared_ptr<Bus> bus;
  std::shared_ptr<PPU> ppu;

  // TODO: APU
};

#endif

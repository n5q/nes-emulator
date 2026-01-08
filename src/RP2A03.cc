#include "RP2A03.hh"
#include "bus.hh"
#include "2C02.hh"

RP2A03::RP2A03() {

}

RP2A03::~RP2A03() {
  
}

void RP2A03::connect_bus(Bus* b) {
  this->bus = b;
}

void RP2A03::connect_ppu(std::shared_ptr<PPU> p) {
  this->ppu = p;
}

// 2A03 interprets writes to $4000 - $4017
void RP2A03::cpu_write(uint16_t addr, uint8_t data) {
  // TODO: APU registers go here

  // -- OAM DMA --
  // writing to this register initiates dma transfer
  if (addr == 0x4014) {
    this->dma_page = data;
    this->dma_addr = 0x00;
    this->dma_transfer = true;
    // first cycle is a dummy/alignment cycle
    this->dma_alignment = true;
    // for (int i = 0; i < 256; i++) {
    //   uint8_t val = bus->cpu_read((this->dma_page << 8) | i, false);
    //   ppu->oam_p[i] = val;
    // }
  }

  // controller strobe
  // if we write to $4016 we capture input
  if (addr == 0x4016) {
    // detect falling edge to latch controller state
    bool old_strobe = controller_strobe & 0x01;
    bool new_strobe = data & 0x01;
    
    // latch on falling edge 
    if (old_strobe && !new_strobe) {
      controller_state[0] = controller[0];
    }
    
    controller_strobe = data;
  }
}

uint8_t RP2A03::cpu_read(uint16_t addr, bool readonly) {
  uint8_t data = 0x00;

  // read controller 1 ($4016)
  if (addr == 0x4016) {
    // If readonly, dontt modify state (for debugging)
    if (readonly) {
      data = (controller_state[0] & 0x80) ? 0x01 : 0x00;
      return data;
    }
    
    // bit 0 = current MSB of controller state
    data = (controller_state[0] & 0x80) ? 0x01 : 0x00;
    
    // shift register for next button
    controller_state[0] <<= 1;
  }
  
  return data;
}


void RP2A03::clk() {
  // if DMA is happening, execute one step of transfer
  if (dma_transfer) {
    // wait for one cycle for synchronization
    if (dma_alignment) {
      if (bus->sys_clocks % 2 == 0) {
        dma_alignment = false;
      }
    }
    else {
      // dma can happen
      if (bus->sys_clocks % 2 == 1) {
        // read data from page specified
        dma_data = bus->cpu_read((uint16_t)(dma_page << 8) | dma_addr, false);
      }
      // write to PPU OAM (write cycle)
      else {
        // write directly to ppu oam memory
        ppu->oam_p[dma_addr] = dma_data;
        // oam addr auto increments on the ppu side when written via registers
        // but since array is written to directly, need to manually increment
        dma_addr++;
        // finished after wrapping around (0x00 -> ... -> 0xFF -> 0x00)
        if (dma_addr == 0x00) {
          dma_transfer = false;
          dma_alignment = true;
        }
      }
    }
  }

  else {
    // TODO : APU 
  }
  
}


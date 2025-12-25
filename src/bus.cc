#include "bus.hh"

Bus::Bus() {
  for (uint8_t &byte: this->cpu_mem) {
    byte = 0x00;
  }
  this->cpu.connect_bus(this);
} 

Bus::~Bus() {

}

void Bus::cpu_write(uint16_t addr, uint8_t data) {
  // cart gets highest priority for all r/w
  if (cart->cpu_write(addr, data)) {
    // dont do anything
  }

  else if (addr >= 0x0000 && addr <= 0x1FFF) {
    data = this->cpu_mem[addr & 0x07FF];
  }
  else if (addr >= 0x2000 && addr <= 0x3FFF) {
    ppu.cpu_write(addr & 0x0007, data);
  }
}

uint8_t Bus::cpu_read(uint16_t addr, bool readonly) {
  uint8_t data = 0x00;

  if (cart->cpu_read(addr, data)) {
    // dont do anything
  }
  
  // 8kb range of adresssed ram
  else if (addr >= 0x0000 && addr <= 0x1FFF) {
    data = this->cpu_mem[addr & 0x07FF];
  }
  else if (addr >= 0x2000 && addr <= 0x3FFF) {
    ppu.cpu_read(addr & 0x0007, readonly);
  }
  return data;
}

void Bus::insert_cartridge(const std::shared_ptr<Cartridge>& cartr) {
  this->cart = cartr;
  this->ppu.connect_cartridge(cartr);
}

void Bus::reset() {
  this->cpu.reset();
  this->sys_clocks = 0;
}
void Bus::clk() {
  // this->sys_clocks++;
}

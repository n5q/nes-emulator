#include "bus.hh"
#include "RP2A03.hh"
#include <memory>

Bus::Bus() {
  for (uint8_t &byte: this->cpu_mem) {
    byte = 0x00;
  }
  this->cpu.connect_bus(this);

  // RP2A03
  this->rp = std::make_shared<RP2A03>();
  this->rp->connect_bus(this);
  // i have no clue
  this->rp->connect_ppu(std::shared_ptr<PPU>(&this->ppu, [](PPU*){}));
} 

Bus::~Bus() {

}

void Bus::cpu_write(uint16_t addr, uint8_t data) {
  #ifdef DEBUG
  if (addr == 0x2001) {
  printf("PPUMASK write:  PC=%04X, A=%02X at SL=%d\n", cpu.pc - 1, cpu.a, ppu.scanline);
}
  // static bool first_frame = true;
  static int write_count = 0;
  
  if (addr == 0x2001 && write_count < 100) {
    printf("[%d] PPUMASK:   %02X at PC=%04X, SL=%d, Cycle=%d\n", 
           write_count++, data, cpu.pc, ppu.scanline, ppu.cycle);
  }
  #endif

  // cart gets highest priority for all r/w
  if (cart->cpu_write(addr, data)) {
    // dont do anything, cart handled it
  }

  else if (addr >= 0x0000 && addr <= 0x1FFF) {
    this->cpu_mem[addr & 0x07FF] = data;
  }
  else if (addr >= 0x2000 && addr <= 0x3FFF) {
    ppu.cpu_write(addr & 0x0007, data);
  }
  else if (addr >= 0x4000 && addr <= 0x4017) {
    rp->cpu_write(addr, data);
  }
}

uint8_t Bus::cpu_read(uint16_t addr, bool readonly) {
  uint8_t data = 0x00;

  // 1. cartridge address range
  if (cart->cpu_read(addr, data)) {
    // dont do anything, cart handled it
  }

  // 2. cpu internal ram
  // 2kb mirrored into 8kb range of adresssed ram
  else if (addr >= 0x0000 && addr <= 0x1FFF) {
    data = this->cpu_mem[addr & 0x07FF];
  }

  // 3. ppu registers (mirrored)
  else if (addr >= 0x2000 && addr <= 0x3FFF) {
    data = ppu.cpu_read(addr & 0x0007, readonly);
  }

  // 4. apu i/o registers
  else if (addr >= 0x4000 && addr <= 0x4017) {
    data = rp->cpu_read(addr, readonly);
  }

  #ifdef DEBUG
  if (cpu.pc >= 33105 && cpu.pc <= 33112 && !readonly) {
    printf("= 0x%02X\n", data);
  }
  #endif

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
  // ppu runs 3x faster than the cpu
  ppu.clk();

  if (ppu.nmi) {
    if (!rp->dma_transfer && cpu.inst_cycles == 0) {
      #ifdef DEBUG
      printf("NMI serviced at PC=0x%04X, SL=%d\n", cpu.pc, ppu.scanline);
      #endif
      ppu.nmi = false;
      cpu.nmi();
    }
    #ifdef DEBUG
    else {
      static int nmi_wait_count = 0;
      if (nmi_wait_count++ % 1000 == 0) {
        printf("NMI waiting: DMA=%d, inst_cycles=%d\n", 
             rp->dma_transfer, cpu.inst_cycles);
      }
    }
    #endif
  }

  // every 3 ppu cycles
  if (!(sys_clocks % 3)) {
    rp->clk();
    // check if dma hijacking bus
    if (rp->dma_transfer) {
      // suspend cpu if dma
    }
    else {
      // normal
      cpu.clk();
    }
  }
  sys_clocks++;
}

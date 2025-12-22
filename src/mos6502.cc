#include "mos6502.hh"
#include "bus.hh"

CPU::CPU() {
  #define INST(name, opcode, addr, inst_cycles) { name, &CPU::opcode, &CPU::addr, inst_cycles }
  lookup = {
    // 0x00
    INST("BRK", BRK, IMM, 7),  INST("ORA", ORA, IDX, 6),  INST("???", XXX, IMP, 2),  INST("???", XXX, IMP, 8),
    INST("???", NOP, IMP, 3),  INST("ORA", ORA, ZPG, 3),  INST("ASL", ASL, ZPG, 5),  INST("???", XXX, IMP, 5),
    INST("PHP", PHP, IMP, 3),  INST("ORA", ORA, IMM, 2),  INST("ASL", ASL, IMP, 2),  INST("???", XXX, IMP, 2),
    INST("???", NOP, IMP, 4),  INST("ORA", ORA, ABS, 4),  INST("ASL", ASL, ABS, 6),  INST("???", XXX, IMP, 6),

    // 0x10
    INST("BPL", BPL, REL, 2),  INST("ORA", ORA, IDY, 5),  INST("???", XXX, IMP, 2),  INST("???", XXX, IMP, 8),
    INST("???", NOP, IMP, 4),  INST("ORA", ORA, ZPX, 4),  INST("ASL", ASL, ZPX, 6),  INST("???", XXX, IMP, 6),
    INST("CLC", CLC, IMP, 2),  INST("ORA", ORA, ABY, 4),  INST("???", NOP, IMP, 2),  INST("???", XXX, IMP, 7),
    INST("???", NOP, IMP, 4),  INST("ORA", ORA, ABX, 4),  INST("ASL", ASL, ABX, 7),  INST("???", XXX, IMP, 7),

    // 0x20
    INST("JSR", JSR, ABS, 6),  INST("AND", AND, IDX, 6),  INST("???", XXX, IMP, 2),  INST("???", XXX, IMP, 8),
    INST("BIT", BIT, ZPG, 3),  INST("AND", AND, ZPG, 3),  INST("ROL", ROL, ZPG, 5),  INST("???", XXX, IMP, 5),
    INST("PLP", PLP, IMP, 4),  INST("AND", AND, IMM, 2),  INST("ROL", ROL, IMP, 2),  INST("???", XXX, IMP, 2),
    INST("BIT", BIT, ABS, 4),  INST("AND", AND, ABS, 4),  INST("ROL", ROL, ABS, 6),  INST("???", XXX, IMP, 6),

    // 0x30
    INST("BMI", BMI, REL, 2),  INST("AND", AND, IDY, 5),  INST("???", XXX, IMP, 2),  INST("???", XXX, IMP, 8),
    INST("???", NOP, IMP, 4),  INST("AND", AND, ZPX, 4),  INST("ROL", ROL, ZPX, 6),  INST("???", XXX, IMP, 6),
    INST("SEC", SEC, IMP, 2),  INST("AND", AND, ABY, 4),  INST("???", NOP, IMP, 2),  INST("???", XXX, IMP, 7),
    INST("???", NOP, IMP, 4),  INST("AND", AND, ABX, 4),  INST("ROL", ROL, ABX, 7),  INST("???", XXX, IMP, 7),

    // 0x40
    INST("RTI", RTI, IMP, 6),  INST("EOR", EOR, IDX, 6),  INST("???", XXX, IMP, 2),  INST("???", XXX, IMP, 8),
    INST("???", NOP, IMP, 3),  INST("EOR", EOR, ZPG, 3),  INST("LSR", LSR, ZPG, 5),  INST("???", XXX, IMP, 5),
    INST("PHA", PHA, IMP, 3),  INST("EOR", EOR, IMM, 2),  INST("LSR", LSR, IMP, 2),  INST("???", XXX, IMP, 2),
    INST("JMP", JMP, ABS, 3),  INST("EOR", EOR, ABS, 4),  INST("LSR", LSR, ABS, 6),  INST("???", XXX, IMP, 6),

    // 0x50
    INST("BVC", BVC, REL, 2),  INST("EOR", EOR, IDY, 5),  INST("???", XXX, IMP, 2),  INST("???", XXX, IMP, 8),
    INST("???", NOP, IMP, 4),  INST("EOR", EOR, ZPX, 4),  INST("LSR", LSR, ZPX, 6),  INST("???", XXX, IMP, 6),
    INST("CLI", CLI, IMP, 2),  INST("EOR", EOR, ABY, 4),  INST("???", NOP, IMP, 2),  INST("???", XXX, IMP, 7),
    INST("???", NOP, IMP, 4),  INST("EOR", EOR, ABX, 4),  INST("LSR", LSR, ABX, 7),  INST("???", XXX, IMP, 7),

    // 0x60
    INST("RTS", RTS, IMP, 6),  INST("ADC", ADC, IDX, 6),  INST("???", XXX, IMP, 2),  INST("???", XXX, IMP, 8),
    INST("???", NOP, IMP, 3),  INST("ADC", ADC, ZPG, 3),  INST("ROR", ROR, ZPG, 5),  INST("???", XXX, IMP, 5),
    INST("PLA", PLA, IMP, 4),  INST("ADC", ADC, IMM, 2),  INST("ROR", ROR, IMP, 2),  INST("???", XXX, IMP, 2),
    INST("JMP", JMP, IND, 5),  INST("ADC", ADC, ABS, 4),  INST("ROR", ROR, ABS, 6),  INST("???", XXX, IMP, 6),

    // 0x70
    INST("BVS", BVS, REL, 2),  INST("ADC", ADC, IDY, 5),  INST("???", XXX, IMP, 2),  INST("???", XXX, IMP, 8),
    INST("???", NOP, IMP, 4),  INST("ADC", ADC, ZPX, 4),  INST("ROR", ROR, ZPX, 6),  INST("???", XXX, IMP, 6),
    INST("SEI", SEI, IMP, 2),  INST("ADC", ADC, ABY, 4),  INST("???", NOP, IMP, 2),  INST("???", XXX, IMP, 7),
    INST("???", NOP, IMP, 4),  INST("ADC", ADC, ABX, 4),  INST("ROR", ROR, ABX, 7),  INST("???", XXX, IMP, 7),

    // 0x80
    INST("???", NOP, IMP, 2),  INST("STA", STA, IDX, 6),  INST("???", NOP, IMP, 2),  INST("???", XXX, IMP, 6),
    INST("STY", STY, ZPG, 3),  INST("STA", STA, ZPG, 3),  INST("STX", STX, ZPG, 3),  INST("???", XXX, IMP, 3),
    INST("DEY", DEY, IMP, 2),  INST("???", NOP, IMP, 2),  INST("TXA", TXA, IMP, 2),  INST("???", XXX, IMP, 2),
    INST("STY", STY, ABS, 4),  INST("STA", STA, ABS, 4),  INST("STX", STX, ABS, 4),  INST("???", XXX, IMP, 4),

    // 0x90
    INST("BCC", BCC, REL, 2),  INST("STA", STA, IDY, 6),  INST("???", XXX, IMP, 2),  INST("???", XXX, IMP, 6),
    INST("STY", STY, ZPX, 4),  INST("STA", STA, ZPX, 4),  INST("STX", STX, ZPY, 4),  INST("???", XXX, IMP, 4),
    INST("TYA", TYA, IMP, 2),  INST("STA", STA, ABY, 5),  INST("TXS", TXS, IMP, 2),  INST("???", XXX, IMP, 5),
    INST("???", NOP, IMP, 5),  INST("STA", STA, ABX, 5),  INST("???", XXX, IMP, 5),  INST("???", XXX, IMP, 5),

    // 0xA0
    INST("LDY", LDY, IMM, 2),  INST("LDA", LDA, IDX, 6),  INST("LDX", LDX, IMM, 2),  INST("???", XXX, IMP, 6),
    INST("LDY", LDY, ZPG, 3),  INST("LDA", LDA, ZPG, 3),  INST("LDX", LDX, ZPG, 3),  INST("???", XXX, IMP, 3),
    INST("TAY", TAY, IMP, 2),  INST("LDA", LDA, IMM, 2),  INST("TAX", TAX, IMP, 2),  INST("???", XXX, IMP, 2),
    INST("LDY", LDY, ABS, 4),  INST("LDA", LDA, ABS, 4),  INST("LDX", LDX, ABS, 4),  INST("???", XXX, IMP, 4),

    // 0xB0
    INST("BCS", BCS, REL, 2),  INST("LDA", LDA, IDY, 5),  INST("???", XXX, IMP, 2),  INST("???", XXX, IMP, 5),
    INST("LDY", LDY, ZPX, 4),  INST("LDA", LDA, ZPX, 4),  INST("LDX", LDX, ZPY, 4),  INST("???", XXX, IMP, 4),
    INST("CLV", CLV, IMP, 2),  INST("LDA", LDA, ABY, 4),  INST("TSX", TSX, IMP, 2),  INST("???", XXX, IMP, 4),
    INST("LDY", LDY, ABX, 4),  INST("LDA", LDA, ABX, 4),  INST("LDX", LDX, ABY, 4),  INST("???", XXX, IMP, 4),

    // 0xC0
    INST("CPY", CPY, IMM, 2),  INST("CMP", CMP, IDX, 6),  INST("???", NOP, IMP, 2),  INST("???", XXX, IMP, 8),
    INST("CPY", CPY, ZPG, 3),  INST("CMP", CMP, ZPG, 3),  INST("DEC", DEC, ZPG, 5),  INST("???", XXX, IMP, 5),
    INST("INY", INY, IMP, 2),  INST("CMP", CMP, IMM, 2),  INST("DEX", DEX, IMP, 2),  INST("???", XXX, IMP, 2),
    INST("CPY", CPY, ABS, 4),  INST("CMP", CMP, ABS, 4),  INST("DEC", DEC, ABS, 6),  INST("???", XXX, IMP, 6),

    // 0xD0
    INST("BNE", BNE, REL, 2),  INST("CMP", CMP, IDY, 5),  INST("???", XXX, IMP, 2),  INST("???", XXX, IMP, 8),
    INST("???", NOP, IMP, 4),  INST("CMP", CMP, ZPX, 4),  INST("DEC", DEC, ZPX, 6),  INST("???", XXX, IMP, 6),
    INST("CLD", CLD, IMP, 2),  INST("CMP", CMP, ABY, 4),  INST("NOP", NOP, IMP, 2),  INST("???", XXX, IMP, 7),
    INST("???", NOP, IMP, 4),  INST("CMP", CMP, ABX, 4),  INST("DEC", DEC, ABX, 7),  INST("???", XXX, IMP, 7),

    // 0xE0
    INST("CPX", CPX, IMM, 2),  INST("SBC", SBC, IDX, 6),  INST("???", NOP, IMP, 2),  INST("???", XXX, IMP, 8),
    INST("CPX", CPX, ZPG, 3),  INST("SBC", SBC, ZPG, 3),  INST("INC", INC, ZPG, 5),  INST("???", XXX, IMP, 5),
    INST("INX", INX, IMP, 2),  INST("SBC", SBC, IMM, 2),  INST("NOP", NOP, IMP, 2),  INST("???", SBC, IMP, 2),
    INST("CPX", CPX, ABS, 4),  INST("SBC", SBC, ABS, 4),  INST("INC", INC, ABS, 6),  INST("???", XXX, IMP, 6),

    // 0xF0
    INST("BEQ", BEQ, REL, 2),  INST("SBC", SBC, IDY, 5),  INST("???", XXX, IMP, 2),  INST("???", XXX, IMP, 8),
    INST("???", NOP, IMP, 4),  INST("SBC", SBC, ZPX, 4),  INST("INC", INC, ZPX, 6),  INST("???", XXX, IMP, 6),
    INST("SED", SED, IMP, 2),  INST("SBC", SBC, ABY, 4),  INST("NOP", NOP, IMP, 2),  INST("???", XXX, IMP, 7),
    INST("???", NOP, IMP, 4),  INST("SBC", SBC, ABX, 4),  INST("INC", INC, ABX, 7),  INST("???", XXX, IMP, 7)
  };
}

CPU::~CPU() {}

void CPU::connect_bus(Bus *b) {
  this->bus = b;
}

void CPU::write(uint16_t addr, uint8_t data) {
  if (addr >= 0x0000 && addr <= 0xFFFF) {
    return bus->cpu_write(addr, data);
  }
}

uint8_t CPU::read(uint16_t addr) {
  if (addr >= 0x0000 && addr <= 0xFFFF) {
    return bus->cpu_read(addr);
  }
  return 0x00;
}

bool CPU::get_flag(FLAG f) {
  return (psr & f) != 0;
}

// void MOS6502::set_flag(FLAG f) {
//     psr = (psr | f);
// }

void CPU::set_flag(FLAG f) {
  psr = (psr | f);
}

void CPU::clear_flag(FLAG f) {
  psr = (psr & ~f);
}

void CPU::sflag(FLAG f, bool b) {
  if (b) {
    set_flag(f);
  } else {
    clear_flag(f);
  }
}

void CPU::fetch() {
  m = read(addr);
}

void CPU::clk() {
  if (inst_cycles == 0) {
    opcode = read(pc++);
    CPU::Instruction inst = lookup[opcode];
    inst_cycles = inst.inst_cycles;

    uint8_t a = (this->*inst.addr_mode)();
    uint8_t b = (this->*inst.opcode)();

    // additional inst_cycles if needed
    inst_cycles += (a & b);
  }
  cycles++;
  inst_cycles--;
}

void CPU::reset() {
  a = 0;
  x = 0;
  y = 0;
  sp = 0xFD;
  psr = 0x00 | UNUSED;

  // reset vector - start executing code from here
  uint16_t reset_vector = 0xFFFC;
  uint16_t low = read(reset_vector);
  uint16_t high = read(reset_vector+1);
  pc = (high << 8) | low;

  addr = 0x0000;
  addr_branch = 0x0000;
  m = 0x0000;
}

void CPU::irq() {
  if (!get_flag(INTERRUPT)) {
    // write current pc to stack
    write(0x0100+sp, (pc >> 8) & 0x00FF);
    sp--;
    write(0x0100+sp, pc & 0x00FF);
    sp--;

    // bits set on irq and psr pushed to stack
    sflag(BREAK, 0);
    sflag(UNUSED, 1);
    sflag(INTERRUPT, 1);
    write(0x0100+sp, psr);
    sp--;

    uint16_t irq_vector = 0xFFFE;
    uint16_t low = read(irq_vector);
    uint16_t high = read(irq_vector+1);
    pc = (high << 8) | low;    

    inst_cycles = 7;
  }
}

// same as irq but not conditional on I flag and different address to jump to
void CPU::nmi() {
  // write current pc to stack
  write(0x0100+sp, (pc >> 8) & 0x00FF);
  sp--;
  write(0x0100+sp, pc & 0x00FF);
  sp--;

  // bits set on nmi and psr pushed to stack
  sflag(BREAK, 0);
  sflag(UNUSED, 1);
  sflag(INTERRUPT, 1);
  write(0x0100+sp, psr);
  sp--;

  uint16_t nmi_vector = 0xFFFA;
  uint16_t low = read(nmi_vector);
  uint16_t high = read(nmi_vector+1);
  pc = (high << 8) | low;    

  inst_cycles = 7;
}

// ADDRESSING MODES

// absolute
uint8_t CPU::ABS() {
  uint16_t low = read(pc++);
  uint16_t high = read(pc++) << 8;
  addr = high | low;
  return 0;
}

// absolute w/ x offset
uint8_t CPU::ABX() {
  uint16_t low = read(pc++);
  uint16_t high = read(pc++) << 8;
  addr = (high | low) + x;
  // check if we have crossed into a new page (high byte) and need a extra clock cycle
  return ((addr & 0xFF00) != high) ? 1 : 0;
}

// absolute w/ y offset
uint8_t CPU::ABY() {
  uint16_t low = read(pc++);
  uint16_t high = read(pc++) << 8;
  addr = (high | low) + y;
  // check if we have crossed into a new page (high byte) and need a extra clock cycle
  return ((addr & 0xFF00) != high) ? 1 : 0;
}

// immediate
uint8_t CPU::IMM() { 
  addr = pc++;
  return 0;
}

// implied (accumulator)
uint8_t CPU::IMP() {
  m = a;
  return 0;
}

// indirect (like pointer, only used by JMP instruction)
uint8_t CPU::IND() {
  uint16_t low = read(pc++);
  uint16_t high = read(pc++) << 8;
  uint16_t ptr = high | low;

  // emulate the bug where the original 6502 has does not
  // correctly fetch the target address if the
  // indirect vector falls on a page boundary
  // https://www.nesdev.org/obelisk-6502-guide/reference.html#JMP
  if (low == 0xFF) {
      addr = (read(ptr & 0xFF00) << 8) | read(ptr);
      return 0;
  }

  addr = (read(ptr + 1) << 8) | read(ptr);
  return 0;
} 

// indexed indirect, zero page offset by x
uint8_t CPU::IDX() {
  uint8_t addr_zp = read(pc++);
  uint8_t ptr = (addr_zp + x) & 0xFF; // wrap around zp

  uint16_t low = read(ptr);
  uint16_t high = read((ptr + 1) & 0xFF) << 8;

  addr = high | low;
  return 0;
} 

// indirect indexed, zero page offset by y
uint8_t CPU::IDY() {
  uint8_t addr_zp = read(pc++);

  uint16_t low = read(addr_zp);
  uint16_t high = read((addr_zp + 1) & 0xFF) << 8;

  uint16_t addr_base = high | low;
  addr = addr_base + y;
  // check if we have crossed into a new page (high byte) and need a extra clock cycle
  return ((addr & 0xFF00) != (addr_base & 0xFF00)) ? 1 : 0;
} 

// relative, we modify addr_branch instead of addr
// since we dont know if we want to commit to 
// the branch yet until the branch condition is checked
uint8_t CPU::REL() {
  int8_t offset = (int8_t) read(pc++);
  addr_branch = pc + offset;
  return 0;
}

// zero page
uint8_t CPU::ZPG() {
  addr = read(pc++) & 0x00FF;
  return 0;
}

// zero page, x offset
uint8_t CPU::ZPX() {
  addr = (read(pc++) + x) & 0x00FF;
  return 0;
}

// zero page, y offset
uint8_t CPU::ZPY() {
  addr = (read(pc++) + y) & 0x00FF;
  return 0;
}


// INSTRUCTIONS
// http://www.6502.org/users/obelisk/6502/reference.html

// ADC - Add with Carry
// A,Z,C,N = A+M+C
// This instruction adds the contents of a memory location to the accumulator together with the carry bit. If overflow occurs the carry bit is set, this enables multiple byte addition to be performed.

uint8_t CPU::ADC() {
  fetch();
  uint16_t add = (uint16_t) a + (uint16_t) m + (uint16_t) get_flag(CARRY);
  sflag(CARRY, add > 0xFF);
  sflag(ZERO, (add & 0x00FF) == 0);
  sflag(NEGATIVE, add & 128);
  // V = ~(a+data) * (a+result)
  sflag(OVERFLOW, (~((uint16_t)a ^ (uint16_t)m) & ((uint16_t)a ^ (uint16_t)add)) & 128);

  a = add & 0x00FF;
  return 1;
}

// AND - Logical AND
// A,Z,N = A&M
// A logical AND is performed, bit by bit, on the accumulator contents using the contents of a byte of memory
uint8_t CPU::AND() {
  fetch();
  a = a & m;

  sflag(ZERO, !a);
  sflag(NEGATIVE, a & (1<<7));

  return 1;
}

// ASL - Arithmetic Shift Left
// A,Z,C,N = M*2 or M,Z,C,N = M*2
// This operation shifts all the bits of the accumulator or memory contents one bit left. Bit 0 is set to 0 and bit 7 is placed in the carry flag. The effect of this operation is to multiply the memory contents by 2 (ignoring 2's complement considerations), setting the carry if the result will not fit in 8 bits.
uint8_t CPU::ASL() {
  fetch();
  sflag(CARRY, m & (1 << 7));

  uint8_t tmp = m << 1;
  if (lookup[opcode].addr_mode == &CPU::IMP) {
      a = tmp;
  }
  else {
      write(addr, tmp);
  }

  sflag(ZERO, !tmp);
  sflag(NEGATIVE, tmp & (1 << 7));
  

  return 0;
}

// BCC - Branch if Carry Clear
// If the carry flag is clear then add the relative displacement to the program counter to cause a branch to a new location.
uint8_t CPU::BCC() {
  if (!get_flag(CARRY)) {
    inst_cycles++;

    // if we cross into new page then take another cycle
    if ((addr_branch & 0xFF00) != (pc & 0xFF00)) {
      inst_cycles++;
    }

    pc = addr_branch;
  }

  return 0;
}

// BCS - Branch if Carry Set
// If the carry flag is set then add the relative displacement to the program counter to cause a branch to a new location.
uint8_t CPU::BCS() {
  if (get_flag(CARRY)) {
    inst_cycles++;

    // if we cross into new page then take another cycle
    if ((addr_branch & 0xFF00) != (pc & 0xFF00)) {
      inst_cycles++;
    }

    pc = addr_branch;
  }

  return 0;
}

// BEQ - Branch if Equal
// If the zero flag is set then add the relative displacement to the program counter to cause a branch to a new location.
uint8_t CPU::BEQ() {
  if (get_flag(ZERO)) {
      inst_cycles++;

    // if we cross into new page then take another cycle
    if ((addr_branch & 0xFF00) != (pc & 0xFF00)) {
      inst_cycles++;
    }

    pc = addr_branch;
    }

    return 0;
}

// BIT - Bit Test
// A & M, N = M7, V = M6
// This instructions is used to test if one or more bits are set in a target memory location. The mask pattern in A is ANDed with the value in memory to set or clear the zero flag, but the result is not kept. Bits 7 and 6 of the value from memory are copied into the N and V flags.
uint8_t CPU::BIT() {
  fetch();
  uint8_t bit = a & m;
  sflag(ZERO, !(bit & 0x00FF));
  sflag(NEGATIVE, m & (1<<7));
  sflag(OVERFLOW, m & (1<<6));

  return 0;
}

// BMI - Branch if Minus
// If the negative flag is set then add the relative displacement to the program counter to cause a branch to a new location.
uint8_t CPU::BMI() {
  if (get_flag(NEGATIVE)) {
    inst_cycles++;

      // if we cross into new page then take another cycle
    if ((addr_branch & 0xFF00) != (pc & 0xFF00)) {
      inst_cycles++;
    }

    pc = addr_branch;
  }

  return 0;
}

// BNE - Branch if Not Equal
// If the zero flag is clear then add the relative displacement to the program counter to cause a branch to a new location.
uint8_t CPU::BNE() {
  if (!get_flag(ZERO)) {
    inst_cycles++;

    // if we cross into new page then take another cycle
    if ((addr_branch & 0xFF00) != (pc & 0xFF00)) {
      inst_cycles++;
    }

    pc = addr_branch;
  }

  return 0;
}

// BPL - Branch if Positive
// If the negative flag is clear then add the relative displacement to the program counter to cause a branch to a new location.
uint8_t CPU::BPL() {
  if (!get_flag(NEGATIVE)) {
    inst_cycles++;

    // if we cross into new page then take another cycle
    if ((addr_branch & 0xFF00) != (pc & 0xFF00)) {
      inst_cycles++;
    }

    pc = addr_branch;
  }

  return 0;
}

// BRK - Force Interrupt
// The BRK instruction forces the generation of an interrupt request. The program counter and processor status are pushed on the stack then the IRQ interrupt vector at $FFFE/F is loaded into the PC and the break flag in the status set to one.
uint8_t CPU::BRK() {
  pc++;
  
  // write current pc to stack
  write(0x0100+sp, (pc >> 8) & 0x00FF);
  sp--;
  write(0x0100+sp, pc & 0x00FF);
  sp--;

  // bits set on irq and psr pushed to stack
  write(0x0100+sp, psr | BREAK | UNUSED);
  sp--;
  sflag(BREAK, 0);
  sflag(INTERRUPT, 1);

  uint16_t irq_vector = 0xFFFE;
  uint16_t low = read(irq_vector);
  uint16_t high = read(irq_vector+1);
  pc = (high << 8) | low;    

  return 0;
}

// BVC - Branch if Overflow Clear
// If the overflow flag is clear then add the relative displacement to the program counter to cause a branch to a new location.
uint8_t CPU::BVC() {
  if (!get_flag(OVERFLOW)) {
    inst_cycles++;

    // if we cross into new page then take another cycle
    if ((addr_branch & 0xFF00) != (pc & 0xFF00)) {
      inst_cycles++;
    }

    pc = addr_branch;
  }

  return 0;
}

// BVS - Branch if Overflow Set
// If the overflow flag is set then add the relative displacement to the program counter to cause a branch to a new location.
uint8_t CPU::BVS() {
  if (get_flag(OVERFLOW)) {
    inst_cycles++;

    // if we cross into new page then take another cycle
    if ((addr_branch & 0xFF00) != (pc & 0xFF00)) {
      inst_cycles++;
    }

    pc = addr_branch;
  }

  return 0;
}

// CLC - Clear Carry Flag
// C = 0
// Set the carry flag to zero.
uint8_t CPU::CLC() {
  sflag(CARRY, 0);
  return 0;
}

// CLD - Clear Decimal Mode
// D = 0
// Sets the decimal mode flag to zero.
uint8_t CPU::CLD() {
  sflag(DECIMAL, 0);
  return 0;
}

// CLI - Clear Interrupt Disable
// I = 0
// Clears the interrupt disable flag allowing normal interrupt requests to be serviced.
uint8_t CPU::CLI() {
  sflag(INTERRUPT, 0);
  return 0;
}

// CLV - Clear Overflow Flag
// V = 0
// Clears the overflow flag.
uint8_t CPU::CLV() {
  sflag(OVERFLOW, 0);
  return 0;
}

// CMP - Compare
// Z,C,N = A-M
// This instruction compares the contents of the accumulator with another memory held value and sets the zero and carry flags as appropriate.
uint8_t CPU::CMP() {
  fetch();
  sflag(ZERO, a == m);
  sflag(NEGATIVE, (a - m) & 128);
  sflag(CARRY, a >= m);
  return 1;
}

// CPX - Compare X Register
// Z,C,N = X-M
// This instruction compares the contents of the X register with another memory held value and sets the zero and carry flags as appropriate.
uint8_t CPU::CPX() {
  fetch();
  sflag(ZERO, x == m);
  sflag(NEGATIVE, (x - m) & 128);
  sflag(CARRY, x >= m);
  return 1;
}

// CPY - Compare Y Register
// Z,C,N = Y-M
// This instruction compares the contents of the Y register with another memory held value and sets the zero and carry flags as appropriate.
uint8_t CPU::CPY() {
  fetch();
  sflag(ZERO, y == m);
  sflag(NEGATIVE, (y - m) & 128);
  sflag(CARRY, y >= m);
  return 1;
}

// DEC - Decrement Memory
// M,Z,N = M-1
// Subtracts one from the value held at a specified memory location setting the zero and negative flags as appropriate.
uint8_t CPU::DEC() {
  fetch();
  write(addr, ((uint16_t) m-1) & 0x00FF);
  sflag(ZERO, !(((uint16_t) m-1) & 0x00FF));
  sflag(NEGATIVE, ((uint16_t) m-1) & 128);
  return 0;
}

// DEX - Decrement X Register
// X,Z,N = X-1
// Subtracts one from the X register setting the zero and negative flags as appropriate.
uint8_t CPU::DEX() {
  x--;
  sflag(ZERO, !x);
  sflag(NEGATIVE, x & 128);
  return 0;
}

// DEY - Decrement Y Register
// Y,Z,N = Y-1
// Subtracts one from the Y register setting the zero and negative flags as appropriate.
uint8_t CPU::DEY() {
  y--;
  sflag(ZERO, !y);
  sflag(NEGATIVE, y & 128);
  return 0;
}

// EOR - Exclusive OR
// A,Z,N = A^M
// An exclusive OR is performed, bit by bit, on the accumulator contents using the contents of a byte of memory.
uint8_t CPU::EOR() {
  fetch();
  a ^= m;
  sflag(ZERO, !a);
  sflag(NEGATIVE, a & 128);
  return 1;
}

// INC - Increment Memory
// M,Z,N = M+1
// Adds one to the value held at a specified memory location setting the zero and negative flags as appropriate.
uint8_t CPU::INC() {
  fetch();
  write(addr, ((uint16_t) m+1) & 0x00FF);
  sflag(ZERO, !(((uint16_t) m+1) & 0x00FF));
  sflag(NEGATIVE, ((uint16_t) m+1) & 128);
  return 0;
}

// INX - Increment X Register
// X,Z,N = X+1
// Adds one to the X register setting the zero and negative flags as appropriate.
uint8_t CPU::INX() {
  x++;
  sflag(ZERO, !x);
  sflag(NEGATIVE, x & 128);
  return 0;
}

// INY - Increment Y Register
// Y,Z,N = Y+1
// Adds one to the Y register setting the zero and negative flags as appropriate.
uint8_t CPU::INY() {
  y++;
  sflag(ZERO, !y);
  sflag(NEGATIVE, y & 128);
  return 0;
}

// JMP - Jump
// Sets the program counter to the address specified by the operand.
uint8_t CPU::JMP() {
  pc = addr;
  return 0;
}

// JSR - Jump to Subroutine
// The JSR instruction pushes the address (minus one) of the return point on to the stack and then sets the program counter to the target memory address.
uint8_t CPU::JSR() {
  pc--;
  write(0x0100 + sp, (pc >> 8) & 0x00FF);
  sp--;
  write(0x0100 + sp, pc & 0x00FF);
  sp--;

  pc = addr;
  return 0;
}

// LDA - Load Accumulator
// A,Z,N = M
// Loads a byte of memory into the accumulator setting the zero and negative flags as appropriate.
uint8_t CPU::LDA() {
  fetch();
  a = m;
  sflag(ZERO, !a);
  sflag(NEGATIVE, a & 128);
  return 1;
}

// LDX - Load X Register
// X,Z,N = M
// Loads a byte of memory into the X register setting the zero and negative flags as appropriate.
uint8_t CPU::LDX() {
  fetch();
  x = m;
  sflag(ZERO, !x);
  sflag(NEGATIVE, x & 128);
  return 1;
}

// LDY - Load Y Register
// Y,Z,N = M
// Loads a byte of memory into the Y register setting the zero and negative flags as appropriate.
uint8_t CPU::LDY() {
  fetch();
  y = m;
  sflag(ZERO, !y);
  sflag(NEGATIVE, y & 128);
  return 1;
}

// LSR - Logical Shift Right
// A,C,Z,N = A/2 or M,C,Z,N = M/2
// Each of the bits in A or M is shift one place to the right. The bit that was in bit 0 is shifted into the carry flag. Bit 7 is set to zero.
uint8_t CPU::LSR() {
  fetch();
  sflag(CARRY, m & 0x0001);
  
  if (lookup[opcode].addr_mode == &CPU::IMP) {
    a = m >> 1; 
  }
  else {
    write(addr, m >> 1);
  }

  sflag(ZERO, !(m >> 1));
  sflag(NEGATIVE, (m >> 1) & 128);
  return 0;
}

// NOP - No Operation
// The NOP instruction causes no changes to the processor other than the normal incrementing of the program counter to the next instruction.
// ? illegal opcodes ? 
uint8_t CPU::NOP() {
  return 0;
}

// ORA - Logical Inclusive OR
// A,Z,N = A|M
// An inclusive OR is performed, bit by bit, on the accumulator contents using the contents of a byte of memory.
uint8_t CPU::ORA() {
  fetch();
  a |= m;
  sflag(ZERO, !a);
  sflag(NEGATIVE, a & 128);
  return 1;
}

// PHA - Push Accumulator
// Pushes a copy of the accumulator on to the stack.
uint8_t CPU::PHA() {
  write(0x0100 + sp, a);
  sp--;
  return 0;
}

// PHP - Push Processor Status
// Pushes a copy of the status flags on to the stack.
uint8_t CPU::PHP() {
  write(0x0100 + sp, psr | BREAK | UNUSED);
  sp--;
  return 0;
}

// PLA - Pull Accumulator
// Pulls an 8 bit value from the stack and into the accumulator. The zero and negative flags are set as appropriate.
uint8_t CPU::PLA() {
  sp++;
  a = read(0x0100 + sp);
  sflag(ZERO, !a);
  sflag(NEGATIVE, a & 128);
  return 0;    
}

// PLP - Pull Processor Status
// Pulls an 8 bit value from the stack and into the processor flags. The flags will take on new states as determined by the value pulled.
uint8_t CPU::PLP() {
  sp++;
  psr = read(0x0100 + sp);
  psr &= ~BREAK;
  psr |= UNUSED;
  return 0;    
}

// ROL - Rotate Left
// Move each of the bits in either A or M one place to the left. Bit 0 is filled with the current value of the carry flag whilst the old bit 7 becomes the new carry flag value.
uint8_t CPU::ROL() {
  fetch();
  uint8_t old_c = get_flag(CARRY);
  sflag(CARRY, m & 128);

  uint8_t result = (m << 1) | old_c;
  
  if (lookup[opcode].addr_mode == &CPU::IMP) {
    a = result; 
  }
  else {
    write(addr, result);
  }

  sflag(ZERO, !result);
  sflag(NEGATIVE, result & 128);
  return 0;
}

// ROR - Rotate Right
// Move each of the bits in either A or M one place to the right. Bit 7 is filled with the current value of the carry flag whilst the old bit 0 becomes the new carry flag value.
uint8_t CPU::ROR() {
  fetch();
  uint8_t result = (m >> 1) | (get_flag(CARRY) << 7);
  sflag(CARRY, m & 0x0001);
  
  if (lookup[opcode].addr_mode == &CPU::IMP) {
      a = result; 
  }
  else {
      write(addr, result);
  }

  sflag(ZERO, !result);
  sflag(NEGATIVE, result & 128);
  return 0;
}

// RTI - Return from Interrupt
// The RTI instruction is used at the end of an interrupt processing routine. It pulls the processor flags from the stack followed by the program counter.
uint8_t CPU::RTI() {
  // read the psr and pc which we pushed to stack in irq() or nmi()
  sp++;
  psr = read(0x0100 + sp);
  psr &= ~BREAK;
  psr |= UNUSED;

  sp++;
  pc = (uint16_t) read(0x0100 + sp);
  sp++;
  pc |= (uint16_t) (read(0x0100 + sp) << 8);
  return 0;
}

// RTS - Return from Subroutine
// The RTS instruction is used at the end of a subroutine to return to the calling routine. It pulls the program counter (minus one) from the stack.
uint8_t CPU::RTS() {
  sp++;
  pc = (uint16_t) read(0x0100 + sp);
  sp++;
  pc |= (uint16_t) (read(0x0100 + sp) << 8);
  pc++;
  return 0;
}


// SBC - Subtract with Carry
// A,Z,C,N = A-M-(1-C)
// This instruction subtracts the contents of a memory location to the accumulator together with the not of the carry bit. If overflow occurs the carry bit is clear, this enables multiple byte subtraction to be performed.
uint8_t CPU::SBC() {
  fetch();

  // invert bottom 8 bits
  uint16_t inverted = ((uint16_t) m) ^ 0x00FF;

  // logic from ADC()
  uint16_t add = (uint16_t) a + (uint16_t) inverted + (uint16_t) get_flag(CARRY);
  sflag(CARRY, add > 0xFF);
  sflag(ZERO, (add & 0x00FF) == 0);
  sflag(NEGATIVE, add & 128);
  // V = ~(a+data) * (a+result)
  sflag(OVERFLOW, (~((uint16_t)a ^ (uint16_t)inverted) & ((uint16_t)a ^ (uint16_t)add)) & 128);

  a = add & 0x00FF;
  return 1;
}

// SEC - Set Carry Flag
// C = 1
// Set the carry flag to one.
uint8_t CPU::SEC() {
  sflag(CARRY, 1);
  return 0;
}

// SED - Set Decimal Flag
// D = 1
// Set the decimal mode flag to one.
uint8_t CPU::SED() {
  sflag(DECIMAL, 1);
  return 0;
}

// SEI - Set Interrupt Disable
// I = 1
// Set the interrupt disable flag to one.
uint8_t CPU::SEI() {
  sflag(INTERRUPT, 1);
  return 0;
}

// STA - Store Accumulator
// M = A
// Stores the contents of the accumulator into memory.
uint8_t CPU::STA() {
  write(addr, a);
  return 0;
}

// STX - Store X Register
// M = X
// Stores the contents of the X register into memory.
uint8_t CPU::STX() {
  write(addr, x);
  return 0;
}

// STY - Store Y Register
// M = Y
// Stores the contents of the Y register into memory.
uint8_t CPU::STY() {
  write(addr, y);
  return 0;
}

// TAX - Transfer Accumulator to X
// X = A
// Copies the current contents of the accumulator into the X register and sets the zero and negative flags as appropriate.
uint8_t CPU::TAX() {
  x = a;
  sflag(ZERO, !x);
  sflag(NEGATIVE, x & 128);
  return 0;
}

// TAY - Transfer Accumulator to Y
// Y = A
// Copies the current contents of the accumulator into the Y register and sets the zero and negative flags as appropriate.
uint8_t CPU::TAY() {
  y = a;
  sflag(ZERO, !y);
  sflag(NEGATIVE, y & 128);
  return 0;
}

// TSX - Transfer Stack Pointer to X
// X = S
// Copies the current contents of the stack register into the X register and sets the zero and negative flags as appropriate.
uint8_t CPU::TSX() {
  x = sp;
  sflag(ZERO, !x);
  sflag(NEGATIVE, x & 128);
  return 0;
}

// TXA - Transfer X to Accumulator
// A = X
// Copies the current contents of the X register into the accumulator and sets the zero and negative flags as appropriate.
uint8_t CPU::TXA() {
  a = x;
  sflag(ZERO, !a);
  sflag(NEGATIVE, a & 128);
  return 0;
}

// TXS - Transfer X to Stack Pointer
// S = X
// Copies the current contents of the X register into the stack register.
uint8_t CPU::TXS() {
  sp = x;
  return 0;
}

// TYA - Transfer Y to Accumulator
// A = Y
// Copies the current contents of the Y register into the accumulator and sets the zero and negative flags as appropriate.
uint8_t CPU::TYA() {
  a = y;
  sflag(ZERO, !a);
  sflag(NEGATIVE, a & 128);
  return 0;
}

// XXX - Illegal opcodes
uint8_t CPU::XXX() {
  return 0;
}

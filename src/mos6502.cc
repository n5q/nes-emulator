#include "mos6502.hh"
#include "bus.hh"

MOS6502::MOS6502() {
    #define INST(name, opcode, addr, cycles) { name, &MOS6502::opcode, &MOS6502::addr, cycles }
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

void MOS6502::connect_bus(Bus *b) {
    return;
}

void MOS6502::write(uint16_t addr, uint8_t data) {
    if (addr >= 0x0000 && addr <= 0xFFFF) {
        return bus->write(addr, data);
    }
}

uint8_t MOS6502::read(uint16_t addr) {
    if (addr >= 0x0000 && addr <= 0xFFFF) {
        return bus->read(addr);
    }
    return 0x00;
}

bool MOS6502::get_flag(FLAG f) {
    return (psr & f) != 0;
}

// void MOS6502::set_flag(FLAG f) {
//     psr = (psr | f);
// }

void MOS6502::set_flag(FLAG f) {
    psr = (psr | f);
}

void MOS6502::clear_flag(FLAG f) {
    psr = (psr & ~f);
}

void MOS6502::sflag(FLAG f, bool b) {
    if (b) {
        set_flag(f);
    } else {
        clear_flag(f);
    }
}

void MOS6502::fetch() {
    m = read(addr);
}

void MOS6502::clk() {
    if (inst_cycles == 0) {
        opcode = read(pc++);
        MOS6502::Instruction inst = lookup[opcode];
        inst_cycles = inst.inst_cycles;

        uint8_t a = (this->*inst.addr_mode)();
        uint8_t b = (this->*inst.opcode)();

        // additional cycles if needed
        inst_cycles += (a & b);
    }
    cycles++;
    inst_cycles--;
}

void MOS6502::reset() {
    a = 0;
    x = 0;
    y = 0;
    sp = 0;
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

void MOS6502::irq() {
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

        cycles = 7;
    }
}

// same as irq but not conditional on I flag and different address to jump to
void MOS6502::nmi() {
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

    cycles = 7;
}

// ADDRESSING MODES

// absolute
uint8_t MOS6502::ABS() {
    uint16_t low = read(pc++);
    uint16_t high = read(pc++) << 8;
    addr = high | low;
    return 0;
}

// absolute w/ x offset
uint8_t MOS6502::ABX() {
    uint16_t low = read(pc++);
    uint16_t high = read(pc++) << 8;
    addr = (high | low) + x;
    // check if we have crossed into a new page (high byte) and need a extra clock cycle
    return ((addr & 0xFF00) != high) ? 1 : 0;
}

// absolute w/ y offset
uint8_t MOS6502::ABY() {
    uint16_t low = read(pc++);
    uint16_t high = read(pc++) << 8;
    addr = (high | low) + y;
    // check if we have crossed into a new page (high byte) and need a extra clock cycle
    return ((addr & 0xFF00) != high) ? 1 : 0;
}

// immediate
uint8_t MOS6502::IMM() { 
    addr = pc++;
    return 0;
}

// implied (accumulator)
uint8_t MOS6502::IMP() {
    m = a;
    return 0;
}

// indirect (like pointer, only used by JMP instruction)
uint8_t MOS6502::IND() {
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
uint8_t MOS6502::IDX() {
    uint8_t addr_zp = read(pc++);
    uint8_t ptr = (addr_zp + x) & 0xFF; // wrap around zp

    uint16_t low = read(ptr);
    uint16_t high = read((ptr + 1) & 0xFF) << 8;

    addr = high | low;
    return 0;
} 

// indirect indexed, zero page offset by y
uint8_t MOS6502::IDY() {
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
uint8_t MOS6502::REL() {
    int8_t offset = (int8_t) read(pc++);
    addr_branch = pc + offset;
    return 0;
}

// zero page
uint8_t MOS6502::ZPG() {
    addr = read(pc++) & 0x00FF;
    return 0;
}

// zero page, x offset
uint8_t MOS6502::ZPX() {
    addr = (read(pc++) + x) & 0x00FF;
    return 0;
}

// zero page, y offset
uint8_t MOS6502::ZPY() {
    addr = (read(pc++) + y) & 0x00FF;
    return 0;
}


// INSTRUCTIONS
// http://www.6502.org/users/obelisk/6502/reference.html

// ADC - Add with Carry
// A,Z,C,N = A+M+C
// This instruction adds the contents of a memory location to the accumulator together with the carry bit. If overflow occurs the carry bit is set, this enables multiple byte addition to be performed.

uint8_t MOS6502::ADC() {
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
uint8_t MOS6502::AND() {
    fetch();
    a = a & m;

    sflag(ZERO, !a);
    sflag(NEGATIVE, a & (1<<7));

    return 1;
}

// ASL - Arithmetic Shift Left
// A,Z,C,N = M*2 or M,Z,C,N = M*2
// This operation shifts all the bits of the accumulator or memory contents one bit left. Bit 0 is set to 0 and bit 7 is placed in the carry flag. The effect of this operation is to multiply the memory contents by 2 (ignoring 2's complement considerations), setting the carry if the result will not fit in 8 bits.
uint8_t MOS6502::ASL() {
    fetch();

    sflag(CARRY, m & (1 << 7));

    uint8_t tmp = m << 1;
    if (lookup[opcode].addr_mode == &MOS6502::IMP) {
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
uint8_t MOS6502::BCC() {
    fetch();

    if (!get_flag(CARRY)) {
        cycles++;
        addr_branch = pc + addr;

        // if we cross into new page then take another cycle
        if ((addr_branch & 0xFF00) != (pc & 0xFF00)) {
            cycles++;
        }

        pc = addr_branch;
    }

    return 0;
}

// BCS - Branch if Carry Set
// If the carry flag is set then add the relative displacement to the program counter to cause a branch to a new location.
uint8_t MOS6502::BCS() {
    fetch();

    if (get_flag(CARRY)) {
        cycles++;
        addr_branch = pc + addr;

        // if we cross into new page then take another cycle
        if ((addr_branch & 0xFF00) != (pc & 0xFF00)) {
            cycles++;
        }

        pc = addr_branch;
    }

    return 0;
}

// BEQ - Branch if Equal
// If the zero flag is set then add the relative displacement to the program counter to cause a branch to a new location.
uint8_t MOS6502::BEQ() {
    fetch();

    if (get_flag(ZERO)) {
        cycles++;
        addr_branch = pc + addr;

        // if we cross into new page then take another cycle
        if ((addr_branch & 0xFF00) != (pc & 0xFF00)) {
            cycles++;
        }

        pc = addr_branch;
    }

    return 0;
}

// BIT - Bit Test
// A & M, N = M7, V = M6
// This instructions is used to test if one or more bits are set in a target memory location. The mask pattern in A is ANDed with the value in memory to set or clear the zero flag, but the result is not kept. Bits 7 and 6 of the value from memory are copied into the N and V flags.
uint8_t MOS6502::BIT() {
    fetch();

    uint8_t bit = a & m;
    sflag(ZERO, !(bit & 0x00FF));
    sflag(NEGATIVE, m & (1<<7));
    sflag(OVERFLOW, m & (1<<8));

    return 0;
}

// BMI - Branch if Minus
// If the negative flag is set then add the relative displacement to the program counter to cause a branch to a new location.
uint8_t MOS6502::BMI() {
    fetch();

    if (get_flag(NEGATIVE)) {
        cycles++;
        addr_branch = pc + addr;

        // if we cross into new page then take another cycle
        if ((addr_branch & 0xFF00) != (pc & 0xFF00)) {
            cycles++;
        }

        pc = addr_branch;
    }

    return 0;
}

// BNE - Branch if Not Equal
// If the zero flag is clear then add the relative displacement to the program counter to cause a branch to a new location.
uint8_t MOS6502::BNE() {
    fetch();

    if (!get_flag(ZERO)) {
        cycles++;
        addr_branch = pc + addr;

        // if we cross into new page then take another cycle
        if ((addr_branch & 0xFF00) != (pc & 0xFF00)) {
            cycles++;
        }

        pc = addr_branch;
    }

    return 0;
}

// BPL - Branch if Positive
// If the negative flag is clear then add the relative displacement to the program counter to cause a branch to a new location.
uint8_t MOS6502::BPL() {
    fetch();

    if (!get_flag(NEGATIVE)) {
        cycles++;
        addr_branch = pc + addr;

        // if we cross into new page then take another cycle
        if ((addr_branch & 0xFF00) != (pc & 0xFF00)) {
            cycles++;
        }

        pc = addr_branch;
    }

    return 0;
}

// BRK - Force Interrupt
// The BRK instruction forces the generation of an interrupt request. The program counter and processor status are pushed on the stack then the IRQ interrupt vector at $FFFE/F is loaded into the PC and the break flag in the status set to one.
uint8_t MOS6502::BRK() {
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
uint8_t MOS6502::BVC() {
    fetch();

    if (!get_flag(OVERFLOW)) {
        cycles++;
        addr_branch = pc + addr;

        // if we cross into new page then take another cycle
        if ((addr_branch & 0xFF00) != (pc & 0xFF00)) {
            cycles++;
        }

        pc = addr_branch;
    }

    return 0;
}

// BVS - Branch if Overflow Set
// If the overflow flag is set then add the relative displacement to the program counter to cause a branch to a new location.
uint8_t MOS6502::BVC() {
    fetch();

    if (get_flag(OVERFLOW)) {
        cycles++;
        addr_branch = pc + addr;

        // if we cross into new page then take another cycle
        if ((addr_branch & 0xFF00) != (pc & 0xFF00)) {
            cycles++;
        }

        pc = addr_branch;
    }

    return 0;
}

// CLC - Clear Carry Flag
// C = 0
// Set the carry flag to zero.
uint8_t MOS6502::CLC() {
    sflag(CARRY, 0);
}

// CLD - Clear Decimal Mode
// D = 0
// Sets the decimal mode flag to zero.
uint8_t MOS6502::CLD() {
    sflag(DECIMAL, 0);
}

// CLI - Clear Interrupt Disable
// I = 0
// Clears the interrupt disable flag allowing normal interrupt requests to be serviced.
uint8_t MOS6502::CLI() {
    sflag(INTERRUPT, 0);
}

// CLV - Clear Overflow Flag
// V = 0
// Clears the overflow flag.
uint8_t MOS6502::CLV() {
    sflag(OVERFLOW, 0);
}

//! CMP - Compare
// Z,C,N = A-M
// This instruction compares the contents of the accumulator with another memory held value and sets the zero and carry flags as appropriate.

// RTI - Return from Interrupt
// The RTI instruction is used at the end of an interrupt processing routine. It pulls the processor flags from the stack followed by the program counter.
uint8_t MOS6502::RTI() {
    // read the psr and pc which we pushed to stack in irq() or nmi()
    sp++;
    psr = read(0x0100 + sp);
    psr &= ~BREAK;
    psr &= ~UNUSED;

    sp++;
    pc = (uint16_t) read(0x0100 + sp);
    sp++;
    pc |= (uint16_t) read((0x0100 + sp) << 8);
    return 0;
}



// SBC - Subtract with Carry
// A,Z,C,N = A-M-(1-C)
// This instruction subtracts the contents of a memory location to the accumulator together with the not of the carry bit. If overflow occurs the carry bit is clear, this enables multiple byte subtraction to be performed.
uint8_t MOS6502::SBC() {
    fetch();

    // invert bottom 8 bits
    uint16_t inverted = ((uint16_t) m) ^ 0x00FF;
    m = inverted;

    // subtracting equivalent to adding inverse
    this->ADC();
    
    return 1;
}



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

uint8_t MOS6502::fetch() {
    fetched = read(addr);
    return fetched;
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
    fetched = a;
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

// ! ADC - Add with Carry
// A,Z,C,N = A+M+C
// This instruction adds the contents of a memory location to the accumulator together with the carry bit. If overflow occurs the carry bit is set, this enables multiple byte addition to be performed.

uint8_t MOS6502::ADC() {
    fetch();
    // uint16_t add = (uint16_t) a + (uint16_t) fetched + (uint16_t) get_flag(FLAG::CARRY);
    return 1;
}

// AND - Logical AND
// A,Z,N = A&M
// A logical AND is performed, bit by bit, on the accumulator contents using the contents of a byte of memory
uint8_t MOS6502::AND() {
    uint8_t m = fetch();
    a = a & m;

    sflag(ZERO, !a);
    sflag(NEGATIVE, a & (1<<7));

    return 1;
}

// ASL - Arithmetic Shift Left
// A,Z,C,N = M*2 or M,Z,C,N = M*2
// This operation shifts all the bits of the accumulator or memory contents one bit left. Bit 0 is set to 0 and bit 7 is placed in the carry flag. The effect of this operation is to multiply the memory contents by 2 (ignoring 2's complement considerations), setting the carry if the result will not fit in 8 bits.
uint8_t MOS6502::ASL() {
    uint8_t m = fetch();

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





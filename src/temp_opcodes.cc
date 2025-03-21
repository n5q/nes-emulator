#define INST(name, op, addr, cyc) { name, &a::op, &a::addr, cyc }

lookup = {
    // 0x00
    INST("BRK", BRK, IMM, 7),  INST("ORA", ORA, IZX, 6),  INST("???", XXX, IMP, 2),  INST("???", XXX, IMP, 8),
    INST("???", NOP, IMP, 3),  INST("ORA", ORA, ZP0, 3),  INST("ASL", ASL, ZP0, 5),  INST("???", XXX, IMP, 5),
    INST("PHP", PHP, IMP, 3),  INST("ORA", ORA, IMM, 2),  INST("ASL", ASL, IMP, 2),  INST("???", XXX, IMP, 2),
    INST("???", NOP, IMP, 4),  INST("ORA", ORA, ABS, 4),  INST("ASL", ASL, ABS, 6),  INST("???", XXX, IMP, 6),

    // 0x10
    INST("BPL", BPL, REL, 2),  INST("ORA", ORA, IZY, 5),  INST("???", XXX, IMP, 2),  INST("???", XXX, IMP, 8),
    INST("???", NOP, IMP, 4),  INST("ORA", ORA, ZPX, 4),  INST("ASL", ASL, ZPX, 6),  INST("???", XXX, IMP, 6),
    INST("CLC", CLC, IMP, 2),  INST("ORA", ORA, ABY, 4),  INST("???", NOP, IMP, 2),  INST("???", XXX, IMP, 7),
    INST("???", NOP, IMP, 4),  INST("ORA", ORA, ABX, 4),  INST("ASL", ASL, ABX, 7),  INST("???", XXX, IMP, 7),

    // 0x20
    INST("JSR", JSR, ABS, 6),  INST("AND", AND, IZX, 6),  INST("???", XXX, IMP, 2),  INST("???", XXX, IMP, 8),
    INST("BIT", BIT, ZP0, 3),  INST("AND", AND, ZP0, 3),  INST("ROL", ROL, ZP0, 5),  INST("???", XXX, IMP, 5),
    INST("PLP", PLP, IMP, 4),  INST("AND", AND, IMM, 2),  INST("ROL", ROL, IMP, 2),  INST("???", XXX, IMP, 2),
    INST("BIT", BIT, ABS, 4),  INST("AND", AND, ABS, 4),  INST("ROL", ROL, ABS, 6),  INST("???", XXX, IMP, 6),

    // 0x30
    INST("BMI", BMI, REL, 2),  INST("AND", AND, IZY, 5),  INST("???", XXX, IMP, 2),  INST("???", XXX, IMP, 8),
    INST("???", NOP, IMP, 4),  INST("AND", AND, ZPX, 4),  INST("ROL", ROL, ZPX, 6),  INST("???", XXX, IMP, 6),
    INST("SEC", SEC, IMP, 2),  INST("AND", AND, ABY, 4),  INST("???", NOP, IMP, 2),  INST("???", XXX, IMP, 7),
    INST("???", NOP, IMP, 4),  INST("AND", AND, ABX, 4),  INST("ROL", ROL, ABX, 7),  INST("???", XXX, IMP, 7),

    // 0x40
    INST("RTI", RTI, IMP, 6),  INST("EOR", EOR, IZX, 6),  INST("???", XXX, IMP, 2),  INST("???", XXX, IMP, 8),
    INST("???", NOP, IMP, 3),  INST("EOR", EOR, ZP0, 3),  INST("LSR", LSR, ZP0, 5),  INST("???", XXX, IMP, 5),
    INST("PHA", PHA, IMP, 3),  INST("EOR", EOR, IMM, 2),  INST("LSR", LSR, IMP, 2),  INST("???", XXX, IMP, 2),
    INST("JMP", JMP, ABS, 3),  INST("EOR", EOR, ABS, 4),  INST("LSR", LSR, ABS, 6),  INST("???", XXX, IMP, 6),

    // 0x50
    INST("BVC", BVC, REL, 2),  INST("EOR", EOR, IZY, 5),  INST("???", XXX, IMP, 2),  INST("???", XXX, IMP, 8),
    INST("???", NOP, IMP, 4),  INST("EOR", EOR, ZPX, 4),  INST("LSR", LSR, ZPX, 6),  INST("???", XXX, IMP, 6),
    INST("CLI", CLI, IMP, 2),  INST("EOR", EOR, ABY, 4),  INST("???", NOP, IMP, 2),  INST("???", XXX, IMP, 7),
    INST("???", NOP, IMP, 4),  INST("EOR", EOR, ABX, 4),  INST("LSR", LSR, ABX, 7),  INST("???", XXX, IMP, 7),

    // 0x60
    INST("RTS", RTS, IMP, 6),  INST("ADC", ADC, IZX, 6),  INST("???", XXX, IMP, 2),  INST("???", XXX, IMP, 8),
    INST("???", NOP, IMP, 3),  INST("ADC", ADC, ZP0, 3),  INST("ROR", ROR, ZP0, 5),  INST("???", XXX, IMP, 5),
    INST("PLA", PLA, IMP, 4),  INST("ADC", ADC, IMM, 2),  INST("ROR", ROR, IMP, 2),  INST("???", XXX, IMP, 2),
    INST("JMP", JMP, IND, 5),  INST("ADC", ADC, ABS, 4),  INST("ROR", ROR, ABS, 6),  INST("???", XXX, IMP, 6),

    // 0x70
    INST("BVS", BVS, REL, 2),  INST("ADC", ADC, IZY, 5),  INST("???", XXX, IMP, 2),  INST("???", XXX, IMP, 8),
    INST("???", NOP, IMP, 4),  INST("ADC", ADC, ZPX, 4),  INST("ROR", ROR, ZPX, 6),  INST("???", XXX, IMP, 6),
    INST("SEI", SEI, IMP, 2),  INST("ADC", ADC, ABY, 4),  INST("???", NOP, IMP, 2),  INST("???", XXX, IMP, 7),
    INST("???", NOP, IMP, 4),  INST("ADC", ADC, ABX, 4),  INST("ROR", ROR, ABX, 7),  INST("???", XXX, IMP, 7),

    // 0x80
    INST("???", NOP, IMP, 2),  INST("STA", STA, IZX, 6),  INST("???", NOP, IMP, 2),  INST("???", XXX, IMP, 6),
    INST("STY", STY, ZP0, 3),  INST("STA", STA, ZP0, 3),  INST("STX", STX, ZP0, 3),  INST("???", XXX, IMP, 3),
    INST("DEY", DEY, IMP, 2),  INST("???", NOP, IMP, 2),  INST("TXA", TXA, IMP, 2),  INST("???", XXX, IMP, 2),
    INST("STY", STY, ABS, 4),  INST("STA", STA, ABS, 4),  INST("STX", STX, ABS, 4),  INST("???", XXX, IMP, 4),

    // 0x90
    INST("BCC", BCC, REL, 2),  INST("STA", STA, IZY, 6),  INST("???", XXX, IMP, 2),  INST("???", XXX, IMP, 6),
    INST("STY", STY, ZPX, 4),  INST("STA", STA, ZPX, 4),  INST("STX", STX, ZPY, 4),  INST("???", XXX, IMP, 4),
    INST("TYA", TYA, IMP, 2),  INST("STA", STA, ABY, 5),  INST("TXS", TXS, IMP, 2),  INST("???", XXX, IMP, 5),
    INST("???", NOP, IMP, 5),  INST("STA", STA, ABX, 5),  INST("???", XXX, IMP, 5),  INST("???", XXX, IMP, 5),

    // 0xA0
    INST("LDY", LDY, IMM, 2),  INST("LDA", LDA, IZX, 6),  INST("LDX", LDX, IMM, 2),  INST("???", XXX, IMP, 6),
    INST("LDY", LDY, ZP0, 3),  INST("LDA", LDA, ZP0, 3),  INST("LDX", LDX, ZP0, 3),  INST("???", XXX, IMP, 3),
    INST("TAY", TAY, IMP, 2),  INST("LDA", LDA, IMM, 2),  INST("TAX", TAX, IMP, 2),  INST("???", XXX, IMP, 2),
    INST("LDY", LDY, ABS, 4),  INST("LDA", LDA, ABS, 4),  INST("LDX", LDX, ABS, 4),  INST("???", XXX, IMP, 4),

    // 0xB0
    INST("BCS", BCS, REL, 2),  INST("LDA", LDA, IZY, 5),  INST("???", XXX, IMP, 2),  INST("???", XXX, IMP, 5),
    INST("LDY", LDY, ZPX, 4),  INST("LDA", LDA, ZPX, 4),  INST("LDX", LDX, ZPY, 4),  INST("???", XXX, IMP, 4),
    INST("CLV", CLV, IMP, 2),  INST("LDA", LDA, ABY, 4),  INST("TSX", TSX, IMP, 2),  INST("???", XXX, IMP, 4),
    INST("LDY", LDY, ABX, 4),  INST("LDA", LDA, ABX, 4),  INST("LDX", LDX, ABY, 4),  INST("???", XXX, IMP, 4),

    // 0xC0
    INST("CPY", CPY, IMM, 2),  INST("CMP", CMP, IZX, 6),  INST("???", NOP, IMP, 2),  INST("???", XXX, IMP, 8),
    INST("CPY", CPY, ZP0, 3),  INST("CMP", CMP, ZP0, 3),  INST("DEC", DEC, ZP0, 5),  INST("???", XXX, IMP, 5),
    INST("INY", INY, IMP, 2),  INST("CMP", CMP, IMM, 2),  INST("DEX", DEX, IMP, 2),  INST("???", XXX, IMP, 2),
    INST("CPY", CPY, ABS, 4),  INST("CMP", CMP, ABS, 4),  INST("DEC", DEC, ABS, 6),  INST("???", XXX, IMP, 6),

    // 0xD0
    INST("BNE", BNE, REL, 2),  INST("CMP", CMP, IZY, 5),  INST("???", XXX, IMP, 2),  INST("???", XXX, IMP, 8),
    INST("???", NOP, IMP, 4),  INST("CMP", CMP, ZPX, 4),  INST("DEC", DEC, ZPX, 6),  INST("???", XXX, IMP, 6),
    INST("CLD", CLD, IMP, 2),  INST("CMP", CMP, ABY, 4),  INST("NOP", NOP, IMP, 2),  INST("???", XXX, IMP, 7),
    INST("???", NOP, IMP, 4),  INST("CMP", CMP, ABX, 4),  INST("DEC", DEC, ABX, 7),  INST("???", XXX, IMP, 7),

    // 0xE0
    INST("CPX", CPX, IMM, 2),  INST("SBC", SBC, IZX, 6),  INST("???", NOP, IMP, 2),  INST("???", XXX, IMP, 8),
    INST("CPX", CPX, ZP0, 3),  INST("SBC", SBC, ZP0, 3),  INST("INC", INC, ZP0, 5),  INST("???", XXX, IMP, 5),
    INST("INX", INX, IMP, 2),  INST("SBC", SBC, IMM, 2),  INST("NOP", NOP, IMP, 2),  INST("???", SBC, IMP, 2),
    INST("CPX", CPX, ABS, 4),  INST("SBC", SBC, ABS, 4),  INST("INC", INC, ABS, 6),  INST("???", XXX, IMP, 6),

    // 0xF0
    INST("BEQ", BEQ, REL, 2),  INST("SBC", SBC, IZY, 5),  INST("???", XXX, IMP, 2),  INST("???", XXX, IMP, 8),
    INST("???", NOP, IMP, 4),  INST("SBC", SBC, ZPX, 4),  INST("INC", INC, ZPX, 6),  INST("???", XXX, IMP, 6),
    INST("SED", SED, IMP, 2),  INST("SBC", SBC, ABY, 4),  INST("NOP", NOP, IMP, 2),  INST("???", XXX, IMP, 7),
    INST("???", NOP, IMP, 4),  INST("SBC", SBC, ABX, 4),  INST("INC", INC, ABX, 7),  INST("???", XXX, IMP, 7)
};

#undef INST

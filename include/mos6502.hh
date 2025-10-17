#ifndef MOS6502_HH
#define MOS6502_HH

#include <cstdint>
#include <string>
#include <vector>

class Bus;

class MOS6502 {
public:
    MOS6502();
    ~MOS6502();

    void connect_bus(Bus *b);

    // registers
    uint8_t  a   = 0x00;   // accumulator
    uint8_t  x   = 0x00;   // x register
    uint8_t  y   = 0x00;   // y register
    uint8_t  sp  = 0x00;   // stack pointer
    uint16_t pc  = 0x0000; // program counter
    uint8_t  psr = 0x00;   // program status register

    // clock 
    void clk();

    // interrupts
    void reset();
    void irq();
    void nmi();

    // flags which are stored in the psr
    enum FLAG : uint8_t {
        CARRY     = 1 << 0,
        ZERO      = 1 << 1,
        INTERRUPT = 1 << 2,
        DECIMAL   = 1 << 3,
        BREAK     = 1 << 4,
        UNUSED    = 1 << 5, // always 1, but unused
        OVERFLOW  = 1 << 6,
        NEGATIVE  = 1 << 7 
    };

    // 13 addressing modes in the 6502, we can handle accumulator and implied as the same
    // each mode takes a certain number of clock cycles as overhead, so we need to account for that
    // https://wiki.cdot.senecapolytechnic.ca/wiki/6502_Addressing_Modes

    uint8_t ABS();  uint8_t ABX();  uint8_t ABY();
    uint8_t IMM();  uint8_t IMP();  uint8_t IND();
    uint8_t IDX();  uint8_t IDY();  uint8_t REL();
    uint8_t ZPG();  uint8_t ZPX();  uint8_t ZPY();

    // opcodes, 56 legal and 169 illegal
    // https://www.masswerk.at/6502/6502_instruction_set.html#

    uint8_t ADC();	uint8_t AND();	uint8_t ASL();	uint8_t BCC();
	uint8_t BCS();	uint8_t BEQ();	uint8_t BIT();	uint8_t BMI();
	uint8_t BNE();	uint8_t BPL();	uint8_t BRK();	uint8_t BVC();
	uint8_t BVS();	uint8_t CLC();	uint8_t CLD();	uint8_t CLI();
	uint8_t CLV();	uint8_t CMP();	uint8_t CPX();	uint8_t CPY();
	uint8_t DEC();	uint8_t DEX();	uint8_t DEY();	uint8_t EOR();
	uint8_t INC();	uint8_t INX();	uint8_t INY();	uint8_t JMP();
	uint8_t JSR();	uint8_t LDA();	uint8_t LDX();	uint8_t LDY();
	uint8_t LSR();	uint8_t NOP();	uint8_t ORA();	uint8_t PHA();
	uint8_t PHP();	uint8_t PLA();	uint8_t PLP();	uint8_t ROL();
	uint8_t ROR();	uint8_t RTI();	uint8_t RTS();	uint8_t SBC();
	uint8_t SEC();	uint8_t SED();	uint8_t SEI();	uint8_t STA();
	uint8_t STX();	uint8_t STY();	uint8_t TAX();	uint8_t TAY();
	uint8_t TSX();	uint8_t TXA();	uint8_t TXS();	uint8_t TYA();

    // illegal opcodes
    uint8_t XXX();

    uint8_t opcode = 0x00;
    uint16_t addr = 0x0000;
    uint16_t addr_branch = 0x0000;
    int8_t disp = 0x00; // displacement for branches
    uint8_t cycles = 0;
    uint8_t inst_cycles = 0;

    uint8_t fetch();
    uint8_t fetched = 0x00;

private:
    struct Instruction {
        std::string inst_name;
        uint8_t (MOS6502::*opcode)();
        uint8_t (MOS6502::*addr_mode)();
        uint8_t inst_cycles;
    };

    std::vector<MOS6502::Instruction> lookup;

    Bus *bus = nullptr;
    void write(uint16_t addr, uint8_t data);
    uint8_t read(uint16_t addr);

    bool get_flag(FLAG f);
    void set_flag(FLAG f);
    void clear_flag(FLAG f);
    void sflag(FLAG f, bool b);
};

#endif
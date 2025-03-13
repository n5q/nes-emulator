#ifndef MOS6502
#define MOS6502
#endif

#include <cstdint>
#include "bus.hh"

class MOS6502 {
public:
    MOS6502();
    ~MOS6502();

    void connect_bus(Bus *b);

private:
    Bus *bus;
};
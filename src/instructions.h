#include <stdint.h>

enum instruction_name {
    INSTRUCTION_NONE,
    ADC, AND, ASL, BCC, BCS, BEQ, BIT, BMI, BNE, BPL, BRK, BVC, BVS, CLC,
    CLD, CLI, CLV, CMP, CPX, CPY, DEC, DEX, DEY, EOR, INC, INX, INY, JMP,
    JSR, LDA, LDX, LDY, LSR, NOP, ORA, PHA, PHP, PLA, PLP, ROL, ROR, RTI,
    RTS, SBC, SEC, SED, SEI, STA, STX, STY, TAX, TAY, TSX, TXA, TXS, TYA,
};

const char *INSTRUCTION_NAME_STRING[] = {
    "INSTRUCTION_NONE",
    "ADC", "AND", "ASL", "BCC", "BCS", "BEQ", "BIT", "BMI", "BNE", "BPL", "BRK", "BVC", "BVS", "CLC",
    "CLD", "CLI", "CLV", "CMP", "CPX", "CPY", "DEC", "DEX", "DEY", "EOR", "INC", "INX", "INY", "JMP",
    "JSR", "LDA", "LDX", "LDY", "LSR", "NOP", "ORA", "PHA", "PHP", "PLA", "PLP", "ROL", "ROR", "RTI",
    "RTS", "SBC", "SEC", "SED", "SEI", "STA", "STX", "STY", "TAX", "TAY", "TSX", "TXA", "TXS", "TYA",
};

enum address_mode {
    ADDRESS_MODE_NONE,
    IMPLICIT,               // No operand
    IMMEDIATE,              // Literal value
    ACCUMULATOR,            // 'A' register

    RELATIVE,               // Signed 8-bit offset from current PC when branching
    ZERO_PAGE,              //  8-bit pointer (into zero_page)
    ABSOLUTE,               // 16-bit pointer
    INDIRECT,               // Set PC to 16-bit value stored at location pointed to by 16-bit operand

    ZERO_PAGE_X,            //  8-bit Operand plus 'X' register contents
    ZERO_PAGE_Y,            //  8-bit Operand plus 'Y' register contents
    ABSOLUTE_X,             // 16-bit Operand plus 'X' register contents
    ABSOLUTE_Y,             // 16-bit Operand plus 'X' register contents

    INDEXED_INDIRECT,       //  8-bit value pointed to by 16-bit pointer stored in zero_page pointed to by Operand plus 'X' register contents
    INDIRECT_INDEXED,       //  8-bit value pointed to by (8-bit operand + 'Y' register contents)
};

const char *ADDRESS_MODE_STRING[] = {
    "ADDRESS_MODE_NONE",
    "IMPLICIT",
    "IMMEDIATE",
    "ACCUMULATOR",
    "RELATIVE",
    "ZERO_PAGE",
    "ABSOLUTE",
    "INDIRECT",
    "ZERO_PAGE_X",
    "ZERO_PAGE_Y",
    "ABSOLUTE_X",
    "ABSOLUTE_Y",
    "INDEXED_INDIRECT",
    "INDIRECT_INDEXED",
};

const enum instruction_name INSTRUCTION_LOOKUP[256] = {
    BRK,                  // 0x00 (IMPLICIT)
    ORA,                  // 0x01 (INDEXED_INDIRECT)
    INSTRUCTION_NONE,     // 0x02 (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0x03 (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0x04 (ADDRESS_MODE_NONE)
    ORA,                  // 0x05 (ZERO_PAGE)
    ASL,                  // 0x06 (ZERO_PAGE)
    INSTRUCTION_NONE,     // 0x07 (ADDRESS_MODE_NONE)
    PHP,                  // 0x08 (IMPLICIT)
    ORA,                  // 0x09 (IMMEDIATE)
    ASL,                  // 0x0A (ACCUMULATOR)
    INSTRUCTION_NONE,     // 0x0B (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0x0C (ADDRESS_MODE_NONE)
    ORA,                  // 0x0D (ABSOLUTE)
    ASL,                  // 0x0E (ABSOLUTE)
    INSTRUCTION_NONE,     // 0x0F (ADDRESS_MODE_NONE)
    BPL,                  // 0x10 (RELATIVE)
    ORA,                  // 0x11 (INDIRECT_INDEXED)
    INSTRUCTION_NONE,     // 0x12 (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0x13 (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0x14 (ADDRESS_MODE_NONE)
    ORA,                  // 0x15 (ZERO_PAGE_X)
    ASL,                  // 0x16 (ZERO_PAGE_X)
    INSTRUCTION_NONE,     // 0x17 (ADDRESS_MODE_NONE)
    CLC,                  // 0x18 (IMPLICIT)
    ORA,                  // 0x19 (ABSOLUTE_Y)
    INSTRUCTION_NONE,     // 0x1A (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0x1B (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0x1C (ADDRESS_MODE_NONE)
    ORA,                  // 0x1D (ABSOLUTE_X)
    ASL,                  // 0x1E (ABSOLUTE_X)
    INSTRUCTION_NONE,     // 0x1F (ADDRESS_MODE_NONE)
    JSR,                  // 0x20 (ABSOLUTE)
    AND,                  // 0x21 (INDEXED_INDIRECT)
    INSTRUCTION_NONE,     // 0x22 (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0x23 (ADDRESS_MODE_NONE)
    BIT,                  // 0x24 (ZERO_PAGE)
    AND,                  // 0x25 (ZERO_PAGE)
    ROL,                  // 0x26 (ZERO_PAGE)
    INSTRUCTION_NONE,     // 0x27 (ADDRESS_MODE_NONE)
    PLP,                  // 0x28 (IMPLICIT)
    AND,                  // 0x29 (IMMEDIATE)
    ROL,                  // 0x2A (ACCUMULATOR)
    INSTRUCTION_NONE,     // 0x2B (ADDRESS_MODE_NONE)
    BIT,                  // 0x2C (ABSOLUTE)
    AND,                  // 0x2D (ABSOLUTE)
    ROL,                  // 0x2E (ABSOLUTE)
    INSTRUCTION_NONE,     // 0x2F (ADDRESS_MODE_NONE)
    BMI,                  // 0x30 (RELATIVE)
    AND,                  // 0x31 (INDIRECT_INDEXED)
    INSTRUCTION_NONE,     // 0x32 (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0x33 (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0x34 (ADDRESS_MODE_NONE)
    AND,                  // 0x35 (ZERO_PAGE_X)
    ROL,                  // 0x36 (ZERO_PAGE_X)
    INSTRUCTION_NONE,     // 0x37 (ADDRESS_MODE_NONE)
    SEC,                  // 0x38 (IMPLICIT)
    AND,                  // 0x39 (ABSOLUTE_Y)
    INSTRUCTION_NONE,     // 0x3A (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0x3B (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0x3C (ADDRESS_MODE_NONE)
    AND,                  // 0x3D (ABSOLUTE_X)
    ROL,                  // 0x3E (ABSOLUTE_X)
    INSTRUCTION_NONE,     // 0x3F (ADDRESS_MODE_NONE)
    RTI,                  // 0x40 (IMPLICIT)
    EOR,                  // 0x41 (INDEXED_INDIRECT)
    INSTRUCTION_NONE,     // 0x42 (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0x43 (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0x44 (ADDRESS_MODE_NONE)
    EOR,                  // 0x45 (ZERO_PAGE)
    LSR,                  // 0x46 (ZERO_PAGE)
    INSTRUCTION_NONE,     // 0x47 (ADDRESS_MODE_NONE)
    PHA,                  // 0x48 (IMPLICIT)
    EOR,                  // 0x49 (IMMEDIATE)
    LSR,                  // 0x4A (ACCUMULATOR)
    INSTRUCTION_NONE,     // 0x4B (ADDRESS_MODE_NONE)
    JMP,                  // 0x4C (ABSOLUTE)
    EOR,                  // 0x4D (ABSOLUTE)
    LSR,                  // 0x4E (ABSOLUTE)
    INSTRUCTION_NONE,     // 0x4F (ADDRESS_MODE_NONE)
    BVC,                  // 0x50 (RELATIVE)
    EOR,                  // 0x51 (INDIRECT_INDEXED)
    INSTRUCTION_NONE,     // 0x52 (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0x53 (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0x54 (ADDRESS_MODE_NONE)
    EOR,                  // 0x55 (ZERO_PAGE_X)
    LSR,                  // 0x56 (ZERO_PAGE_X)
    INSTRUCTION_NONE,     // 0x57 (ADDRESS_MODE_NONE)
    CLI,                  // 0x58 (IMPLICIT)
    EOR,                  // 0x59 (ABSOLUTE_Y)
    INSTRUCTION_NONE,     // 0x5A (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0x5B (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0x5C (ADDRESS_MODE_NONE)
    EOR,                  // 0x5D (ABSOLUTE_X)
    LSR,                  // 0x5E (ABSOLUTE_X)
    INSTRUCTION_NONE,     // 0x5F (ADDRESS_MODE_NONE)
    RTS,                  // 0x60 (IMPLICIT)
    ADC,                  // 0x61 (INDEXED_INDIRECT)
    INSTRUCTION_NONE,     // 0x62 (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0x63 (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0x64 (ADDRESS_MODE_NONE)
    ADC,                  // 0x65 (ZERO_PAGE)
    ROR,                  // 0x66 (ZERO_PAGE)
    INSTRUCTION_NONE,     // 0x67 (ADDRESS_MODE_NONE)
    PLA,                  // 0x68 (IMPLICIT)
    ADC,                  // 0x69 (IMMEDIATE)
    ROR,                  // 0x6A (ACCUMULATOR)
    INSTRUCTION_NONE,     // 0x6B (ADDRESS_MODE_NONE)
    JMP,                  // 0x6C (INDIRECT)
    ADC,                  // 0x6D (ABSOLUTE)
    ROR,                  // 0x6E (ABSOLUTE)
    INSTRUCTION_NONE,     // 0x6F (ADDRESS_MODE_NONE)
    BVS,                  // 0x70 (RELATIVE)
    ADC,                  // 0x71 (INDIRECT_INDEXED)
    INSTRUCTION_NONE,     // 0x72 (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0x73 (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0x74 (ADDRESS_MODE_NONE)
    ADC,                  // 0x75 (ZERO_PAGE_X)
    ROR,                  // 0x76 (ZERO_PAGE_X)
    INSTRUCTION_NONE,     // 0x77 (ADDRESS_MODE_NONE)
    SEI,                  // 0x78 (IMPLICIT)
    ADC,                  // 0x79 (ABSOLUTE_Y)
    INSTRUCTION_NONE,     // 0x7A (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0x7B (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0x7C (ADDRESS_MODE_NONE)
    ADC,                  // 0x7D (ABSOLUTE_X)
    ROR,                  // 0x7E (ABSOLUTE_X)
    INSTRUCTION_NONE,     // 0x7F (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0x80 (ADDRESS_MODE_NONE)
    STA,                  // 0x81 (INDEXED_INDIRECT)
    INSTRUCTION_NONE,     // 0x82 (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0x83 (ADDRESS_MODE_NONE)
    STY,                  // 0x84 (ZERO_PAGE)
    STA,                  // 0x85 (ZERO_PAGE)
    STX,                  // 0x86 (ZERO_PAGE)
    INSTRUCTION_NONE,     // 0x87 (ADDRESS_MODE_NONE)
    DEY,                  // 0x88 (IMPLICIT)
    INSTRUCTION_NONE,     // 0x89 (ADDRESS_MODE_NONE)
    TXA,                  // 0x8A (IMPLICIT)
    INSTRUCTION_NONE,     // 0x8B (ADDRESS_MODE_NONE)
    STY,                  // 0x8C (ABSOLUTE)
    STA,                  // 0x8D (ABSOLUTE)
    STX,                  // 0x8E (ABSOLUTE)
    INSTRUCTION_NONE,     // 0x8F (ADDRESS_MODE_NONE)
    BCC,                  // 0x90 (RELATIVE)
    STA,                  // 0x91 (INDIRECT_INDEXED)
    INSTRUCTION_NONE,     // 0x92 (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0x93 (ADDRESS_MODE_NONE)
    STY,                  // 0x94 (ZERO_PAGE_X)
    STA,                  // 0x95 (ZERO_PAGE_X)
    STX,                  // 0x96 (ZERO_PAGE_Y)
    INSTRUCTION_NONE,     // 0x97 (ADDRESS_MODE_NONE)
    TYA,                  // 0x98 (IMPLICIT)
    STA,                  // 0x99 (ABSOLUTE_Y)
    TXS,                  // 0x9A (IMPLICIT)
    INSTRUCTION_NONE,     // 0x9B (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0x9C (ADDRESS_MODE_NONE)
    STA,                  // 0x9D (ABSOLUTE_X)
    INSTRUCTION_NONE,     // 0x9E (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0x9F (ADDRESS_MODE_NONE)
    LDY,                  // 0xA0 (IMMEDIATE)
    LDA,                  // 0xA1 (INDEXED_INDIRECT)
    LDX,                  // 0xA2 (IMMEDIATE)
    INSTRUCTION_NONE,     // 0xA3 (ADDRESS_MODE_NONE)
    LDY,                  // 0xA4 (ZERO_PAGE)
    LDA,                  // 0xA5 (ZERO_PAGE)
    LDX,                  // 0xA6 (ZERO_PAGE)
    INSTRUCTION_NONE,     // 0xA7 (ADDRESS_MODE_NONE)
    TAY,                  // 0xA8 (IMPLICIT)
    LDA,                  // 0xA9 (IMMEDIATE)
    TAX,                  // 0xAA (IMPLICIT)
    INSTRUCTION_NONE,     // 0xAB (ADDRESS_MODE_NONE)
    LDY,                  // 0xAC (ABSOLUTE)
    LDA,                  // 0xAD (ABSOLUTE)
    LDX,                  // 0xAE (ABSOLUTE)
    INSTRUCTION_NONE,     // 0xAF (ADDRESS_MODE_NONE)
    BCS,                  // 0xB0 (RELATIVE)
    LDA,                  // 0xB1 (INDIRECT_INDEXED)
    INSTRUCTION_NONE,     // 0xB2 (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0xB3 (ADDRESS_MODE_NONE)
    LDY,                  // 0xB4 (ZERO_PAGE_X)
    LDA,                  // 0xB5 (ZERO_PAGE_X)
    LDX,                  // 0xB6 (ZERO_PAGE_Y)
    INSTRUCTION_NONE,     // 0xB7 (ADDRESS_MODE_NONE)
    CLV,                  // 0xB8 (IMPLICIT)
    LDA,                  // 0xB9 (ABSOLUTE_Y)
    TSX,                  // 0xBA (IMPLICIT)
    INSTRUCTION_NONE,     // 0xBB (ADDRESS_MODE_NONE)
    LDY,                  // 0xBC (ABSOLUTE_X)
    LDA,                  // 0xBD (ABSOLUTE_X)
    LDX,                  // 0xBE (ABSOLUTE_Y)
    INSTRUCTION_NONE,     // 0xBF (ADDRESS_MODE_NONE)
    CPY,                  // 0xC0 (IMMEDIATE)
    CMP,                  // 0xC1 (INDEXED_INDIRECT)
    INSTRUCTION_NONE,     // 0xC2 (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0xC3 (ADDRESS_MODE_NONE)
    CPY,                  // 0xC4 (ZERO_PAGE)
    CMP,                  // 0xC5 (ZERO_PAGE)
    DEC,                  // 0xC6 (ZERO_PAGE)
    INSTRUCTION_NONE,     // 0xC7 (ADDRESS_MODE_NONE)
    INY,                  // 0xC8 (IMPLICIT)
    CMP,                  // 0xC9 (IMMEDIATE)
    DEX,                  // 0xCA (IMPLICIT)
    INSTRUCTION_NONE,     // 0xCB (ADDRESS_MODE_NONE)
    CPY,                  // 0xCC (ABSOLUTE)
    CMP,                  // 0xCD (ABSOLUTE)
    DEC,                  // 0xCE (ABSOLUTE)
    INSTRUCTION_NONE,     // 0xCF (ADDRESS_MODE_NONE)
    BNE,                  // 0xD0 (RELATIVE)
    CMP,                  // 0xD1 (INDIRECT_INDEXED)
    INSTRUCTION_NONE,     // 0xD2 (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0xD3 (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0xD4 (ADDRESS_MODE_NONE)
    CMP,                  // 0xD5 (ZERO_PAGE_X)
    DEC,                  // 0xD6 (ZERO_PAGE_X)
    INSTRUCTION_NONE,     // 0xD7 (ADDRESS_MODE_NONE)
    CLD,                  // 0xD8 (IMPLICIT)
    CMP,                  // 0xD9 (ABSOLUTE_Y)
    INSTRUCTION_NONE,     // 0xDA (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0xDB (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0xDC (ADDRESS_MODE_NONE)
    CMP,                  // 0xDD (ABSOLUTE_X)
    DEC,                  // 0xDE (ABSOLUTE_X)
    INSTRUCTION_NONE,     // 0xDF (ADDRESS_MODE_NONE)
    CPX,                  // 0xE0 (IMMEDIATE)
    SBC,                  // 0xE1 (INDEXED_INDIRECT)
    INSTRUCTION_NONE,     // 0xE2 (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0xE3 (ADDRESS_MODE_NONE)
    CPX,                  // 0xE4 (ZERO_PAGE)
    SBC,                  // 0xE5 (ZERO_PAGE)
    INC,                  // 0xE6 (ZERO_PAGE)
    INSTRUCTION_NONE,     // 0xE7 (ADDRESS_MODE_NONE)
    INX,                  // 0xE8 (IMPLICIT)
    SBC,                  // 0xE9 (IMMEDIATE)
    NOP,                  // 0xEA (IMPLICIT)
    INSTRUCTION_NONE,     // 0xEB (ADDRESS_MODE_NONE)
    CPX,                  // 0xEC (ABSOLUTE)
    SBC,                  // 0xED (ABSOLUTE)
    INC,                  // 0xEE (ABSOLUTE)
    INSTRUCTION_NONE,     // 0xEF (ADDRESS_MODE_NONE)
    BEQ,                  // 0xF0 (RELATIVE)
    SBC,                  // 0xF1 (INDIRECT_INDEXED)
    INSTRUCTION_NONE,     // 0xF2 (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0xF3 (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0xF4 (ADDRESS_MODE_NONE)
    SBC,                  // 0xF5 (ZERO_PAGE_X)
    INC,                  // 0xF6 (ZERO_PAGE_X)
    INSTRUCTION_NONE,     // 0xF7 (ADDRESS_MODE_NONE)
    SED,                  // 0xF8 (IMPLICIT)
    SBC,                  // 0xF9 (ABSOLUTE_Y)
    INSTRUCTION_NONE,     // 0xFA (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0xFB (ADDRESS_MODE_NONE)
    INSTRUCTION_NONE,     // 0xFC (ADDRESS_MODE_NONE)
    SBC,                  // 0xFD (ABSOLUTE_X)
    INC,                  // 0xFE (ABSOLUTE_X)
    INSTRUCTION_NONE      // 0xFF (ADDRESS_MODE_NONE)
};

const enum address_mode ADDRESS_MODE_LOOKUP[256] = {
    IMPLICIT,              // 0x00 (BRK)
    INDEXED_INDIRECT,      // 0x01 (ORA)
    ADDRESS_MODE_NONE,     // 0x02 (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0x03 (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0x04 (INSTRUCTION_NONE)
    ZERO_PAGE,             // 0x05 (ORA)
    ZERO_PAGE,             // 0x06 (ASL)
    ADDRESS_MODE_NONE,     // 0x07 (INSTRUCTION_NONE)
    IMPLICIT,              // 0x08 (PHP)
    IMMEDIATE,             // 0x09 (ORA)
    ACCUMULATOR,           // 0x0A (ASL)
    ADDRESS_MODE_NONE,     // 0x0B (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0x0C (INSTRUCTION_NONE)
    ABSOLUTE,              // 0x0D (ORA)
    ABSOLUTE,              // 0x0E (ASL)
    ADDRESS_MODE_NONE,     // 0x0F (INSTRUCTION_NONE)
    RELATIVE,              // 0x10 (BPL)
    INDIRECT_INDEXED,      // 0x11 (ORA)
    ADDRESS_MODE_NONE,     // 0x12 (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0x13 (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0x14 (INSTRUCTION_NONE)
    ZERO_PAGE_X,           // 0x15 (ORA)
    ZERO_PAGE_X,           // 0x16 (ASL)
    ADDRESS_MODE_NONE,     // 0x17 (INSTRUCTION_NONE)
    IMPLICIT,              // 0x18 (CLC)
    ABSOLUTE_Y,            // 0x19 (ORA)
    ADDRESS_MODE_NONE,     // 0x1A (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0x1B (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0x1C (INSTRUCTION_NONE)
    ABSOLUTE_X,            // 0x1D (ORA)
    ABSOLUTE_X,            // 0x1E (ASL)
    ADDRESS_MODE_NONE,     // 0x1F (INSTRUCTION_NONE)
    ABSOLUTE,              // 0x20 (JSR)
    INDEXED_INDIRECT,      // 0x21 (AND)
    ADDRESS_MODE_NONE,     // 0x22 (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0x23 (INSTRUCTION_NONE)
    ZERO_PAGE,             // 0x24 (BIT)
    ZERO_PAGE,             // 0x25 (AND)
    ZERO_PAGE,             // 0x26 (ROL)
    ADDRESS_MODE_NONE,     // 0x27 (INSTRUCTION_NONE)
    IMPLICIT,              // 0x28 (PLP)
    IMMEDIATE,             // 0x29 (AND)
    ACCUMULATOR,           // 0x2A (ROL)
    ADDRESS_MODE_NONE,     // 0x2B (INSTRUCTION_NONE)
    ABSOLUTE,              // 0x2C (BIT)
    ABSOLUTE,              // 0x2D (AND)
    ABSOLUTE,              // 0x2E (ROL)
    ADDRESS_MODE_NONE,     // 0x2F (INSTRUCTION_NONE)
    RELATIVE,              // 0x30 (BMI)
    INDIRECT_INDEXED,      // 0x31 (AND)
    ADDRESS_MODE_NONE,     // 0x32 (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0x33 (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0x34 (INSTRUCTION_NONE)
    ZERO_PAGE_X,           // 0x35 (AND)
    ZERO_PAGE_X,           // 0x36 (ROL)
    ADDRESS_MODE_NONE,     // 0x37 (INSTRUCTION_NONE)
    IMPLICIT,              // 0x38 (SEC)
    ABSOLUTE_Y,            // 0x39 (AND)
    ADDRESS_MODE_NONE,     // 0x3A (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0x3B (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0x3C (INSTRUCTION_NONE)
    ABSOLUTE_X,            // 0x3D (AND)
    ABSOLUTE_X,            // 0x3E (ROL)
    ADDRESS_MODE_NONE,     // 0x3F (INSTRUCTION_NONE)
    IMPLICIT,              // 0x40 (RTI)
    INDEXED_INDIRECT,      // 0x41 (EOR)
    ADDRESS_MODE_NONE,     // 0x42 (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0x43 (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0x44 (INSTRUCTION_NONE)
    ZERO_PAGE,             // 0x45 (EOR)
    ZERO_PAGE,             // 0x46 (LSR)
    ADDRESS_MODE_NONE,     // 0x47 (INSTRUCTION_NONE)
    IMPLICIT,              // 0x48 (PHA)
    IMMEDIATE,             // 0x49 (EOR)
    ACCUMULATOR,           // 0x4A (LSR)
    ADDRESS_MODE_NONE,     // 0x4B (INSTRUCTION_NONE)
    ABSOLUTE,              // 0x4C (JMP)
    ABSOLUTE,              // 0x4D (EOR)
    ABSOLUTE,              // 0x4E (LSR)
    ADDRESS_MODE_NONE,     // 0x4F (INSTRUCTION_NONE)
    RELATIVE,              // 0x50 (BVC)
    INDIRECT_INDEXED,      // 0x51 (EOR)
    ADDRESS_MODE_NONE,     // 0x52 (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0x53 (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0x54 (INSTRUCTION_NONE)
    ZERO_PAGE_X,           // 0x55 (EOR)
    ZERO_PAGE_X,           // 0x56 (LSR)
    ADDRESS_MODE_NONE,     // 0x57 (INSTRUCTION_NONE)
    IMPLICIT,              // 0x58 (CLI)
    ABSOLUTE_Y,            // 0x59 (EOR)
    ADDRESS_MODE_NONE,     // 0x5A (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0x5B (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0x5C (INSTRUCTION_NONE)
    ABSOLUTE_X,            // 0x5D (EOR)
    ABSOLUTE_X,            // 0x5E (LSR)
    ADDRESS_MODE_NONE,     // 0x5F (INSTRUCTION_NONE)
    IMPLICIT,              // 0x60 (RTS)
    INDEXED_INDIRECT,      // 0x61 (ADC)
    ADDRESS_MODE_NONE,     // 0x62 (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0x63 (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0x64 (INSTRUCTION_NONE)
    ZERO_PAGE,             // 0x65 (ADC)
    ZERO_PAGE,             // 0x66 (ROR)
    ADDRESS_MODE_NONE,     // 0x67 (INSTRUCTION_NONE)
    IMPLICIT,              // 0x68 (PLA)
    IMMEDIATE,             // 0x69 (ADC)
    ACCUMULATOR,           // 0x6A (ROR)
    ADDRESS_MODE_NONE,     // 0x6B (INSTRUCTION_NONE)
    INDIRECT,              // 0x6C (JMP)
    ABSOLUTE,              // 0x6D (ADC)
    ABSOLUTE,              // 0x6E (ROR)
    ADDRESS_MODE_NONE,     // 0x6F (INSTRUCTION_NONE)
    RELATIVE,              // 0x70 (BVS)
    INDIRECT_INDEXED,      // 0x71 (ADC)
    ADDRESS_MODE_NONE,     // 0x72 (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0x73 (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0x74 (INSTRUCTION_NONE)
    ZERO_PAGE_X,           // 0x75 (ADC)
    ZERO_PAGE_X,           // 0x76 (ROR)
    ADDRESS_MODE_NONE,     // 0x77 (INSTRUCTION_NONE)
    IMPLICIT,              // 0x78 (SEI)
    ABSOLUTE_Y,            // 0x79 (ADC)
    ADDRESS_MODE_NONE,     // 0x7A (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0x7B (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0x7C (INSTRUCTION_NONE)
    ABSOLUTE_X,            // 0x7D (ADC)
    ABSOLUTE_X,            // 0x7E (ROR)
    ADDRESS_MODE_NONE,     // 0x7F (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0x80 (INSTRUCTION_NONE)
    INDEXED_INDIRECT,      // 0x81 (STA)
    ADDRESS_MODE_NONE,     // 0x82 (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0x83 (INSTRUCTION_NONE)
    ZERO_PAGE,             // 0x84 (STY)
    ZERO_PAGE,             // 0x85 (STA)
    ZERO_PAGE,             // 0x86 (STX)
    ADDRESS_MODE_NONE,     // 0x87 (INSTRUCTION_NONE)
    IMPLICIT,              // 0x88 (DEY)
    ADDRESS_MODE_NONE,     // 0x89 (INSTRUCTION_NONE)
    IMPLICIT,              // 0x8A (TXA)
    ADDRESS_MODE_NONE,     // 0x8B (INSTRUCTION_NONE)
    ABSOLUTE,              // 0x8C (STY)
    ABSOLUTE,              // 0x8D (STA)
    ABSOLUTE,              // 0x8E (STX)
    ADDRESS_MODE_NONE,     // 0x8F (INSTRUCTION_NONE)
    RELATIVE,              // 0x90 (BCC)
    INDIRECT_INDEXED,      // 0x91 (STA)
    ADDRESS_MODE_NONE,     // 0x92 (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0x93 (INSTRUCTION_NONE)
    ZERO_PAGE_X,           // 0x94 (STY)
    ZERO_PAGE_X,           // 0x95 (STA)
    ZERO_PAGE_Y,           // 0x96 (STX)
    ADDRESS_MODE_NONE,     // 0x97 (INSTRUCTION_NONE)
    IMPLICIT,              // 0x98 (TYA)
    ABSOLUTE_Y,            // 0x99 (STA)
    IMPLICIT,              // 0x9A (TXS)
    ADDRESS_MODE_NONE,     // 0x9B (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0x9C (INSTRUCTION_NONE)
    ABSOLUTE_X,            // 0x9D (STA)
    ADDRESS_MODE_NONE,     // 0x9E (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0x9F (INSTRUCTION_NONE)
    IMMEDIATE,             // 0xA0 (LDY)
    INDEXED_INDIRECT,      // 0xA1 (LDA)
    IMMEDIATE,             // 0xA2 (LDX)
    ADDRESS_MODE_NONE,     // 0xA3 (INSTRUCTION_NONE)
    ZERO_PAGE,             // 0xA4 (LDY)
    ZERO_PAGE,             // 0xA5 (LDA)
    ZERO_PAGE,             // 0xA6 (LDX)
    ADDRESS_MODE_NONE,     // 0xA7 (INSTRUCTION_NONE)
    IMPLICIT,              // 0xA8 (TAY)
    IMMEDIATE,             // 0xA9 (LDA)
    IMPLICIT,              // 0xAA (TAX)
    ADDRESS_MODE_NONE,     // 0xAB (INSTRUCTION_NONE)
    ABSOLUTE,              // 0xAC (LDY)
    ABSOLUTE,              // 0xAD (LDA)
    ABSOLUTE,              // 0xAE (LDX)
    ADDRESS_MODE_NONE,     // 0xAF (INSTRUCTION_NONE)
    RELATIVE,              // 0xB0 (BCS)
    INDIRECT_INDEXED,      // 0xB1 (LDA)
    ADDRESS_MODE_NONE,     // 0xB2 (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0xB3 (INSTRUCTION_NONE)
    ZERO_PAGE_X,           // 0xB4 (LDY)
    ZERO_PAGE_X,           // 0xB5 (LDA)
    ZERO_PAGE_Y,           // 0xB6 (LDX)
    ADDRESS_MODE_NONE,     // 0xB7 (INSTRUCTION_NONE)
    IMPLICIT,              // 0xB8 (CLV)
    ABSOLUTE_Y,            // 0xB9 (LDA)
    IMPLICIT,              // 0xBA (TSX)
    ADDRESS_MODE_NONE,     // 0xBB (INSTRUCTION_NONE)
    ABSOLUTE_X,            // 0xBC (LDY)
    ABSOLUTE_X,            // 0xBD (LDA)
    ABSOLUTE_Y,            // 0xBE (LDX)
    ADDRESS_MODE_NONE,     // 0xBF (INSTRUCTION_NONE)
    IMMEDIATE,             // 0xC0 (CPY)
    INDEXED_INDIRECT,      // 0xC1 (CMP)
    ADDRESS_MODE_NONE,     // 0xC2 (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0xC3 (INSTRUCTION_NONE)
    ZERO_PAGE,             // 0xC4 (CPY)
    ZERO_PAGE,             // 0xC5 (CMP)
    ZERO_PAGE,             // 0xC6 (DEC)
    ADDRESS_MODE_NONE,     // 0xC7 (INSTRUCTION_NONE)
    IMPLICIT,              // 0xC8 (INY)
    IMMEDIATE,             // 0xC9 (CMP)
    IMPLICIT,              // 0xCA (DEX)
    ADDRESS_MODE_NONE,     // 0xCB (INSTRUCTION_NONE)
    ABSOLUTE,              // 0xCC (CPY)
    ABSOLUTE,              // 0xCD (CMP)
    ABSOLUTE,              // 0xCE (DEC)
    ADDRESS_MODE_NONE,     // 0xCF (INSTRUCTION_NONE)
    RELATIVE,              // 0xD0 (BNE)
    INDIRECT_INDEXED,      // 0xD1 (CMP)
    ADDRESS_MODE_NONE,     // 0xD2 (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0xD3 (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0xD4 (INSTRUCTION_NONE)
    ZERO_PAGE_X,           // 0xD5 (CMP)
    ZERO_PAGE_X,           // 0xD6 (DEC)
    ADDRESS_MODE_NONE,     // 0xD7 (INSTRUCTION_NONE)
    IMPLICIT,              // 0xD8 (CLD)
    ABSOLUTE_Y,            // 0xD9 (CMP)
    ADDRESS_MODE_NONE,     // 0xDA (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0xDB (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0xDC (INSTRUCTION_NONE)
    ABSOLUTE_X,            // 0xDD (CMP)
    ABSOLUTE_X,            // 0xDE (DEC)
    ADDRESS_MODE_NONE,     // 0xDF (INSTRUCTION_NONE)
    IMMEDIATE,             // 0xE0 (CPX)
    INDEXED_INDIRECT,      // 0xE1 (SBC)
    ADDRESS_MODE_NONE,     // 0xE2 (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0xE3 (INSTRUCTION_NONE)
    ZERO_PAGE,             // 0xE4 (CPX)
    ZERO_PAGE,             // 0xE5 (SBC)
    ZERO_PAGE,             // 0xE6 (INC)
    ADDRESS_MODE_NONE,     // 0xE7 (INSTRUCTION_NONE)
    IMPLICIT,              // 0xE8 (INX)
    IMMEDIATE,             // 0xE9 (SBC)
    IMPLICIT,              // 0xEA (NOP)
    ADDRESS_MODE_NONE,     // 0xEB (INSTRUCTION_NONE)
    ABSOLUTE,              // 0xEC (CPX)
    ABSOLUTE,              // 0xED (SBC)
    ABSOLUTE,              // 0xEE (INC)
    ADDRESS_MODE_NONE,     // 0xEF (INSTRUCTION_NONE)
    RELATIVE,              // 0xF0 (BEQ)
    INDIRECT_INDEXED,      // 0xF1 (SBC)
    ADDRESS_MODE_NONE,     // 0xF2 (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0xF3 (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0xF4 (INSTRUCTION_NONE)
    ZERO_PAGE_X,           // 0xF5 (SBC)
    ZERO_PAGE_X,           // 0xF6 (INC)
    ADDRESS_MODE_NONE,     // 0xF7 (INSTRUCTION_NONE)
    IMPLICIT,              // 0xF8 (SED)
    ABSOLUTE_Y,            // 0xF9 (SBC)
    ADDRESS_MODE_NONE,     // 0xFA (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0xFB (INSTRUCTION_NONE)
    ADDRESS_MODE_NONE,     // 0xFC (INSTRUCTION_NONE)
    ABSOLUTE_X,            // 0xFD (SBC)
    ABSOLUTE_X,            // 0xFE (INC)
    ADDRESS_MODE_NONE,     // 0xFF (INSTRUCTION_NONE)
};

const uint8_t INSTRUCTION_CYCLES[] = {
    [ADC] = 2,
    [AND] = 2,
    [ASL] = 2,
    [BCC] = 0,
    [BCS] = 0,
    [BEQ] = 0,
    [BIT] = 2,
    [BMI] = 0,
    [BNE] = 0,
    [BPL] = 0,
    [BRK] = 7,
    [BVC] = 0,
    [BVS] = 0,
    [CLC] = 2,
    [CLD] = 2,
    [CLI] = 2,
    [CLV] = 2,
    [CMP] = 2,
    [CPX] = 2,
    [CPY] = 2,
    [DEC] = 4,
    [DEX] = 2,
    [DEY] = 2,
    [EOR] = 2,
    [INC] = 4,
    [INX] = 2,
    [INY] = 2,
    [JMP] = 1,
    [JSR] = 4,
    [LDA] = 2,
    [LDX] = 2,
    [LDY] = 2,
    [LSR] = 2,
    [NOP] = 2,
    [ORA] = 2,
    [PHA] = 3,
    [PHP] = 3,
    [PLA] = 4,
    [PLP] = 4,
    [ROL] = 2,
    [ROR] = 2,
    [RTI] = 6,
    [RTS] = 6,
    [SBC] = 2,
    [SEC] = 2,
    [SED] = 2,
    [SEI] = 2,
    [STA] = 2,
    [STX] = 2,
    [STY] = 2,
    [TAX] = 2,
    [TAY] = 2,
    [TSX] = 2,
    [TXA] = 2,
    [TXS] = 2,
    [TYA] = 2,
};

const uint8_t ADDRESS_MODE_CYCLES[] = {
    [IMPLICIT]         = 0,
    [IMMEDIATE]        = 0,
    [ACCUMULATOR]      = 0,
    [RELATIVE]         = 2,
    [ZERO_PAGE]        = 1,
    [ABSOLUTE]         = 2,
    [INDIRECT]         = 4,
    [ZERO_PAGE_X]      = 2,
    [ZERO_PAGE_Y]      = 2,
    [ABSOLUTE_X]       = 2,
    [ABSOLUTE_Y]       = 2,
    [INDEXED_INDIRECT] = 4,
    [INDIRECT_INDEXED] = 3,
};

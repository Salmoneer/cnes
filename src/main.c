#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "instructions.h"

const uint16_t NMI_VECTOR = 0xfffa;
const uint16_t RESET_VECTOR = 0xfffc;
const uint16_t IRQ_VECTOR = 0xfffe;

enum flag {
    CARRY     = 0,
    ZERO      = 1,
    INTERRUPT = 2,
    DECIMAL   = 3,
    BREAK     = 4,
    ONE       = 5,
    OVERFLOW  = 6,
    NEGATIVE  = 7,
};

struct {
    struct {
        char nes[4];
        uint8_t prg_size;
        uint8_t chr_size;
        uint8_t flags_6;
        uint8_t flags_7;
        char padding[8];
    } header;

    uint8_t *prg_rom;
    uint8_t *chr_rom;

    uint8_t mapper;
} cartridge = { 0 };

struct {
    struct {
        uint16_t pc;
        uint8_t a, x, y, s, p;
    } cpu;

    uint8_t *ram;
} nes = { 0 };


uint8_t *read_file(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (f == NULL) return NULL;

    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    rewind(f);

    uint8_t *buffer = malloc(length);
    fread(buffer, length, 1, f);

    return buffer;
}

void load_cartridge(uint8_t *data) {
    if ((data[7] & 0x0c) == 0x08) {
        printf("iNES 2.0 is unsupported.\n");
        exit(EXIT_FAILURE);
    }

    memcpy(&cartridge.header, data, 16);

    bool trainer = cartridge.header.flags_6 & (1 << 3);

    cartridge.prg_rom = data + 16 + (trainer ? 512 : 0);
    cartridge.chr_rom = cartridge.prg_rom + cartridge.header.prg_size;

    cartridge.mapper = (cartridge.header.flags_7 & 0xf0) | (cartridge.header.flags_6 >> 4);
}

void print_header() {
    printf("Magic: ");
    for (int i = 0; i < 3; i++) {
        printf("%c", cartridge.header.nes[i]);
    }
    printf(" 0x%02x\n", cartridge.header.nes[3]);

    printf("PRG ROM size: 16KiB * %d\n", cartridge.header.prg_size);
    printf("CHR ROM size:  8KiB * %d\n", cartridge.header.chr_size);

    printf("Flags (6): 0x%02x\n", cartridge.header.flags_6);
    printf("Flags (7): 0x%02x\n", cartridge.header.flags_7);

    printf("Padding:");
    for (int i = 0; i < 8; i++) {
        printf(" 0x%02x", cartridge.header.padding[i]);
    }

    printf("\n");
}


uint8_t cpu_read_8(uint16_t address) {
    if (address < 0x2000) {
        return nes.ram[address & 0x07ff];
    } else if (address >= 0x8000) {
        return cartridge.prg_rom[address - 0x8000];
    }

    printf("WARNING: Read from unmapped address: %04x\n", address);
    return 0;
}

uint16_t cpu_read_16(uint16_t address) {
    return (cpu_read_8(address + 1) << 8) | cpu_read_8(address);
}

void cpu_write_8(uint16_t address, uint8_t data) {
    if (address < 0x2000) {
        nes.ram[address & 0x07ff] = data;
    } else if (address >= 0x8000) {
        cartridge.prg_rom[address - 0x8000] = data;
    } else {
        printf("WARNING: Write to unmapped address: %04x with data: %02x\n", address, data);
    }
}

void cpu_write_16(uint16_t address, uint16_t data) {
    cpu_write_8(address, data >> 8);
    cpu_write_8(address + 1, data & 0xff);
}

void set_flag(enum flag f, bool set) {
    if (set) {
        nes.cpu.p |= 1 << f;
    } else {
        nes.cpu.p &= ~(1 << f);
    }
}

uint8_t get_flag(enum flag f) {
    if (nes.cpu.p & f) {
        return 1;
    } else {
        return 0;
    }
}

void stack_push_8(uint8_t data) {
    nes.ram[0x100 + nes.cpu.s--] = data;
}

void stack_push_16(uint16_t data) {
    stack_push_8(data >> 8);
    stack_push_8(data & 0xff);
}

uint8_t stack_pop_8() {
    return nes.ram[0x100 + ++nes.cpu.s];
}

uint16_t stack_pop_16() {
    return stack_pop_8() | (stack_pop_8() << 8);
}


uint16_t instruction_length(enum address_mode mode) {
    switch (mode) {
        case IMPLICIT:
        case ACCUMULATOR:
            return 1;
        case IMMEDIATE:
        case RELATIVE:
        case ZERO_PAGE:
        case ZERO_PAGE_X:
        case ZERO_PAGE_Y:
        case INDEXED_INDIRECT:
        case INDIRECT_INDEXED:
            return 2;
        case INDIRECT:
        case ABSOLUTE:
        case ABSOLUTE_X:
        case ABSOLUTE_Y:
            return 3;
        default:
            printf("ERROR: Unable to find length of instruction with unknown addressing mode with id: %d\nHalting execution\n", mode);
            exit(EXIT_FAILURE);
    }
}

uint16_t read_operand(enum address_mode mode) {
    uint8_t operand_8 = cpu_read_8(nes.cpu.pc + 1);
    uint16_t operand_16 = cpu_read_16(nes.cpu.pc + 1);

    switch (mode) {
        case IMPLICIT:
        case ACCUMULATOR:
            return 0;
        case IMMEDIATE:
        case RELATIVE:
            return nes.cpu.pc + 1;
        case ZERO_PAGE:
            return operand_8;
        case ABSOLUTE:
            return operand_16;
        case INDIRECT:
            return cpu_read_16(operand_16);
        case ZERO_PAGE_X:
            return (operand_8 + nes.cpu.x) % 256;
        case ZERO_PAGE_Y:
            return (operand_8 + nes.cpu.y) % 256;
        case ABSOLUTE_X:
            return operand_16 + nes.cpu.x;
        case ABSOLUTE_Y:
            return operand_16 + nes.cpu.y;
        case INDEXED_INDIRECT:
            return cpu_read_16((operand_8 + nes.cpu.x) % 256) + 256 * cpu_read_16((operand_8 + nes.cpu.x + 1) % 256);
        case INDIRECT_INDEXED:
            return cpu_read_8(operand_8) + 256 * cpu_read_8((operand_8 + 1) % 256) + nes.cpu.y;
        default:
            printf("ERROR: Unknown addressing mode with id: %d\nHalting execution\n", mode);
            exit(EXIT_FAILURE);
    }
}


void _adc(enum address_mode mode, uint16_t address) {
    uint8_t data = cpu_read_8(address);

    int result = nes.cpu.a + data + get_flag(CARRY);

    set_flag(CARRY, result > 0xff);
    set_flag(ZERO, (result & 0xff) == 0);
    set_flag(OVERFLOW, ~(nes.cpu.a ^ data) & (nes.cpu.a ^ result) & 0x80);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.a = result;
}

void _and(enum address_mode mode, uint16_t address) {
    uint8_t result = nes.cpu.a & cpu_read_8(address);

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.a = result;
}

void _asl(enum address_mode mode, uint16_t address) {
    int result;
    if (mode == ACCUMULATOR) {
        result = nes.cpu.a << 1;
    } else {
        result = cpu_read_8(address) << 1;
    }

    set_flag(CARRY, result & 0x100);
    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    if (mode == ACCUMULATOR) {
        nes.cpu.a = result;
    } else {
        cpu_write_8(address, result);
    }
}

void _bcc(enum address_mode mode, uint16_t address) {
    if (!get_flag(CARRY)) {
        nes.cpu.pc += cpu_read_8(address);
    }
}

void _bcs(enum address_mode mode, uint16_t address) {
    if (get_flag(CARRY)) {
        nes.cpu.pc += cpu_read_8(address);
    }
}

void _beq(enum address_mode mode, uint16_t address) {
    if (get_flag(ZERO)) {
        nes.cpu.pc += cpu_read_8(address);
    }
}

void _bit(enum address_mode mode, uint16_t address) {
    uint8_t result = nes.cpu.a & cpu_read_8(address);

    set_flag(ZERO, result == 0);
    set_flag(OVERFLOW, result & 0x40);
    set_flag(NEGATIVE, result & 0x80);
}

void _bmi(enum address_mode mode, uint16_t address) {
    if (get_flag(NEGATIVE)) {
        nes.cpu.pc += cpu_read_8(address);
    }
}

void _bne(enum address_mode mode, uint16_t address) {
    if (!get_flag(ZERO)) {
        nes.cpu.pc += cpu_read_8(address);
    }
}

void _bpl(enum address_mode mode, uint16_t address) {
    if (!get_flag(NEGATIVE)) {
        nes.cpu.pc += cpu_read_8(address);
    }
}

void _brk(enum address_mode mode, uint16_t address) {
    stack_push_16(nes.cpu.pc);
    stack_push_8(nes.cpu.p);
    nes.cpu.pc = cpu_read_16(IRQ_VECTOR);
    set_flag(BREAK, true);
}

void _bvc(enum address_mode mode, uint16_t address) {
    if (!get_flag(OVERFLOW)) {
        nes.cpu.pc += cpu_read_8(address);
    }
}

void _bvs(enum address_mode mode, uint16_t address) {
    if (get_flag(OVERFLOW)) {
        nes.cpu.pc += cpu_read_8(address);
    }
}

void _clc(enum address_mode mode, uint16_t address) {
    set_flag(CARRY, false);
}

void _cld(enum address_mode mode, uint16_t address) {
    set_flag(DECIMAL, false);
}

void _cli(enum address_mode mode, uint16_t address) {
    set_flag(INTERRUPT, false);
}

void _clv(enum address_mode mode, uint16_t address) {
    set_flag(OVERFLOW, false);
}

void _cmp(enum address_mode mode, uint16_t address) {
    uint8_t result = nes.cpu.a - cpu_read_8(address);

    set_flag(CARRY, result >= 0);
    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);
}

void _cpx(enum address_mode mode, uint16_t address) {
    uint8_t result = nes.cpu.x - cpu_read_8(address);

    set_flag(CARRY, result >= 0);
    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);
}

void _cpy(enum address_mode mode, uint16_t address) {
    uint8_t result = nes.cpu.y - cpu_read_8(address);

    set_flag(CARRY, result >= 0);
    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);
}

void _dec(enum address_mode mode, uint16_t address) {
    uint8_t result = cpu_read_8(address) - 1;

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    cpu_write_8(address, result);
}

void _dex(enum address_mode mode, uint16_t address) {
    uint8_t result = nes.cpu.x - 1;

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.x = result;
}

void _dey(enum address_mode mode, uint16_t address) {
    uint8_t result = nes.cpu.y - 1;

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.y = result;
}

void _eor(enum address_mode mode, uint16_t address) {
    uint8_t result = nes.cpu.a ^ cpu_read_8(address);

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.a = result;
}

void _inc(enum address_mode mode, uint16_t address) {
    uint8_t result = cpu_read_8(address) + 1;

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    cpu_write_8(address, result);
}

void _inx(enum address_mode mode, uint16_t address) {
    uint8_t result = nes.cpu.x + 1;

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.x = result;
}

void _iny(enum address_mode mode, uint16_t address) {
    uint8_t result = nes.cpu.y + 1;

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.x = result;
}

void _jmp(enum address_mode mode, uint16_t address) {
    // TODO: Indirect mode page boundary bug
    nes.cpu.pc = address;
}

void _jsr(enum address_mode mode, uint16_t address) {
    stack_push_16(instruction_length(mode) - 1);
    nes.cpu.pc = address;
}

void _lda(enum address_mode mode, uint16_t address) {
    uint8_t data = cpu_read_8(address);

    set_flag(ZERO, data == 0);
    set_flag(NEGATIVE, data & 0x80);

    nes.cpu.a = data;
}

void _ldx(enum address_mode mode, uint16_t address) {
    uint8_t data = cpu_read_8(address);

    set_flag(ZERO, data == 0);
    set_flag(NEGATIVE, data & 0x80);

    nes.cpu.x = data;
}

void _ldy(enum address_mode mode, uint16_t address) {
    uint8_t data = cpu_read_8(address);

    set_flag(ZERO, data == 0);
    set_flag(NEGATIVE, data & 0x80);

    nes.cpu.y = data;
}

void _lsr(enum address_mode mode, uint16_t address) {
    uint8_t data;
    if (mode == ACCUMULATOR) {
        data = nes.cpu.a;
    } else {
        data = cpu_read_8(address);
    }

    uint8_t result = data >> 1;

    set_flag(CARRY, data & 1);
    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    if (mode == ACCUMULATOR) {
        nes.cpu.a = result;
    } else {
        cpu_write_8(address, result);
    }
}

void _nop(enum address_mode mode, uint16_t address) {}

void _ora(enum address_mode mode, uint16_t address) {
    uint8_t result = nes.cpu.a | cpu_read_8(address);

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.a = result;
}

void _pha(enum address_mode mode, uint16_t address) {
    stack_push_8(nes.cpu.a);
}

void _php(enum address_mode mode, uint16_t address) {
    stack_push_8(nes.cpu.p);
}

void _pla(enum address_mode mode, uint16_t address) {
    uint8_t result = stack_pop_8();

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.a = result;
}

void _plp(enum address_mode mode, uint16_t address) {
    uint8_t result = stack_pop_8();

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.p = result;
}

void _rol(enum address_mode mode, uint16_t address) {
    uint8_t data;
    if (mode == ACCUMULATOR) {
        data = nes.cpu.a;
    } else {
        data = cpu_read_8(address);
    }

    uint8_t result = data << 1 | get_flag(CARRY);

    set_flag(CARRY, data & 0x80);
    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    if (mode == ACCUMULATOR) {
        nes.cpu.a = result;
    } else {
        cpu_write_8(address, result);
    }
}

void _ror(enum address_mode mode, uint16_t address) {
    uint8_t data;
    if (mode == ACCUMULATOR) {
        data = nes.cpu.a;
    } else {
        data = cpu_read_8(address);
    }

    uint8_t result = data >> 1 | get_flag(CARRY);

    set_flag(CARRY, data & 1);
    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    if (mode == ACCUMULATOR) {
        nes.cpu.a = result;
    } else {
        cpu_write_8(address, result);
    }
}

void _rti(enum address_mode mode, uint16_t address) {
    nes.cpu.p = stack_pop_8();
    nes.cpu.pc = stack_pop_16();
}

void _rts(enum address_mode mode, uint16_t address) {
    nes.cpu.pc = stack_pop_16() + 1;
}

void _sbc(enum address_mode mode, uint16_t address) {
    uint8_t data = ~cpu_read_8(address);

    int result = nes.cpu.a + data + get_flag(CARRY);

    set_flag(CARRY, result > 0xff);
    set_flag(ZERO, (result & 0xff) == 0);
    set_flag(OVERFLOW, ~(nes.cpu.a ^ data) & (nes.cpu.a ^ result) & 0x80);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.a = result;
}

void _sec(enum address_mode mode, uint16_t address) {
    set_flag(CARRY, 1);
}

void _sed(enum address_mode mode, uint16_t address) {
    set_flag(DECIMAL, 1);
}

void _sei(enum address_mode mode, uint16_t address) {
    set_flag(INTERRUPT, 1);
}

void _sta(enum address_mode mode, uint16_t address) {
    cpu_write_8(address, nes.cpu.a);
}

void _stx(enum address_mode mode, uint16_t address) {
    cpu_write_8(address, nes.cpu.x);
}

void _sty(enum address_mode mode, uint16_t address) {
    cpu_write_8(address, nes.cpu.y);
}

void _tax(enum address_mode mode, uint16_t address) {
    uint8_t result = nes.cpu.a;

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.x = result;
}

void _tay(enum address_mode mode, uint16_t address) {
    uint8_t result = nes.cpu.a;

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.y = result;
}

void _tsx(enum address_mode mode, uint16_t address) {
    uint8_t result = nes.cpu.s;

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.x = result;
}

void _txa(enum address_mode mode, uint16_t address) {
    uint8_t result = nes.cpu.x;

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.a = result;
}

void _txs(enum address_mode mode, uint16_t address) {
    uint8_t result = nes.cpu.x;

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.s = result;
}

void _tya(enum address_mode mode, uint16_t address) {
    uint8_t result = nes.cpu.y;

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.a = result;
}


int execute_next() {
    uint16_t initial_pc = nes.cpu.pc;

    uint8_t opcode = cpu_read_8(nes.cpu.pc);

    enum instruction_name name = INSTRUCTION_LOOKUP[opcode];
    enum address_mode mode = ADDRESS_MODE_LOOKUP[opcode];

    uint16_t address = read_operand(mode);

    printf("%04x: %s mode %s (", nes.cpu.pc, INSTRUCTION_NAME_STRING[name], ADDRESS_MODE_STRING[mode]);
    for (int i = 0; i < instruction_length(mode); i++) {
        if (i != 0) printf(" ");
        printf("%02x", cpu_read_8(nes.cpu.pc + i));
    }
    printf(")\n");

    switch (name) {
        case ADC: _adc(mode, address); break;
        case AND: _and(mode, address); break;
        case ASL: _asl(mode, address); break;
        case BCC: _bcc(mode, address); break;
        case BCS: _bcs(mode, address); break;
        case BEQ: _beq(mode, address); break;
        case BIT: _bit(mode, address); break;
        case BMI: _bmi(mode, address); break;
        case BNE: _bne(mode, address); break;
        case BPL: _bpl(mode, address); break;
        case BRK: _brk(mode, address); break;
        case BVC: _bvc(mode, address); break;
        case BVS: _bvs(mode, address); break;
        case CLC: _clc(mode, address); break;
        case CLD: _cld(mode, address); break;
        case CLI: _cli(mode, address); break;
        case CLV: _clv(mode, address); break;
        case CMP: _cmp(mode, address); break;
        case CPX: _cpx(mode, address); break;
        case CPY: _cpy(mode, address); break;
        case DEC: _dec(mode, address); break;
        case DEX: _dex(mode, address); break;
        case DEY: _dey(mode, address); break;
        case EOR: _eor(mode, address); break;
        case INC: _inc(mode, address); break;
        case INX: _inx(mode, address); break;
        case INY: _iny(mode, address); break;
        case JMP: _jmp(mode, address); break;
        case JSR: _jsr(mode, address); break;
        case LDA: _lda(mode, address); break;
        case LDX: _ldx(mode, address); break;
        case LDY: _ldy(mode, address); break;
        case LSR: _lsr(mode, address); break;
        case NOP: _nop(mode, address); break;
        case ORA: _ora(mode, address); break;
        case PHA: _pha(mode, address); break;
        case PHP: _php(mode, address); break;
        case PLA: _pla(mode, address); break;
        case PLP: _plp(mode, address); break;
        case ROL: _rol(mode, address); break;
        case ROR: _ror(mode, address); break;
        case RTI: _rti(mode, address); break;
        case RTS: _rts(mode, address); break;
        case SBC: _sbc(mode, address); break;
        case SEC: _sec(mode, address); break;
        case SED: _sed(mode, address); break;
        case SEI: _sei(mode, address); break;
        case STA: _sta(mode, address); break;
        case STX: _stx(mode, address); break;
        case STY: _sty(mode, address); break;
        case TAX: _tax(mode, address); break;
        case TAY: _tay(mode, address); break;
        case TSX: _tsx(mode, address); break;
        case TXA: _txa(mode, address); break;
        case TXS: _txs(mode, address); break;
        case TYA: _tya(mode, address); break;
        default:
            printf("ERROR: Unknown instruction with opcode: 0x%02x\nHalting execution\n", opcode);
            break;
    }

    // Prevent jmp and branch instructions from skipping the next instruction
    if (initial_pc == nes.cpu.pc) {
        nes.cpu.pc += instruction_length(mode);
    }

    return 0;
}

void poweron() {
    nes.cpu.pc = cpu_read_16(RESET_VECTOR);
    printf("----- POWER ON -----\n");
    printf("Starting execution at 0x%04x\n", nes.cpu.pc);
}

void run() {
    int cycles = 0;
    for (int i = 0; i < 1000; i++) {
        if (cycles == 0) {
            cycles += execute_next();
        } else {
            cycles--;
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Please provide a file\n");
        exit(EXIT_FAILURE);
    }

    uint8_t *data = read_file(argv[1]);

    if (data == NULL) {
        printf("Please provide a valid file\n");
        exit(EXIT_FAILURE);
    }

    load_cartridge(data);

    uint8_t ram[2048] = { 0 };

    nes.ram = ram;

    poweron();
    run();

    free(data);
}

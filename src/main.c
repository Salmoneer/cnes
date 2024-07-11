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


bool DEBUG = false;


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


struct {
    uint8_t *data;

    uint64_t cycles;
    uint64_t cycles_queue;
} state = { 0 };

void cleanup() {
    if (state.data != NULL) free(state.data);
}


#define eprintf(format, ...) fprintf(stderr, format, ##__VA_ARGS__)


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

void load_cartridge() {
    if ((state.data[7] & 0x0c) == 0x08) {
        printf("iNES 2.0 is not supported.\n");
        exit(EXIT_FAILURE);
    }

    memcpy(cartridge.header.nes, state.data, 4);
    cartridge.header.prg_size = state.data[4];
    cartridge.header.chr_size = state.data[5];
    cartridge.header.flags_6 = state.data[6];
    cartridge.header.flags_7 = state.data[7];
    memcpy(cartridge.header.padding, state.data + 8, 8);

    bool trainer = cartridge.header.flags_6 & (1 << 3);

    cartridge.prg_rom = state.data + 16 + (trainer ? 512 : 0);
    cartridge.chr_rom = cartridge.prg_rom + cartridge.header.prg_size;

    cartridge.mapper = (cartridge.header.flags_7 & 0xf0) | (cartridge.header.flags_6 >> 4);
}

void print_header() {
    if (DEBUG) return;

    printf("Magic: ");
    for (int i = 0; i < 3; i++) {
        printf("%c", cartridge.header.nes[i]);
    }
    printf(" 0x%02X\n", cartridge.header.nes[3]);

    printf("PRG ROM size: %d * 16KiB\n", cartridge.header.prg_size);
    printf("CHR ROM size: %d *  8KiB\n", cartridge.header.chr_size);

    printf("Flags (6): 0x%02X\n", cartridge.header.flags_6);
    printf("Flags (7): 0x%02X\n", cartridge.header.flags_7);

    printf("Mapper: 0x%02X\n", cartridge.mapper);
}


uint8_t cpu_read_8(uint16_t address) {
    if (address < 0x2000) {
        return nes.ram[address & 0x07ff];
    } else if (address >= 0x8000) {
        return cartridge.prg_rom[(address - 0x8000) & (cartridge.header.prg_size == 1 ? 0x3fff : 0xffff)];
    }

    eprintf("WARNING: Read from unmapped address: %04x\n", address);
    return 0;
}

uint16_t cpu_read_16(uint16_t address) {
    return cpu_read_8(address) | (cpu_read_8(address + 1) << 8);
}

void cpu_write_8(uint16_t address, uint8_t data) {
    if (address < 0x2000) {
        nes.ram[address & 0x07ff] = data;
    } else if (address >= 0x8000) {
        cartridge.prg_rom[address - 0x8000] = data;
    } else {
        eprintf("WARNING: Write to unmapped address: %04x with data: %02X\n", address, data);
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
    if (nes.cpu.p & (1 << f)) {
        return 1;
    } else {
        return 0;
    }
}

void stack_push_8(uint8_t data) {
    nes.ram[0x0100 + nes.cpu.s--] = data;
}

void stack_push_16(uint16_t data) {
    stack_push_8(data >> 8);
    stack_push_8(data & 0xff);
}

uint8_t stack_pop_8() {
    return nes.ram[0x0100 + ++nes.cpu.s];
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
            eprintf("ERROR: Unable to find length of instruction with unknown addressing mode with id: %d\nHalting execution\n", mode);
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
            return cpu_read_8(operand_16) + 256 * cpu_read_8((operand_16 & 0xff00) | (((operand_16 & 0xff) + 1) % 256));
        case ZERO_PAGE_X:
            return (operand_8 + nes.cpu.x) % 256;
        case ZERO_PAGE_Y:
            return (operand_8 + nes.cpu.y) % 256;
        case ABSOLUTE_X:
            return operand_16 + nes.cpu.x;
        case ABSOLUTE_Y:
            return operand_16 + nes.cpu.y;
        case INDEXED_INDIRECT:
            return cpu_read_8((operand_8 + nes.cpu.x) % 256) + 256 * cpu_read_8((operand_8 + nes.cpu.x + 1) % 256);
        case INDIRECT_INDEXED:
            return cpu_read_8(operand_8) + 256 * cpu_read_8((operand_8 + 1) % 256) + nes.cpu.y;
        default:
            eprintf("ERROR: Unknown addressing mode with id: %d\nHalting execution\n", mode);
            exit(EXIT_FAILURE);
    }
}

bool page_cross(enum address_mode mode) {
    switch (mode) {
        case ABSOLUTE_X:
            return (cpu_read_16(nes.cpu.pc + 1) & 0xff) + nes.cpu.x > 0xff;
        case ABSOLUTE_Y:
            return (cpu_read_16(nes.cpu.pc + 1) & 0xff) + nes.cpu.y > 0xff;
        case INDIRECT_INDEXED:
            return ((cpu_read_16(cpu_read_8(nes.cpu.pc + 1)) & 0xff) + nes.cpu.y) > 0xff;
        default:
            return 0;
    }
}

void print_next_instruction() {
    uint8_t opcode = cpu_read_8(nes.cpu.pc);

    enum instruction_name name = INSTRUCTION_LOOKUP[opcode];
    enum address_mode mode = ADDRESS_MODE_LOOKUP[opcode];

    uint16_t address = read_operand(mode);

    printf("%04X ", nes.cpu.pc);
    int indent = 5;
    for (int i = 0; i < instruction_length(mode); i++) {
        printf(" %02X", cpu_read_8(nes.cpu.pc + i));
        indent += 3;
    }
    printf("%*c", 16 - indent, ' ');
    indent = 16;
    printf("%s ", INSTRUCTION_NAME_STRING[name]);

    indent = 0;
    switch (mode) {
        case IMMEDIATE:
            printf("#$%02X%n", cpu_read_8(address), &indent);
            break;
        case ACCUMULATOR:
            printf("A");
            indent = 1;
            break;
        case RELATIVE:
            printf("$%04X%n", nes.cpu.pc + cpu_read_8(address) + 2, &indent);
            break;
        case ZERO_PAGE:
            printf("$%02X%n", address, &indent);
            break;
        case ABSOLUTE:
            printf("$%04X%n", address, &indent);
            break;
        case INDIRECT:
            printf("($%04X) = %04X%n", cpu_read_16(nes.cpu.pc + 1), address, &indent);
            break;
        case ZERO_PAGE_X:
            printf("$%02X,X @ %02X%n", cpu_read_8(nes.cpu.pc + 1), address, &indent);
            break;
        case ZERO_PAGE_Y:
            printf("$%02X,Y @ %02X%n", cpu_read_8(nes.cpu.pc + 1), address, &indent);
            break;
        case ABSOLUTE_X:
            printf("$%04X,X @ %04X%n", cpu_read_16(nes.cpu.pc + 1), address, &indent);
            break;
        case ABSOLUTE_Y:
            printf("$%04X,Y @ %04X%n", cpu_read_16(nes.cpu.pc + 1), address, &indent);
            break;
        case INDEXED_INDIRECT:
            printf("($%02X,X) @ %02X = %04X%n", cpu_read_8(nes.cpu.pc + 1), (uint8_t)(cpu_read_8(nes.cpu.pc + 1) + nes.cpu.x), address, &indent);
            break;
        case INDIRECT_INDEXED:
            printf("($%02X),Y = %04X @ %04X%n", cpu_read_8(nes.cpu.pc + 1), cpu_read_8(cpu_read_8(nes.cpu.pc + 1)) + 256 * cpu_read_8((cpu_read_8(nes.cpu.pc + 1) + 1) % 256), address, &indent);
            break;

        case IMPLICIT:
        default:
            break;
    }

    if (mode != IMMEDIATE && mode != ACCUMULATOR) {
        int store_add = 0;
        if (name == STA || name == STX || name == STY ||
                name == LDA || name == LDX || name == LDY ||
                name == ORA || name == EOR || name == AND ||
                name == ADC || name == SBC || name == BIT ||
                name == CMP || name == CPX || name == CPY ||
                name == LSR || name == ROR ||
                name == ASL || name == ROL ||
                name == INC || name == DEC
           ) {
            printf(" = %02X%n", cpu_read_8(address), &store_add);
        }
        indent += store_add;
    }

    printf("%*cA:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%ld\n", 28 - indent, ' ', nes.cpu.a, nes.cpu.x, nes.cpu.y, nes.cpu.p, nes.cpu.s, state.cycles);
}


uint8_t _adc(enum address_mode mode, uint16_t address) {
    uint8_t data = cpu_read_8(address);

    int result = nes.cpu.a + data + get_flag(CARRY);

    set_flag(CARRY, result > 0xff);
    set_flag(ZERO, (result & 0xff) == 0);
    set_flag(OVERFLOW, ~(nes.cpu.a ^ data) & (nes.cpu.a ^ result) & 0x80);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.a = result;

    return 0;
}

uint8_t _and(enum address_mode mode, uint16_t address) {
    uint8_t result = nes.cpu.a & cpu_read_8(address);

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.a = result;

    return 0;
}

uint8_t _asl(enum address_mode mode, uint16_t address) {
    int result;
    if (mode == ACCUMULATOR) {
        result = nes.cpu.a << 1;
    } else {
        result = cpu_read_8(address) << 1;
    }

    set_flag(CARRY, result & 0x0100);
    set_flag(ZERO, (result & 0xff) == 0);
    set_flag(NEGATIVE, result & 0x80);

    if (mode == ACCUMULATOR) {
        nes.cpu.a = result;
    } else {
        cpu_write_8(address, result);
    }

    if (mode == ACCUMULATOR) {
        return 0;
    } else if (mode == ABSOLUTE_X) {
        return 3;
    } else {
        return 2;
    }
}

uint8_t _bcc(enum address_mode mode, uint16_t address) {
    uint8_t cycles = 0;
    uint16_t initial_pc = nes.cpu.pc;

    if (!get_flag(CARRY)) {
        nes.cpu.pc += cpu_read_8(address) + 2;
        cycles += 1;
    }

    if (nes.cpu.pc >> 8 != initial_pc >> 8) {
        cycles += 2;
    }

    return cycles;
}

uint8_t _bcs(enum address_mode mode, uint16_t address) {
    uint8_t cycles = 0;
    uint16_t initial_pc = nes.cpu.pc;

    if (get_flag(CARRY)) {
        nes.cpu.pc += cpu_read_8(address) + 2;
        cycles += 1;
    }

    if (nes.cpu.pc >> 8 != initial_pc >> 8) {
        cycles += 2;
    }

    return cycles;
}

uint8_t _beq(enum address_mode mode, uint16_t address) {
    uint8_t cycles = 0;
    // uint16_t initial_pc = nes.cpu.pc;

    if (get_flag(ZERO)) {
        nes.cpu.pc += cpu_read_8(address) + 2;
        cycles += 1;
    }

    // if (nes.cpu.pc >> 8 != initial_pc >> 8) {
    //     cycles += 2;
    // }

    return cycles;
}

uint8_t _bit(enum address_mode mode, uint16_t address) {
    uint8_t data = cpu_read_8(address);
    uint8_t result = nes.cpu.a & data;

    set_flag(ZERO, result == 0);

    set_flag(OVERFLOW, data & 0x40);
    set_flag(NEGATIVE, data & 0x80);

    return 0;
}

uint8_t _bmi(enum address_mode mode, uint16_t address) {
    uint8_t cycles = 0;
    uint16_t initial_pc = nes.cpu.pc;

    if (get_flag(NEGATIVE)) {
        nes.cpu.pc += cpu_read_8(address) + 2;
        cycles += 1;
    }

    if (nes.cpu.pc >> 8 != initial_pc >> 8) {
        cycles += 2;
    }

    return cycles;
}

uint8_t _bne(enum address_mode mode, uint16_t address) {
    uint8_t cycles = 0;
    uint16_t initial_pc = nes.cpu.pc;

    if (!get_flag(ZERO)) {
        nes.cpu.pc += cpu_read_8(address) + 2;
        cycles += 1;
    }

    if (nes.cpu.pc >> 8 != initial_pc >> 8) {
        cycles += 2;
    }

    return cycles;
}

uint8_t _bpl(enum address_mode mode, uint16_t address) {
    uint8_t cycles = 0;
    uint16_t initial_pc = nes.cpu.pc;

    if (!get_flag(NEGATIVE)) {
        nes.cpu.pc += cpu_read_8(address) + 2;
        cycles += 1;
    }

    if (nes.cpu.pc >> 8 != initial_pc >> 8) {
        cycles += 2;
    }

    return cycles;
}

uint8_t _brk(enum address_mode mode, uint16_t address) {
    stack_push_16(nes.cpu.pc);
    stack_push_8(nes.cpu.p);
    nes.cpu.pc = cpu_read_16(IRQ_VECTOR);
    set_flag(BREAK, true);

    return 0;
}

uint8_t _bvc(enum address_mode mode, uint16_t address) {
    uint8_t cycles = 0;
    uint16_t initial_pc = nes.cpu.pc;

    if (!get_flag(OVERFLOW)) {
        nes.cpu.pc += cpu_read_8(address) + 2;
        cycles += 1;
    }

    if (nes.cpu.pc >> 8 != initial_pc >> 8) {
        cycles += 2;
    }

    return cycles;
}

uint8_t _bvs(enum address_mode mode, uint16_t address) {
    uint8_t cycles = 0;
    uint16_t initial_pc = nes.cpu.pc;

    if (get_flag(OVERFLOW)) {
        nes.cpu.pc += cpu_read_8(address) + 2;
        cycles += 1;
    }

    if (nes.cpu.pc >> 8 != initial_pc >> 8) {
        cycles += 2;
    }

    return cycles;
}

uint8_t _clc(enum address_mode mode, uint16_t address) {
    set_flag(CARRY, false);

    return 0;
}

uint8_t _cld(enum address_mode mode, uint16_t address) {
    set_flag(DECIMAL, false);

    return 0;
}

uint8_t _cli(enum address_mode mode, uint16_t address) {
    set_flag(INTERRUPT, false);

    return 0;
}

uint8_t _clv(enum address_mode mode, uint16_t address) {
    set_flag(OVERFLOW, false);

    return 0;
}

uint8_t _cmp(enum address_mode mode, uint16_t address) {
    uint8_t data = cpu_read_8(address);
    uint8_t result = nes.cpu.a - data;

    set_flag(CARRY, nes.cpu.a >= data);
    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    return 0;
}

uint8_t _cpx(enum address_mode mode, uint16_t address) {
    uint8_t data = cpu_read_8(address);
    uint8_t result = nes.cpu.x - data;

    set_flag(CARRY, nes.cpu.x >= data);
    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    return 0;
}

uint8_t _cpy(enum address_mode mode, uint16_t address) {
    uint8_t data = cpu_read_8(address);
    uint8_t result = nes.cpu.y - data;

    set_flag(CARRY, nes.cpu.y >= data);
    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    return 0;
}

uint8_t _dec(enum address_mode mode, uint16_t address) {
    uint8_t result = cpu_read_8(address) - 1;

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    cpu_write_8(address, result);

    if (mode == ABSOLUTE_X) {
        return 1;
    } else {
        return 0;
    }
}

uint8_t _dex(enum address_mode mode, uint16_t address) {
    uint8_t result = nes.cpu.x - 1;

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.x = result;

    return 0;
}

uint8_t _dey(enum address_mode mode, uint16_t address) {
    uint8_t result = nes.cpu.y - 1;

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.y = result;

    return 0;
}

uint8_t _eor(enum address_mode mode, uint16_t address) {
    uint8_t result = nes.cpu.a ^ cpu_read_8(address);

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.a = result;

    return 0;
}

uint8_t _inc(enum address_mode mode, uint16_t address) {
    uint8_t result = cpu_read_8(address) + 1;

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    cpu_write_8(address, result);

    if (mode == ABSOLUTE_X) {
        return 1;
    } else {
        return 0;
    }
}

uint8_t _inx(enum address_mode mode, uint16_t address) {
    uint8_t result = nes.cpu.x + 1;

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.x = result;

    return 0;
}

uint8_t _iny(enum address_mode mode, uint16_t address) {
    uint8_t result = nes.cpu.y + 1;

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.y = result;

    return 0;
}

uint8_t _jmp(enum address_mode mode, uint16_t address) {
    // TODO: Indirect mode page boundary bug
    nes.cpu.pc = address;

    return 0;
}

uint8_t _jsr(enum address_mode mode, uint16_t address) {
    stack_push_16(nes.cpu.pc + instruction_length(mode) - 1);
    nes.cpu.pc = address;

    return 0;
}

uint8_t _lda(enum address_mode mode, uint16_t address) {
    uint8_t data = cpu_read_8(address);

    set_flag(ZERO, data == 0);
    set_flag(NEGATIVE, data & 0x80);

    nes.cpu.a = data;

    if ((mode == ABSOLUTE_X || mode == ABSOLUTE_Y ||
        mode == INDIRECT_INDEXED) && page_cross(mode)) {
        return 1;
    } else {
        return 0;
    }
}

uint8_t _ldx(enum address_mode mode, uint16_t address) {
    uint8_t data = cpu_read_8(address);

    set_flag(ZERO, data == 0);
    set_flag(NEGATIVE, data & 0x80);

    nes.cpu.x = data;

    if (mode == ABSOLUTE_Y && page_cross(mode)) {
        return 1;
    } else {
        return 0;
    }
}

uint8_t _ldy(enum address_mode mode, uint16_t address) {
    uint8_t data = cpu_read_8(address);

    set_flag(ZERO, data == 0);
    set_flag(NEGATIVE, data & 0x80);

    nes.cpu.y = data;

    if (mode == ABSOLUTE_X && page_cross(mode)) {
        return 1;
    } else {
        return 0;
    }
}

uint8_t _lsr(enum address_mode mode, uint16_t address) {
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

    if (mode == ACCUMULATOR) {
        return 0;
    } else if (mode == ABSOLUTE_X) {
        return 3;
    } else {
        return 2;
    }
}

uint8_t _nop(enum address_mode mode, uint16_t address) {return 0;}

uint8_t _ora(enum address_mode mode, uint16_t address) {
    uint8_t result = nes.cpu.a | cpu_read_8(address);

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.a = result;

    return 0;
}

uint8_t _pha(enum address_mode mode, uint16_t address) {
    stack_push_8(nes.cpu.a);

    return 0;
}

uint8_t _php(enum address_mode mode, uint16_t address) {
    stack_push_8(nes.cpu.p | (1 << BREAK));

    return 0;
}

uint8_t _pla(enum address_mode mode, uint16_t address) {
    uint8_t result = stack_pop_8();

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.a = result;

    return 0;
}

uint8_t _plp(enum address_mode mode, uint16_t address) {
    uint8_t initial_flags = nes.cpu.p;
    uint8_t result = stack_pop_8();

    nes.cpu.p = result;
    set_flag(ONE, true);
    set_flag(BREAK, initial_flags & (1 << BREAK));

    return 0;
}

uint8_t _rol(enum address_mode mode, uint16_t address) {
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

    if (mode == ACCUMULATOR) {
        return 0;
    } else if (mode == ABSOLUTE_X) {
        return 3;
    } else {
        return 2;
    }
}

uint8_t _ror(enum address_mode mode, uint16_t address) {
    uint8_t data;
    if (mode == ACCUMULATOR) {
        data = nes.cpu.a;
    } else {
        data = cpu_read_8(address);
    }

    uint8_t result = data >> 1 | (get_flag(CARRY) << 7);

    set_flag(CARRY, data & 1);
    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    if (mode == ACCUMULATOR) {
        nes.cpu.a = result;
    } else {
        cpu_write_8(address, result);
    }

    if (mode == ACCUMULATOR) {
        return 0;
    } else if (mode == ABSOLUTE_X) {
        return 3;
    } else {
        return 2;
    }
}

uint8_t _rti(enum address_mode mode, uint16_t address) {
    nes.cpu.p = stack_pop_8();
    set_flag(ONE, true);
    nes.cpu.pc = stack_pop_16();

    return 0;
}

uint8_t _rts(enum address_mode mode, uint16_t address) {
    nes.cpu.pc = stack_pop_16() + 1;

    return 0;
}

uint8_t _sbc(enum address_mode mode, uint16_t address) {
    uint8_t data = ~cpu_read_8(address);

    int result = nes.cpu.a + data + get_flag(CARRY);

    set_flag(CARRY, result > 0xff);
    set_flag(ZERO, (result & 0xff) == 0);
    set_flag(OVERFLOW, ~(nes.cpu.a ^ data) & (nes.cpu.a ^ result) & 0x80);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.a = result;

    return 0;
}

uint8_t _sec(enum address_mode mode, uint16_t address) {
    set_flag(CARRY, 1);

    return 0;
}

uint8_t _sed(enum address_mode mode, uint16_t address) {
    set_flag(DECIMAL, 1);

    return 0;
}

uint8_t _sei(enum address_mode mode, uint16_t address) {
    set_flag(INTERRUPT, 1);

    return 0;
}

uint8_t _sta(enum address_mode mode, uint16_t address) {
    cpu_write_8(address, nes.cpu.a);

    if (mode == ABSOLUTE_X || mode == ABSOLUTE_Y || mode == INDIRECT_INDEXED) {
        return 1;
    } else {
        return 0;
    }
}

uint8_t _stx(enum address_mode mode, uint16_t address) {
    cpu_write_8(address, nes.cpu.x);

    return 0;
}

uint8_t _sty(enum address_mode mode, uint16_t address) {
    cpu_write_8(address, nes.cpu.y);

    return 0;
}

uint8_t _tax(enum address_mode mode, uint16_t address) {
    uint8_t result = nes.cpu.a;

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.x = result;

    return 0;
}

uint8_t _tay(enum address_mode mode, uint16_t address) {
    uint8_t result = nes.cpu.a;

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.y = result;

    return 0;
}

uint8_t _tsx(enum address_mode mode, uint16_t address) {
    uint8_t result = nes.cpu.s;

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.x = result;

    return 0;
}

uint8_t _txa(enum address_mode mode, uint16_t address) {
    uint8_t result = nes.cpu.x;

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.a = result;

    return 0;
}

uint8_t _txs(enum address_mode mode, uint16_t address) {
    nes.cpu.s = nes.cpu.x;

    return 0;
}

uint8_t _tya(enum address_mode mode, uint16_t address) {
    uint8_t result = nes.cpu.y;

    set_flag(ZERO, result == 0);
    set_flag(NEGATIVE, result & 0x80);

    nes.cpu.a = result;

    return 0;
}


int execute_next() {
    uint16_t initial_pc = nes.cpu.pc;

    uint8_t opcode = cpu_read_8(nes.cpu.pc);

    enum instruction_name name = INSTRUCTION_LOOKUP[opcode];
    enum address_mode mode = ADDRESS_MODE_LOOKUP[opcode];

    uint8_t cycles = INSTRUCTION_CYCLES[name] + ADDRESS_MODE_CYCLES[mode];

    uint16_t address = read_operand(mode);

    if (DEBUG) {
        print_next_instruction();
    }

    switch (name) {
        case ADC: cycles += _adc(mode, address); break;
        case AND: cycles += _and(mode, address); break;
        case ASL: cycles += _asl(mode, address); break;
        case BCC: cycles += _bcc(mode, address); break;
        case BCS: cycles += _bcs(mode, address); break;
        case BEQ: cycles += _beq(mode, address); break;
        case BIT: cycles += _bit(mode, address); break;
        case BMI: cycles += _bmi(mode, address); break;
        case BNE: cycles += _bne(mode, address); break;
        case BPL: cycles += _bpl(mode, address); break;
        case BRK: cycles += _brk(mode, address); break;
        case BVC: cycles += _bvc(mode, address); break;
        case BVS: cycles += _bvs(mode, address); break;
        case CLC: cycles += _clc(mode, address); break;
        case CLD: cycles += _cld(mode, address); break;
        case CLI: cycles += _cli(mode, address); break;
        case CLV: cycles += _clv(mode, address); break;
        case CMP: cycles += _cmp(mode, address); break;
        case CPX: cycles += _cpx(mode, address); break;
        case CPY: cycles += _cpy(mode, address); break;
        case DEC: cycles += _dec(mode, address); break;
        case DEX: cycles += _dex(mode, address); break;
        case DEY: cycles += _dey(mode, address); break;
        case EOR: cycles += _eor(mode, address); break;
        case INC: cycles += _inc(mode, address); break;
        case INX: cycles += _inx(mode, address); break;
        case INY: cycles += _iny(mode, address); break;
        case JMP: cycles += _jmp(mode, address); break;
        case JSR: cycles += _jsr(mode, address); break;
        case LDA: cycles += _lda(mode, address); break;
        case LDX: cycles += _ldx(mode, address); break;
        case LDY: cycles += _ldy(mode, address); break;
        case LSR: cycles += _lsr(mode, address); break;
        case NOP: cycles += _nop(mode, address); break;
        case ORA: cycles += _ora(mode, address); break;
        case PHA: cycles += _pha(mode, address); break;
        case PHP: cycles += _php(mode, address); break;
        case PLA: cycles += _pla(mode, address); break;
        case PLP: cycles += _plp(mode, address); break;
        case ROL: cycles += _rol(mode, address); break;
        case ROR: cycles += _ror(mode, address); break;
        case RTI: cycles += _rti(mode, address); break;
        case RTS: cycles += _rts(mode, address); break;
        case SBC: cycles += _sbc(mode, address); break;
        case SEC: cycles += _sec(mode, address); break;
        case SED: cycles += _sed(mode, address); break;
        case SEI: cycles += _sei(mode, address); break;
        case STA: cycles += _sta(mode, address); break;
        case STX: cycles += _stx(mode, address); break;
        case STY: cycles += _sty(mode, address); break;
        case TAX: cycles += _tax(mode, address); break;
        case TAY: cycles += _tay(mode, address); break;
        case TSX: cycles += _tsx(mode, address); break;
        case TXA: cycles += _txa(mode, address); break;
        case TXS: cycles += _txs(mode, address); break;
        case TYA: cycles += _tya(mode, address); break;
        default:
            eprintf("ERROR: Unknown instruction with opcode: 0x%02X\nHalting execution\n", opcode);
            break;
    }

    // Prevent jmp and branch instructions from skipping the next instruction
    if (initial_pc == nes.cpu.pc) {
        nes.cpu.pc += instruction_length(mode);
    }

    return cycles;
}

void poweron() {
    nes.cpu.pc = cpu_read_16(RESET_VECTOR);
    if (DEBUG) nes.cpu.pc = 0xc000; // nestest.nes
    state.cycles = 7;
    nes.cpu.s = 0xfd;
    set_flag(INTERRUPT, true);
    set_flag(ONE, true);
}

void run() {
    for (;;) {
        if (state.cycles_queue == 0) {
            state.cycles_queue = execute_next();
        } else {
            state.cycles_queue--;
            state.cycles++;
        }
    }
}

int main(int argc, char **argv) {
    atexit(cleanup);

    if (argc < 2) {
        printf("Please provide a file\n");
        exit(EXIT_FAILURE);
    }

    if (argc > 2 && strcmp(argv[2], "--debug") == 0) {
        DEBUG = true;
    }

    state.data = read_file(argv[1]);

    if (state.data == NULL) {
        printf("Please provide a valid file\n");
        exit(EXIT_FAILURE);
    }

    load_cartridge();
    print_header();

    if (argc > 2 && strcmp(argv[2], "--read-header") == 0) {
        exit(EXIT_SUCCESS);
    }

    uint8_t ram[2048] = { 0 };

    nes.ram = ram;

    poweron();
    run();
}

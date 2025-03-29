/*
 *  decoder.c
 *
 *  Implements 'decoder.h'.
 */

#include "decoder.h"

/* Global instruction table */
static const instruction_t gc_instructions[] =
{
    /* Standard 6500 set (documented only) */
    { "BRK",     0x00, S65_IMPLIED,             S65_SET_6500     },
    { "ORA",     0x01, S65_INDIRECT_X,          S65_SET_6500     },
    { "ORA",     0x05, S65_ZEROPAGE,            S65_SET_6500     },
    { "ASL",     0x06, S65_ZEROPAGE,            S65_SET_6500     },
    { "PHP",     0x08, S65_IMPLIED,             S65_SET_6500     },
    { "ORA",     0x09, S65_IMMEDIATE,           S65_SET_6500     },
    { "ASL",     0x0A, S65_IMPLIED,             S65_SET_6500     },
    { "ORA",     0x0D, S65_ABSOLUTE,            S65_SET_6500     },
    { "ASL",     0x0E, S65_ABSOLUTE,            S65_SET_6500     },
    { "BPL",     0x10, S65_RELATIVE,            S65_SET_6500     },
    { "ORA",     0x11, S65_INDIRECT_Y,          S65_SET_6500     },
    { "ORA",     0x15, S65_ZEROPAGE_X,          S65_SET_6500     },
    { "ASL",     0x16, S65_ZEROPAGE_X,          S65_SET_6500     },
    { "CLC",     0x18, S65_IMPLIED,             S65_SET_6500     },
    { "ORA",     0x19, S65_ABSOLUTE_Y,          S65_SET_6500     },
    { "ORA",     0x1D, S65_ABSOLUTE_X,          S65_SET_6500     },
    { "ASL",     0x1E, S65_ABSOLUTE_X,          S65_SET_6500     },
    { "JSR",     0x20, S65_ABSOLUTE,            S65_SET_6500     },
    { "AND",     0x21, S65_INDIRECT_X,          S65_SET_6500     },
    { "BIT",     0x24, S65_ZEROPAGE,            S65_SET_6500     },
    { "AND",     0x25, S65_ZEROPAGE,            S65_SET_6500     },
    { "ROL",     0x26, S65_ZEROPAGE,            S65_SET_6500     },
    { "PLP",     0x28, S65_IMPLIED,             S65_SET_6500     },
    { "AND",     0x29, S65_IMMEDIATE,           S65_SET_6500     },
    { "ROL",     0x2A, S65_IMPLIED,             S65_SET_6500     },
    { "BIT",     0x2C, S65_ABSOLUTE,            S65_SET_6500     },
    { "AND",     0x2D, S65_ABSOLUTE,            S65_SET_6500     },
    { "ROL",     0x2E, S65_ABSOLUTE,            S65_SET_6500     },
    { "BMI",     0x30, S65_RELATIVE,            S65_SET_6500     },
    { "AND",     0x31, S65_INDIRECT_Y,          S65_SET_6500     },
    { "AND",     0x35, S65_ZEROPAGE_X,          S65_SET_6500     },
    { "ROL",     0x36, S65_ZEROPAGE_X,          S65_SET_6500     },
    { "SEC",     0x38, S65_IMPLIED,             S65_SET_6500     },
    { "AND",     0x39, S65_ABSOLUTE_Y,          S65_SET_6500     },
    { "AND",     0x3D, S65_ABSOLUTE_X,          S65_SET_6500     },
    { "ROL",     0x3E, S65_ABSOLUTE_X,          S65_SET_6500     },
    { "RTI",     0x40, S65_IMPLIED,             S65_SET_6500     },
    { "EOR",     0x41, S65_INDIRECT_X,          S65_SET_6500     },
    { "EOR",     0x45, S65_ZEROPAGE_X,          S65_SET_6500     },
    { "LSR",     0x46, S65_ZEROPAGE,            S65_SET_6500     },
    { "PHA",     0x48, S65_IMPLIED,             S65_SET_6500     },
    { "EOR",     0x49, S65_IMMEDIATE,           S65_SET_6500     },
    { "LSR",     0x4A, S65_IMPLIED,             S65_SET_6500     },
    { "JMP",     0x4C, S65_ABSOLUTE,            S65_SET_6500     },
    { "EOR",     0x4D, S65_ABSOLUTE,            S65_SET_6500     },
    { "LSR",     0x4E, S65_ABSOLUTE,            S65_SET_6500     },
    { "BVC",     0x50, S65_RELATIVE,            S65_SET_6500     },
    { "EOR",     0x51, S65_INDIRECT_Y,          S65_SET_6500     },
    { "EOR",     0x55, S65_ZEROPAGE_X,          S65_SET_6500     },
    { "LSR",     0x56, S65_ZEROPAGE_X,          S65_SET_6500     },
    { "CLI",     0x58, S65_IMPLIED,             S65_SET_6500     },
    { "EOR",     0x59, S65_ABSOLUTE_Y,          S65_SET_6500     },
    { "EOR",     0x5D, S65_ABSOLUTE_X,          S65_SET_6500     },
    { "LSR",     0x5E, S65_ABSOLUTE_X,          S65_SET_6500     },
    { "RTS",     0x60, S65_IMPLIED,             S65_SET_6500     },
    { "ADC",     0x61, S65_INDIRECT_X,          S65_SET_6500     },
    { "ADC",     0x65, S65_ZEROPAGE,            S65_SET_6500     },
    { "ROR",     0x66, S65_ZEROPAGE,            S65_SET_6500     },
    { "PLA",     0x68, S65_IMPLIED,             S65_SET_6500     },
    { "ADC",     0x69, S65_IMMEDIATE,           S65_SET_6500     },
    { "ROR",     0x6A, S65_IMPLIED,             S65_SET_6500     },
    { "JMP",     0x6C, S65_INDIRECT,            S65_SET_6500     },
    { "ADC",     0x6D, S65_ABSOLUTE,            S65_SET_6500     },
    { "ROR",     0x6E, S65_ABSOLUTE,            S65_SET_6500     },
    { "BVS",     0x70, S65_RELATIVE,            S65_SET_6500     },
    { "ADC",     0x71, S65_INDIRECT_Y,          S65_SET_6500     },
    { "ADC",     0x75, S65_ZEROPAGE_X,          S65_SET_6500     },
    { "ROR",     0x76, S65_ZEROPAGE_X,          S65_SET_6500     },
    { "SEI",     0x78, S65_IMPLIED,             S65_SET_6500     },
    { "ADC",     0x79, S65_ABSOLUTE_Y,          S65_SET_6500     },
    { "ADC",     0x7D, S65_ABSOLUTE_X,          S65_SET_6500     },
    { "ROR",     0x7E, S65_ABSOLUTE_X,          S65_SET_6500     },
    { "STA",     0x81, S65_INDIRECT_X,          S65_SET_6500     },
    { "STY",     0x84, S65_ZEROPAGE,            S65_SET_6500     },
    { "STA",     0x85, S65_ZEROPAGE,            S65_SET_6500     },
    { "STX",     0x86, S65_ZEROPAGE,            S65_SET_6500     },
    { "DEY",     0x88, S65_IMPLIED,             S65_SET_6500     },
    { "TXA",     0x8A, S65_IMPLIED,             S65_SET_6500     },
    { "STY",     0x8C, S65_ABSOLUTE,            S65_SET_6500     },
    { "STA",     0x8D, S65_ABSOLUTE,            S65_SET_6500     },
    { "STX",     0x8E, S65_ABSOLUTE,            S65_SET_6500     },
    { "BCC",     0x90, S65_RELATIVE,            S65_SET_6500     },
    { "STA",     0x91, S65_INDIRECT_Y,          S65_SET_6500     },
    { "STY",     0x94, S65_ZEROPAGE_X,          S65_SET_6500     },
    { "STA",     0x95, S65_ZEROPAGE_X,          S65_SET_6500     },
    { "STX",     0x96, S65_ZEROPAGE_Y,          S65_SET_6500     },
    { "TYA",     0x98, S65_IMPLIED,             S65_SET_6500     },
    { "STA",     0x99, S65_ABSOLUTE_Y,          S65_SET_6500     },
    { "TXS",     0x9A, S65_IMPLIED,             S65_SET_6500     },
    { "STA",     0x9D, S65_ABSOLUTE_X,          S65_SET_6500     },
    { "LDY",     0xA0, S65_IMMEDIATE,           S65_SET_6500     },
    { "LDA",     0xA1, S65_INDIRECT_X,          S65_SET_6500     },
    { "LDX",     0xA2, S65_IMMEDIATE,           S65_SET_6500     },
    { "LDY",     0xA4, S65_ZEROPAGE,            S65_SET_6500     },
    { "LDA",     0xA5, S65_ZEROPAGE,            S65_SET_6500     },
    { "LDX",     0xA6, S65_ZEROPAGE,            S65_SET_6500     },
    { "TAY",     0xA8, S65_IMPLIED,             S65_SET_6500     },
    { "LDA",     0xA9, S65_IMMEDIATE,           S65_SET_6500     },
    { "TAX",     0xAA, S65_IMPLIED,             S65_SET_6500     },
    { "LDY",     0xAC, S65_ABSOLUTE,            S65_SET_6500     },
    { "LDA",     0xAD, S65_ABSOLUTE,            S65_SET_6500     },
    { "LDX",     0xAE, S65_ABSOLUTE,            S65_SET_6500     },
    { "BCS",     0xB0, S65_RELATIVE,            S65_SET_6500     },
    { "LDA",     0xB1, S65_INDIRECT_Y,          S65_SET_6500     },
    { "LDY",     0xB4, S65_ZEROPAGE_X,          S65_SET_6500     },
    { "LDA",     0xB5, S65_ZEROPAGE_X,          S65_SET_6500     },
    { "LDX",     0xB6, S65_ZEROPAGE_Y,          S65_SET_6500     },
    { "CLV",     0xB8, S65_IMPLIED,             S65_SET_6500     },
    { "LDA",     0xB9, S65_ABSOLUTE_Y,          S65_SET_6500     },
    { "TSX",     0xBA, S65_IMPLIED,             S65_SET_6500     },
    { "LDY",     0xBC, S65_ABSOLUTE_X,          S65_SET_6500     },
    { "LDA",     0xBD, S65_ABSOLUTE_X,          S65_SET_6500     },
    { "LDX",     0xBE, S65_ABSOLUTE_Y,          S65_SET_6500     },
    { "CPY",     0xC0, S65_IMMEDIATE,           S65_SET_6500     },
    { "CMP",     0xC1, S65_INDIRECT_X,          S65_SET_6500     },
    { "CPY",     0xC4, S65_ZEROPAGE,            S65_SET_6500     },
    { "CMP",     0xC5, S65_ZEROPAGE,            S65_SET_6500     },
    { "DEC",     0xC6, S65_ZEROPAGE,            S65_SET_6500     },
    { "INY",     0xC8, S65_IMPLIED,             S65_SET_6500     },
    { "CMP",     0xC9, S65_IMMEDIATE,           S65_SET_6500     },
    { "DEX",     0xCA, S65_IMPLIED,             S65_SET_6500     },
    { "CPY",     0xCC, S65_ABSOLUTE,            S65_SET_6500     },
    { "CMP",     0xCD, S65_ABSOLUTE,            S65_SET_6500     },
    { "DEC",     0xCE, S65_ABSOLUTE,            S65_SET_6500     },
    { "BNE",     0xD0, S65_RELATIVE,            S65_SET_6500     },
    { "CMP",     0xD1, S65_INDIRECT_Y,          S65_SET_6500     },
    { "CMP",     0xD5, S65_ZEROPAGE_X,          S65_SET_6500     },
    { "DEC",     0xD6, S65_ZEROPAGE_X,          S65_SET_6500     },
    { "CLD",     0xD8, S65_IMPLIED,             S65_SET_6500     },
    { "CMP",     0xD9, S65_ABSOLUTE_Y,          S65_SET_6500     },
    { "CMP",     0xDD, S65_ABSOLUTE_X,          S65_SET_6500     },
    { "DEC",     0xDE, S65_ABSOLUTE_X,          S65_SET_6500     },
    { "CPX",     0xE0, S65_IMMEDIATE,           S65_SET_6500     },
    { "SBC",     0xE1, S65_INDIRECT_X,          S65_SET_6500     },
    { "CPX",     0xE4, S65_ZEROPAGE,            S65_SET_6500     },
    { "SBC",     0xE5, S65_ZEROPAGE,            S65_SET_6500     },
    { "INC",     0xE6, S65_ZEROPAGE,            S65_SET_6500     },
    { "INX",     0xE8, S65_IMPLIED,             S65_SET_6500     },
    { "SBC",     0xE9, S65_IMMEDIATE,           S65_SET_6500     },
    { "NOP",     0xEA, S65_IMPLIED,             S65_SET_6500     },
    { "CPX",     0xEC, S65_ABSOLUTE,            S65_SET_6500     },
    { "SBC",     0xED, S65_ABSOLUTE,            S65_SET_6500     },
    { "INC",     0xEE, S65_ABSOLUTE,            S65_SET_6500     },
    { "BEQ",     0xF0, S65_RELATIVE,            S65_SET_6500     },
    { "SBC",     0xF1, S65_INDIRECT_Y,          S65_SET_6500     },
    { "SBC",     0xF5, S65_ZEROPAGE_X,          S65_SET_6500     },
    { "INC",     0xF6, S65_ZEROPAGE_X,          S65_SET_6500     },
    { "SED",     0xF8, S65_IMPLIED,             S65_SET_6500     },
    { "SBC",     0xF9, S65_ABSOLUTE_Y,          S65_SET_6500     },
    { "SBC",     0xFD, S65_ABSOLUTE_X,          S65_SET_6500     },
    { "INC",     0xFE, S65_ABSOLUTE_X,          S65_SET_6500     },

    /* Undocumented original 6500 set */
    { "JAM!",    0x02, S65_IMPLIED,             S65_SET_6500     },
    { "SLO!",    0x03, S65_INDIRECT_X,          S65_SET_6500     },
    { "NOP!",    0x04, S65_ZEROPAGE,            S65_SET_6500     },
    { "SLO!",    0x07, S65_ZEROPAGE,            S65_SET_6500     },
    { "ANC!",    0x0B, S65_IMMEDIATE,           S65_SET_6500     },
    { "NOP!",    0x0C, S65_ABSOLUTE,            S65_SET_6500     },
    { "SLO!",    0x0F, S65_ABSOLUTE,            S65_SET_6500     },
    { "JAM!",    0x12, S65_IMPLIED,             S65_SET_6500     },
    { "SLO!",    0x13, S65_INDIRECT_Y,          S65_SET_6500     },
    { "NOP!",    0x14, S65_ZEROPAGE_X,          S65_SET_6500     },
    { "SLO!",    0x17, S65_ZEROPAGE_X,          S65_SET_6500     },
    { "NOP!",    0x1A, S65_IMPLIED,             S65_SET_6500     },
    { "SLO!",    0x1B, S65_ABSOLUTE_Y,          S65_SET_6500     },
    { "NOP!",    0x1C, S65_ABSOLUTE_X,          S65_SET_6500     },
    { "SLO!",    0x1F, S65_ABSOLUTE_X,          S65_SET_6500     },
    { "JAM!",    0x22, S65_IMPLIED,             S65_SET_6500     },
    { "RLA!",    0x23, S65_INDIRECT_X,          S65_SET_6500     },
    { "RLA!",    0x27, S65_ZEROPAGE,            S65_SET_6500     },
    { "ANC!",    0x2B, S65_IMMEDIATE,           S65_SET_6500     },
    { "RLA!",    0x2F, S65_ABSOLUTE,            S65_SET_6500     },
    { "JAM!",    0x32, S65_IMPLIED,             S65_SET_6500     },
    { "RLA!",    0x33, S65_INDIRECT_Y,          S65_SET_6500     },
    { "NOP!",    0x34, S65_ZEROPAGE_X,          S65_SET_6500     },
    { "RLA!",    0x37, S65_ZEROPAGE_X,          S65_SET_6500     },
    { "NOP!",    0x3A, S65_IMPLIED,             S65_SET_6500     },
    { "RLA!",    0x3B, S65_ABSOLUTE_Y,          S65_SET_6500     },
    { "NOP!",    0x3C, S65_ABSOLUTE_X,          S65_SET_6500     },
    { "RLA!",    0x3F, S65_ABSOLUTE_X,          S65_SET_6500     },
    { "JAM!",    0x42, S65_IMPLIED,             S65_SET_6500     },
    { "SRE!",    0x43, S65_INDIRECT_X,          S65_SET_6500     },
    { "NOP!",    0x44, S65_ZEROPAGE,            S65_SET_6500     },
    { "SRE!",    0x47, S65_ZEROPAGE,            S65_SET_6500     },
    { "ALR!",    0x4B, S65_IMMEDIATE,           S65_SET_6500     },
    { "SRE!",    0x4F, S65_ABSOLUTE,            S65_SET_6500     },
    { "JAM!",    0x52, S65_IMPLIED,             S65_SET_6500     },
    { "SRE!",    0x53, S65_INDIRECT_Y,          S65_SET_6500     },
    { "NOP!",    0x54, S65_ZEROPAGE_X,          S65_SET_6500     },
    { "SRE!",    0x57, S65_ZEROPAGE_X,          S65_SET_6500     },
    { "NOP!",    0x5A, S65_IMPLIED,             S65_SET_6500     },
    { "SRE!",    0x5B, S65_ABSOLUTE_Y,          S65_SET_6500     },
    { "NOP!",    0x5C, S65_ABSOLUTE_X,          S65_SET_6500     },
    { "SRE!",    0x5F, S65_ABSOLUTE_X,          S65_SET_6500     },
    { "JAM!",    0x62, S65_IMPLIED,             S65_SET_6500     },
    { "RRA!",    0x63, S65_INDIRECT_X,          S65_SET_6500     },
    { "NOP!",    0x64, S65_ZEROPAGE,            S65_SET_6500     },
    { "RRA!",    0x67, S65_ZEROPAGE,            S65_SET_6500     },
    { "ARR!",    0x6B, S65_IMMEDIATE,           S65_SET_6500     },
    { "RRA!",    0x6F, S65_ABSOLUTE,            S65_SET_6500     },
    { "JAM!",    0x72, S65_IMPLIED,             S65_SET_6500     },
    { "RRA!",    0x73, S65_INDIRECT_Y,          S65_SET_6500     },
    { "NOP!",    0x74, S65_ZEROPAGE_X,          S65_SET_6500     },
    { "RRA!",    0x77, S65_ZEROPAGE_X,          S65_SET_6500     },
    { "NOP!",    0x7A, S65_IMPLIED,             S65_SET_6500     },
    { "RRA!",    0x7B, S65_ABSOLUTE_Y,          S65_SET_6500     },
    { "NOP!",    0x7C, S65_ABSOLUTE_X,          S65_SET_6500     },
    { "RRA!",    0x7F, S65_ABSOLUTE_X,          S65_SET_6500     },
    { "NOP!",    0x80, S65_IMMEDIATE,           S65_SET_6500     },
    { "NOP!",    0x82, S65_IMMEDIATE,           S65_SET_6500     },
    { "SAX!",    0x83, S65_INDIRECT_X,          S65_SET_6500     },
    { "SAX!",    0x87, S65_ZEROPAGE,            S65_SET_6500     },
    { "NOP!",    0x89, S65_IMMEDIATE,           S65_SET_6500     },
    { "ANE!",    0x8B, S65_IMMEDIATE,           S65_SET_6500     },
    { "SAX!",    0x8F, S65_ABSOLUTE,            S65_SET_6500     },
    { "JAM!",    0x92, S65_IMPLIED,             S65_SET_6500     },
    { "SHA!",    0x93, S65_INDIRECT_Y,          S65_SET_6500     },
    { "SAX!",    0x97, S65_ZEROPAGE_Y,          S65_SET_6500     },
    { "TAS!",    0x9B, S65_ABSOLUTE_Y,          S65_SET_6500     },
    { "SHY!",    0x9C, S65_ABSOLUTE_X,          S65_SET_6500     },
    { "SHX!",    0x9E, S65_ABSOLUTE_Y,          S65_SET_6500     },
    { "SHA!",    0x9F, S65_ABSOLUTE_Y,          S65_SET_6500     },
    { "LAX!",    0xA3, S65_INDIRECT_X,          S65_SET_6500     },
    { "LAX!",    0xA7, S65_ZEROPAGE,            S65_SET_6500     },
    { "LXA!",    0xAB, S65_IMMEDIATE,           S65_SET_6500     },
    { "LAX!",    0xAF, S65_ABSOLUTE,            S65_SET_6500     },
    { "JAM!",    0xB2, S65_IMPLIED,             S65_SET_6500     },
    { "LAX!",    0xB3, S65_INDIRECT_Y,          S65_SET_6500     },
    { "LAX!",    0xB7, S65_ZEROPAGE_Y,          S65_SET_6500     },
    { "LAS!",    0xBB, S65_ABSOLUTE_Y,          S65_SET_6500     },
    { "LAX!",    0xBF, S65_ABSOLUTE_Y,          S65_SET_6500     },
    { "NOP!",    0xC2, S65_IMMEDIATE,           S65_SET_6500     },
    { "DCP!",    0xC3, S65_INDIRECT_X,          S65_SET_6500     },
    { "DCP!",    0xC7, S65_ZEROPAGE,            S65_SET_6500     },
    { "SBX!",    0xCB, S65_IMMEDIATE,           S65_SET_6500     },
    { "DCP!",    0xCF, S65_ABSOLUTE,            S65_SET_6500     },
    { "JAM!",    0xD2, S65_IMPLIED,             S65_SET_6500     },
    { "DCP!",    0xD3, S65_INDIRECT_Y,          S65_SET_6500     },
    { "NOP!",    0xD4, S65_ZEROPAGE_X,          S65_SET_6500     },
    { "DCP!",    0xD7, S65_ZEROPAGE_X,          S65_SET_6500     },
    { "NOP!",    0xDA, S65_IMPLIED,             S65_SET_6500     },
    { "DCP!",    0xDB, S65_ABSOLUTE_Y,          S65_SET_6500     },
    { "NOP!",    0xDC, S65_ABSOLUTE_X,          S65_SET_6500     },
    { "DCP!",    0xDF, S65_ABSOLUTE_X,          S65_SET_6500     },
    { "NOP!",    0xE2, S65_IMMEDIATE,           S65_SET_6500     },
    { "ISC!",    0xE3, S65_INDIRECT_X,          S65_SET_6500     },
    { "ISC!",    0xE7, S65_ZEROPAGE,            S65_SET_6500     },
    { "SBC!",    0xEB, S65_IMMEDIATE,           S65_SET_6500     },
    { "ISC!",    0xEF, S65_ABSOLUTE,            S65_SET_6500     },
    { "JAM!",    0xF2, S65_IMPLIED,             S65_SET_6500     },
    { "ISC!",    0xF3, S65_INDIRECT_Y,          S65_SET_6500     },
    { "NOP!",    0xF4, S65_ZEROPAGE_X,          S65_SET_6500     },
    { "ISC!",    0xF7, S65_ZEROPAGE_X,          S65_SET_6500     },
    { "NOP!",    0xFA, S65_IMPLIED,             S65_SET_6500     },
    { "ISC!",    0xFB, S65_ABSOLUTE_Y,          S65_SET_6500     },
    { "NOP!",    0xFC, S65_ABSOLUTE_X,          S65_SET_6500     },
    { "ISC!",    0xFF, S65_ABSOLUTE_X,          S65_SET_6500     },

    /* Extended (65C00 original) set */
    { "TSB",     0x04, S65_ZEROPAGE,            S65_SET_65C00    },
    { "TSB",     0x0C, S65_ABSOLUTE,            S65_SET_65C00    },
    { "ORA",     0x12, S65_ZEROPAGE_INDIRECT,   S65_SET_65C00    },
    { "TRB",     0x14, S65_ZEROPAGE,            S65_SET_65C00    },
    { "INC",     0x1A, S65_IMPLIED,             S65_SET_65C00    },
    { "TRB",     0x1C, S65_ABSOLUTE,            S65_SET_65C00    },
    { "AND",     0x32, S65_ZEROPAGE_INDIRECT,   S65_SET_65C00    },
    { "BIT",     0x34, S65_ZEROPAGE_X,          S65_SET_65C00    },
    { "DEC",     0x3A, S65_IMPLIED,             S65_SET_65C00    },
    { "BIT",     0x3C, S65_ABSOLUTE_X,          S65_SET_65C00    },
    { "EOR",     0x52, S65_ZEROPAGE_INDIRECT,   S65_SET_65C00    },
    { "PHY",     0x5A, S65_IMPLIED,             S65_SET_65C00    },
    { "STZ",     0x64, S65_ZEROPAGE,            S65_SET_65C00    },
    { "ADC",     0x72, S65_ZEROPAGE_INDIRECT,   S65_SET_65C00    },
    { "STZ",     0x74, S65_ZEROPAGE_X,          S65_SET_65C00    },
    { "PLY",     0x7A, S65_IMPLIED,             S65_SET_65C00    },
    { "JMP",     0x7C, S65_ABSOLUTE_INDIRECT_X, S65_SET_65C00    },
    { "BRA",     0x80, S65_RELATIVE,            S65_SET_65C00    },
    { "BIT",     0x89, S65_IMMEDIATE,           S65_SET_65C00    },
    { "STA",     0x92, S65_ZEROPAGE_INDIRECT,   S65_SET_65C00    },
    { "STZ",     0x9C, S65_ABSOLUTE,            S65_SET_65C00    },
    { "STZ",     0x9E, S65_ABSOLUTE_X,          S65_SET_65C00    },
    { "LDA",     0xB2, S65_ZEROPAGE_INDIRECT,   S65_SET_65C00    },
    { "CMP",     0xD2, S65_ZEROPAGE_INDIRECT,   S65_SET_65C00    },
    { "PHX",     0xDA, S65_IMPLIED,             S65_SET_65C00    },
    { "SBC",     0xF2, S65_ZEROPAGE_INDIRECT,   S65_SET_65C00    },
    { "PLX",     0xFA, S65_IMPLIED,             S65_SET_65C00    },

    /* Newest WDC65C00 set (fully defined) */
    { "RMB0",    0X07, S65_ZEROPAGE,            S65_SET_WDC65C00 },
    { "BBR0",    0X0F, S65_RELATIVE,            S65_SET_WDC65C00 },
    { "RMB1",    0X17, S65_ZEROPAGE,            S65_SET_WDC65C00 },
    { "BBR1",    0X1F, S65_RELATIVE,            S65_SET_WDC65C00 },
    { "RMB2",    0X27, S65_ZEROPAGE,            S65_SET_WDC65C00 },
    { "BBR2",    0X2F, S65_RELATIVE,            S65_SET_WDC65C00 },
    { "RMB3",    0X37, S65_ZEROPAGE,            S65_SET_WDC65C00 },
    { "BBR3",    0X3F, S65_RELATIVE,            S65_SET_WDC65C00 },
    { "RMB4",    0X47, S65_ZEROPAGE,            S65_SET_WDC65C00 },
    { "BBR4",    0X4F, S65_RELATIVE,            S65_SET_WDC65C00 },
    { "RMB5",    0X57, S65_ZEROPAGE,            S65_SET_WDC65C00 },
    { "BBR5",    0X5F, S65_RELATIVE,            S65_SET_WDC65C00 },
    { "RMB6",    0X67, S65_ZEROPAGE,            S65_SET_WDC65C00 },
    { "BBR6",    0X6F, S65_RELATIVE,            S65_SET_WDC65C00 },
    { "RMB7",    0X77, S65_ZEROPAGE,            S65_SET_WDC65C00 },
    { "BBR7",    0X7F, S65_RELATIVE,            S65_SET_WDC65C00 },
    { "SMB0",    0X87, S65_ZEROPAGE,            S65_SET_WDC65C00 },
    { "BBS0",    0X8F, S65_RELATIVE,            S65_SET_WDC65C00 },
    { "SMB1",    0X97, S65_ZEROPAGE,            S65_SET_WDC65C00 },
    { "BBS1",    0X9F, S65_RELATIVE,            S65_SET_WDC65C00 },
    { "SMB2",    0XA7, S65_ZEROPAGE,            S65_SET_WDC65C00 },
    { "BBS2",    0XAF, S65_RELATIVE,            S65_SET_WDC65C00 },
    { "SMB3",    0XB7, S65_ZEROPAGE,            S65_SET_WDC65C00 },
    { "BBS3",    0XBF, S65_RELATIVE,            S65_SET_WDC65C00 },
    { "SMB4",    0XC7, S65_ZEROPAGE,            S65_SET_WDC65C00 },
    { "WAI",     0XCB, S65_IMPLIED,             S65_SET_WDC65C00 },
    { "BBS4",    0XCF, S65_RELATIVE,            S65_SET_WDC65C00 },
    { "SMB5",    0XD7, S65_ZEROPAGE,            S65_SET_WDC65C00 },
    { "STP",     0XDB, S65_IMPLIED,             S65_SET_WDC65C00 },
    { "BBS5",    0XDF, S65_RELATIVE,            S65_SET_WDC65C00 },
    { "SMB6",    0XE7, S65_ZEROPAGE,            S65_SET_WDC65C00 },
    { "BBS6",    0XEF, S65_RELATIVE,            S65_SET_WDC65C00 },
    { "SMB7",    0XF7, S65_ZEROPAGE,            S65_SET_WDC65C00 },
    { "BBS7",    0XFF, S65_RELATIVE,            S65_SET_WDC65C00 },
    /* The rest is NOP... To be defined */
};


/* Translates an instruction.
 *
 * @param dt_memory     program memory data
 * @param wd_pc         program counter value
 * @param st_set        instruction set
 * 
 * @returns Valid instruction type or
 * NULL if failed.
 */
const instruction_t *s65_decode(const data_t *dt_memory, word wd_pc, instruction_set_t st_set)
{
    assert(dt_memory);
    assert(wd_pc < dt_memory->sz_bksize);

    /* Target instruction */
    const instruction_t *tmp_inst = NULL;
    /* The OPCODE */
    const byte tmp_opcode = dt_memory->pb_block[wd_pc];

    /* Finding the OPCODE (as 6500) */
    for(size_t i = 0; i < sizeof(gc_instructions) / sizeof(gc_instructions[0]); ++i)
    {
        if(gc_instructions[i].b_opcode == tmp_opcode)
        {
            tmp_inst = &gc_instructions[i];
            break;
        }
    }
    /* Finding the OPCODE (as 65C00) */
    for(size_t i = 0; st_set >= S65_SET_65C00 && i < sizeof(gc_instructions) / sizeof(gc_instructions[0]); ++i)
    {
        if(gc_instructions[i].b_opcode == tmp_opcode &&
            gc_instructions[i].st_set == S65_SET_65C00)
        {
            tmp_inst = &gc_instructions[i];
            break;
        }
    }
    /* Finding the OPCODE (as WDC65C00) */
    for(size_t i = 0; st_set == S65_SET_WDC65C00 && i < sizeof(gc_instructions) / sizeof(gc_instructions[0]); ++i)
    {
        if(gc_instructions[i].b_opcode == tmp_opcode &&
            gc_instructions[i].st_set == S65_SET_WDC65C00)
        {
            tmp_inst = &gc_instructions[i];
            break;
        }
    }

    return tmp_inst;
}

/* Converts an instruction into the list
 * of operations.
 *
 * @param dt_memory     program memory data
 * @param wd_pc         program counter value
 * @param st_set        instruction set
 * 
 * @returns Valid pointer to op_result or 
 * NULL if failed.
 */
op_result_t *s65_convert(const data_t *dt_memory, word wd_pc, instruction_set_t st_set)
{
    assert(dt_memory);

    /* The instruction */
    const instruction_t *tmp_instr = s65_decode(dt_memory, wd_pc, st_set);
    /* Alias */
    const char *tmp_mnem = tmp_instr->s_mnemo;
    /* The result itself */
    op_result_t *tmp_result = NULL;

    /* These timing schemes are taken from
     * Appendix A of Synertek's datasheet */

    /* A.1. 1 byte instructions */
    if(s65_instr_size(tmp_instr) == 1u &&
    (strcmp(tmp_mnem, "ASL") == 0 ||
     strcmp(tmp_mnem, "CLC") == 0 ||
     strcmp(tmp_mnem, "CLD") == 0 ||
     strcmp(tmp_mnem, "CLI") == 0 ||
     strcmp(tmp_mnem, "CLV") == 0 ||
     strcmp(tmp_mnem, "DEX") == 0 ||
     strcmp(tmp_mnem, "DEY") == 0 ||
     strcmp(tmp_mnem, "INX") == 0 ||
     strcmp(tmp_mnem, "INY") == 0 ||
     strcmp(tmp_mnem, "LSR") == 0 ||
     strcmp(tmp_mnem, "NOP") == 0 ||
     strcmp(tmp_mnem, "ROL") == 0 ||
     strcmp(tmp_mnem, "SEC") == 0 ||
     strcmp(tmp_mnem, "SED") == 0 ||
     strcmp(tmp_mnem, "SEI") == 0 ||
     strcmp(tmp_mnem, "TAX") == 0 ||
     strcmp(tmp_mnem, "TAY") == 0 ||
     strcmp(tmp_mnem, "TSX") == 0 ||
     strcmp(tmp_mnem, "TXA") == 0 ||
     strcmp(tmp_mnem, "TXS") == 0 ||
     strcmp(tmp_mnem, "TYA") == 0))
    {
        /* Two cycles, 5 operations (fetch, inc pc, fetch (discarded), execute, fetch new) */
        if((tmp_result = s65_new_opresult(5u)) == NULL)
        {
            /* Failed to allocate */
            return NULL;
        }

        /* Writing to the list */
        /* T0 */
        tmp_result->op_list[0].ot_type = S65_OP_FETCH;
        tmp_result->op_list[0].b_first = tmp_instr->b_opcode;
        tmp_result->op_list[0].d_cycle = 0;

        /* T1 */
        tmp_result->op_list[1].ot_type = S65_OP_PC_INC;
        tmp_result->op_list[1].d_cycle = 1;

        tmp_result->op_list[2].ot_type = S65_OP_FETCH;
        tmp_result->op_list[2].d_flags = S65_OPFLAG_DISCARD;
        tmp_result->op_list[2].d_cycle = 1;

        /* T2 (next fetching) */
        tmp_result->op_list[3].ot_type = S65_OP_FETCH;
        tmp_result->op_list[3].d_cycle = 0;

        /* Executing THIS instruction */
        tmp_result->op_list[4].d_cycle = 0;

        /* Shift left */
        if(strcmp(tmp_mnem, "ASL") == 0)
        {
            /* ACC shift left */
            tmp_result->op_list[4].ot_type = S65_OP_ASL;
            tmp_result->op_list[4].b_first = S65_REG_ACC;
        }
        /* Flag clear (CLn) */
        else if(strstr(tmp_mnem, "CL") == tmp_mnem)
        {
            /* Clearing n flag */
            tmp_result->op_list[4].ot_type = S65_OP_CLEAR;
            tmp_result->op_list[4].b_first = s65_char_to_sreg(tmp_mnem[2]);
        }
        /* Decrement (DEn) */
        else if(strstr(tmp_mnem, "DE") == tmp_mnem)
        {
            /* Decrementing n register */
            tmp_result->op_list[4].ot_type = S65_OP_DEC;
            tmp_result->op_list[4].b_first = s65_asci_to_reg(tmp_mnem + 2);
        }
        /* Incrementing (INn) */
        else if(strstr(tmp_mnem, "IN") == tmp_mnem)
        {
            /* Incrementing n register */
            tmp_result->op_list[4].ot_type = S65_OP_INC;
            tmp_result->op_list[4].b_first = s65_asci_to_reg(tmp_mnem + 2);
        }
        /* Shift right */
        else if(strcmp(tmp_mnem, "LSR") == 0)
        {
            /* ACC shift right */
            tmp_result->op_list[4].ot_type = S65_OP_LSR;
            tmp_result->op_list[4].b_first = S65_REG_ACC;
        }
        /* No operation */
        else if(strcmp(tmp_mnem, "NOP") == 0)
        {
            /* NOP */
            tmp_result->op_list[4].ot_type = S65_OP_NOP;
        }
    }

    return tmp_result;
}

/* Translates an operation into assembly. 
 *
 * @param op_oper       the operation
 * @param d_flags       options (S65_ASM_)
 */
char *s65_to_asm(const operation_t *op_oper, int d_flags)
{
    assert(op_oper);

#define _BUFFER_SIZE         64u

    /* The buffer to be returned */
    char *tmp_buffer = NULL;

    /* Allocation */
    if((tmp_buffer = (char *) malloc(_BUFFER_SIZE)) == NULL)
    {
        /* Failed */
        return NULL;
    }

    /* If it is OPCODE Fetch */
    if(op_oper->ot_type == S65_OP_FETCH)
    {
        snprintf(tmp_buffer, _BUFFER_SIZE, (d_flags & S65_ASM_LOWERCASE) ? "Fetch: %02x" : "Fetch: %02X", op_oper->b_first);
        goto TO_ASM_END;
    }
    /* If it is program counter incremented */
    else if(op_oper->ot_type == S65_OP_PC_INC)
    {
        snprintf(tmp_buffer, _BUFFER_SIZE, "PC <-- PC + 1");
        goto TO_ASM_END;
    }
    /* If it is NOP */
    else if(op_oper->ot_type == S65_OP_NOP)
    {
        snprintf(tmp_buffer, _BUFFER_SIZE, "No operation");
        goto TO_ASM_END;
    }
    /* If it is unknown */
    else if(op_oper->d_flags == S65_OP_UNKNOWN)
    {
        snprintf(tmp_buffer, _BUFFER_SIZE, "Unknown operation");
        goto TO_ASM_END;
    }

    /* First argument */

    /* A register */
    if(S65_IS_REGISTER(op_oper->b_first))
        snprintf(tmp_buffer, _BUFFER_SIZE, "%s ", s65_reg_to_asci(op_oper->b_first, (d_flags & S65_ASM_SHORTEN)));

    /* Not a register - physical address */
    else if(d_flags & S65_ASM_AS_HEXADECIMAL)
        snprintf(tmp_buffer, _BUFFER_SIZE, (d_flags & S65_ASM_LOWERCASE) ? "%04x" : "%04X", op_oper->b_first);
    else if(d_flags & S65_ASM_AS_OCTAL)
        snprintf(tmp_buffer, _BUFFER_SIZE, (d_flags & S65_ASM_LOWERCASE) ? "%04o" : "%04O", op_oper->b_first);
    else
        snprintf(tmp_buffer, _BUFFER_SIZE, "%04u", op_oper->b_first);

    /* Arrow operator */
    strncat(tmp_buffer, " <-- ", _BUFFER_SIZE);

    /* First, edited operand again */


    TO_ASM_END:

    /* Adding info. if discarded */
    if(op_oper->d_flags & S65_OPFLAG_DISCARD)
        strncat(tmp_buffer, "(discarded)", _BUFFER_SIZE);

    return tmp_buffer;

#undef _BUFFER_SIZE
}
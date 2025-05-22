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
    { "BRK",     0x00, S65_IMPLIED,     },
    { "ORA",     0x01, S65_INDIRECT_X,  },
    { "ORA",     0x05, S65_ZEROPAGE,    },
    { "ASL",     0x06, S65_ZEROPAGE,    },
    { "PHP",     0x08, S65_IMPLIED,     },
    { "ORA",     0x09, S65_IMMEDIATE,   },
    { "ASL",     0x0A, S65_ACCUMULATOR, },
    { "ORA",     0x0D, S65_ABSOLUTE,    },
    { "ASL",     0x0E, S65_ABSOLUTE,    },
    { "BPL",     0x10, S65_RELATIVE,    },
    { "ORA",     0x11, S65_INDIRECT_Y,  },
    { "ORA",     0x15, S65_ZEROPAGE_X,  },
    { "ASL",     0x16, S65_ZEROPAGE_X,  },
    { "CLC",     0x18, S65_IMPLIED,     },
    { "ORA",     0x19, S65_ABSOLUTE_Y,  },
    { "ORA",     0x1D, S65_ABSOLUTE_X,  },
    { "ASL",     0x1E, S65_ABSOLUTE_X,  },
    { "JSR",     0x20, S65_ABSOLUTE,    },
    { "AND",     0x21, S65_INDIRECT_X,  },
    { "BIT",     0x24, S65_ZEROPAGE,    },
    { "AND",     0x25, S65_ZEROPAGE,    },
    { "ROL",     0x26, S65_ZEROPAGE,    },
    { "PLP",     0x28, S65_IMPLIED,     },
    { "AND",     0x29, S65_IMMEDIATE,   },
    { "ROL",     0x2A, S65_ACCUMULATOR, },
    { "BIT",     0x2C, S65_ABSOLUTE,    },
    { "AND",     0x2D, S65_ABSOLUTE,    },
    { "ROL",     0x2E, S65_ABSOLUTE,    },
    { "BMI",     0x30, S65_RELATIVE,    },
    { "AND",     0x31, S65_INDIRECT_Y,  },
    { "AND",     0x35, S65_ZEROPAGE_X,  },
    { "ROL",     0x36, S65_ZEROPAGE_X,  },
    { "SEC",     0x38, S65_IMPLIED,     },
    { "AND",     0x39, S65_ABSOLUTE_Y,  },
    { "AND",     0x3D, S65_ABSOLUTE_X,  },
    { "ROL",     0x3E, S65_ABSOLUTE_X,  },
    { "RTI",     0x40, S65_IMPLIED,     },
    { "EOR",     0x41, S65_INDIRECT_X,  },
    { "EOR",     0x45, S65_ZEROPAGE,    },
    { "LSR",     0x46, S65_ZEROPAGE,    },
    { "PHA",     0x48, S65_IMPLIED,     },
    { "EOR",     0x49, S65_IMMEDIATE,   },
    { "LSR",     0x4A, S65_ACCUMULATOR, },
    { "JMP",     0x4C, S65_ABSOLUTE,    },
    { "EOR",     0x4D, S65_ABSOLUTE,    },
    { "LSR",     0x4E, S65_ABSOLUTE,    },
    { "BVC",     0x50, S65_RELATIVE,    },
    { "EOR",     0x51, S65_INDIRECT_Y,  },
    { "EOR",     0x55, S65_ZEROPAGE_X,  },
    { "LSR",     0x56, S65_ZEROPAGE_X,  },
    { "CLI",     0x58, S65_IMPLIED,     },
    { "EOR",     0x59, S65_ABSOLUTE_Y,  },
    { "EOR",     0x5D, S65_ABSOLUTE_X,  },
    { "LSR",     0x5E, S65_ABSOLUTE_X,  },
    { "RTS",     0x60, S65_IMPLIED,     },
    { "ADC",     0x61, S65_INDIRECT_X,  },
    { "ADC",     0x65, S65_ZEROPAGE,    },
    { "ROR",     0x66, S65_ZEROPAGE,    },
    { "PLA",     0x68, S65_IMPLIED,     },
    { "ADC",     0x69, S65_IMMEDIATE,   },
    { "ROR",     0x6A, S65_ACCUMULATOR, },
    { "JMP",     0x6C, S65_INDIRECT,    },
    { "ADC",     0x6D, S65_ABSOLUTE,    },
    { "ROR",     0x6E, S65_ABSOLUTE,    },
    { "BVS",     0x70, S65_RELATIVE,    },
    { "ADC",     0x71, S65_INDIRECT_Y,  },
    { "ADC",     0x75, S65_ZEROPAGE_X,  },
    { "ROR",     0x76, S65_ZEROPAGE_X,  },
    { "SEI",     0x78, S65_IMPLIED,     },
    { "ADC",     0x79, S65_ABSOLUTE_Y,  },
    { "ADC",     0x7D, S65_ABSOLUTE_X,  },
    { "ROR",     0x7E, S65_ABSOLUTE_X,  },
    { "STA",     0x81, S65_INDIRECT_X,  },
    { "STY",     0x84, S65_ZEROPAGE,    },
    { "STA",     0x85, S65_ZEROPAGE,    },
    { "STX",     0x86, S65_ZEROPAGE,    },
    { "DEY",     0x88, S65_IMPLIED,     },
    { "TXA",     0x8A, S65_IMPLIED,     },
    { "STY",     0x8C, S65_ABSOLUTE,    },
    { "STA",     0x8D, S65_ABSOLUTE,    },
    { "STX",     0x8E, S65_ABSOLUTE,    },
    { "BCC",     0x90, S65_RELATIVE,    },
    { "STA",     0x91, S65_INDIRECT_Y,  },
    { "STY",     0x94, S65_ZEROPAGE_X,  },
    { "STA",     0x95, S65_ZEROPAGE_X,  },
    { "STX",     0x96, S65_ZEROPAGE_Y,  },
    { "TYA",     0x98, S65_IMPLIED,     },
    { "STA",     0x99, S65_ABSOLUTE_Y,  },
    { "TXS",     0x9A, S65_IMPLIED,     },
    { "STA",     0x9D, S65_ABSOLUTE_X,  },
    { "LDY",     0xA0, S65_IMMEDIATE,   },
    { "LDA",     0xA1, S65_INDIRECT_X,  },
    { "LDX",     0xA2, S65_IMMEDIATE,   },
    { "LDY",     0xA4, S65_ZEROPAGE,    },
    { "LDA",     0xA5, S65_ZEROPAGE,    },
    { "LDX",     0xA6, S65_ZEROPAGE,    },
    { "TAY",     0xA8, S65_IMPLIED,     },
    { "LDA",     0xA9, S65_IMMEDIATE,   },
    { "TAX",     0xAA, S65_IMPLIED,     },
    { "LDY",     0xAC, S65_ABSOLUTE,    },
    { "LDA",     0xAD, S65_ABSOLUTE,    },
    { "LDX",     0xAE, S65_ABSOLUTE,    },
    { "BCS",     0xB0, S65_RELATIVE,    },
    { "LDA",     0xB1, S65_INDIRECT_Y,  },
    { "LDY",     0xB4, S65_ZEROPAGE_X,  },
    { "LDA",     0xB5, S65_ZEROPAGE_X,  },
    { "LDX",     0xB6, S65_ZEROPAGE_Y,  },
    { "CLV",     0xB8, S65_IMPLIED,     },
    { "LDA",     0xB9, S65_ABSOLUTE_Y,  },
    { "TSX",     0xBA, S65_IMPLIED,     },
    { "LDY",     0xBC, S65_ABSOLUTE_X,  },
    { "LDA",     0xBD, S65_ABSOLUTE_X,  },
    { "LDX",     0xBE, S65_ABSOLUTE_Y,  },
    { "CPY",     0xC0, S65_IMMEDIATE,   },
    { "CMP",     0xC1, S65_INDIRECT_X,  },
    { "CPY",     0xC4, S65_ZEROPAGE,    },
    { "CMP",     0xC5, S65_ZEROPAGE,    },
    { "DEC",     0xC6, S65_ZEROPAGE,    },
    { "INY",     0xC8, S65_IMPLIED,     },
    { "CMP",     0xC9, S65_IMMEDIATE,   },
    { "DEX",     0xCA, S65_IMPLIED,     },
    { "CPY",     0xCC, S65_ABSOLUTE,    },
    { "CMP",     0xCD, S65_ABSOLUTE,    },
    { "DEC",     0xCE, S65_ABSOLUTE,    },
    { "BNE",     0xD0, S65_RELATIVE,    },
    { "CMP",     0xD1, S65_INDIRECT_Y,  },
    { "CMP",     0xD5, S65_ZEROPAGE_X,  },
    { "DEC",     0xD6, S65_ZEROPAGE_X,  },
    { "CLD",     0xD8, S65_IMPLIED,     },
    { "CMP",     0xD9, S65_ABSOLUTE_Y,  },
    { "CMP",     0xDD, S65_ABSOLUTE_X,  },
    { "DEC",     0xDE, S65_ABSOLUTE_X,  },
    { "CPX",     0xE0, S65_IMMEDIATE,   },
    { "SBC",     0xE1, S65_INDIRECT_X,  },
    { "CPX",     0xE4, S65_ZEROPAGE,    },
    { "SBC",     0xE5, S65_ZEROPAGE,    },
    { "INC",     0xE6, S65_ZEROPAGE,    },
    { "INX",     0xE8, S65_IMPLIED,     },
    { "SBC",     0xE9, S65_IMMEDIATE,   },
    { "NOP",     0xEA, S65_IMPLIED,     },
    { "CPX",     0xEC, S65_ABSOLUTE,    },
    { "SBC",     0xED, S65_ABSOLUTE,    },
    { "INC",     0xEE, S65_ABSOLUTE,    },
    { "BEQ",     0xF0, S65_RELATIVE,    },
    { "SBC",     0xF1, S65_INDIRECT_Y,  },
    { "SBC",     0xF5, S65_ZEROPAGE_X,  },
    { "INC",     0xF6, S65_ZEROPAGE_X,  },
    { "SED",     0xF8, S65_IMPLIED,     },
    { "SBC",     0xF9, S65_ABSOLUTE_Y,  },
    { "SBC",     0xFD, S65_ABSOLUTE_X,  },
    { "INC",     0xFE, S65_ABSOLUTE_X,  },

    /* Undocumented original 6500 set */
    { "JAM!",    0x02, S65_IMPLIED,     },
    { "SLO!",    0x03, S65_INDIRECT_X,  },
    { "NOP!",    0x04, S65_ZEROPAGE,    },
    { "SLO!",    0x07, S65_ZEROPAGE,    },
    { "ANC!",    0x0B, S65_IMMEDIATE,   },
    { "NOP!",    0x0C, S65_ABSOLUTE,    },
    { "SLO!",    0x0F, S65_ABSOLUTE,    },
    { "JAM!",    0x12, S65_IMPLIED,     },
    { "SLO!",    0x13, S65_INDIRECT_Y,  },
    { "NOP!",    0x14, S65_ZEROPAGE_X,  },
    { "SLO!",    0x17, S65_ZEROPAGE_X,  },
    { "NOP!",    0x1A, S65_IMPLIED,     },
    { "SLO!",    0x1B, S65_ABSOLUTE_Y,  },
    { "NOP!",    0x1C, S65_ABSOLUTE_X,  },
    { "SLO!",    0x1F, S65_ABSOLUTE_X,  },
    { "JAM!",    0x22, S65_IMPLIED,     },
    { "RLA!",    0x23, S65_INDIRECT_X,  },
    { "RLA!",    0x27, S65_ZEROPAGE,    },
    { "ANC!",    0x2B, S65_IMMEDIATE,   },
    { "RLA!",    0x2F, S65_ABSOLUTE,    },
    { "JAM!",    0x32, S65_IMPLIED,     },
    { "RLA!",    0x33, S65_INDIRECT_Y,  },
    { "NOP!",    0x34, S65_ZEROPAGE_X,  },
    { "RLA!",    0x37, S65_ZEROPAGE_X,  },
    { "NOP!",    0x3A, S65_IMPLIED,     },
    { "RLA!",    0x3B, S65_ABSOLUTE_Y,  },
    { "NOP!",    0x3C, S65_ABSOLUTE_X,  },
    { "RLA!",    0x3F, S65_ABSOLUTE_X,  },
    { "JAM!",    0x42, S65_IMPLIED,     },
    { "SRE!",    0x43, S65_INDIRECT_X,  },
    { "NOP!",    0x44, S65_ZEROPAGE,    },
    { "SRE!",    0x47, S65_ZEROPAGE,    },
    { "ALR!",    0x4B, S65_IMMEDIATE,   },
    { "SRE!",    0x4F, S65_ABSOLUTE,    },
    { "JAM!",    0x52, S65_IMPLIED,     },
    { "SRE!",    0x53, S65_INDIRECT_Y,  },
    { "NOP!",    0x54, S65_ZEROPAGE_X,  },
    { "SRE!",    0x57, S65_ZEROPAGE_X,  },
    { "NOP!",    0x5A, S65_IMPLIED,     },
    { "SRE!",    0x5B, S65_ABSOLUTE_Y,  },
    { "NOP!",    0x5C, S65_ABSOLUTE_X,  },
    { "SRE!",    0x5F, S65_ABSOLUTE_X,  },
    { "JAM!",    0x62, S65_IMPLIED,     },
    { "RRA!",    0x63, S65_INDIRECT_X,  },
    { "NOP!",    0x64, S65_ZEROPAGE,    },
    { "RRA!",    0x67, S65_ZEROPAGE,    },
    { "ARR!",    0x6B, S65_IMMEDIATE,   },
    { "RRA!",    0x6F, S65_ABSOLUTE,    },
    { "JAM!",    0x72, S65_IMPLIED,     },
    { "RRA!",    0x73, S65_INDIRECT_Y,  },
    { "NOP!",    0x74, S65_ZEROPAGE_X,  },
    { "RRA!",    0x77, S65_ZEROPAGE_X,  },
    { "NOP!",    0x7A, S65_IMPLIED,     },
    { "RRA!",    0x7B, S65_ABSOLUTE_Y,  },
    { "NOP!",    0x7C, S65_ABSOLUTE_X,  },
    { "RRA!",    0x7F, S65_ABSOLUTE_X,  },
    { "NOP!",    0x80, S65_IMMEDIATE,   },
    { "NOP!",    0x82, S65_IMMEDIATE,   },
    { "SAX!",    0x83, S65_INDIRECT_X,  },
    { "SAX!",    0x87, S65_ZEROPAGE,    },
    { "NOP!",    0x89, S65_IMMEDIATE,   },
    { "ANE!",    0x8B, S65_IMMEDIATE,   },
    { "SAX!",    0x8F, S65_ABSOLUTE,    },
    { "JAM!",    0x92, S65_IMPLIED,     },
    { "SHA!",    0x93, S65_INDIRECT_Y,  },
    { "SAX!",    0x97, S65_ZEROPAGE_Y,  },
    { "TAS!",    0x9B, S65_ABSOLUTE_Y,  },
    { "SHY!",    0x9C, S65_ABSOLUTE_X,  },
    { "SHX!",    0x9E, S65_ABSOLUTE_Y,  },
    { "SHA!",    0x9F, S65_ABSOLUTE_Y,  },
    { "LAX!",    0xA3, S65_INDIRECT_X,  },
    { "LAX!",    0xA7, S65_ZEROPAGE,    },
    { "LXA!",    0xAB, S65_IMMEDIATE,   },
    { "LAX!",    0xAF, S65_ABSOLUTE,    },
    { "JAM!",    0xB2, S65_IMPLIED,     },
    { "LAX!",    0xB3, S65_INDIRECT_Y,  },
    { "LAX!",    0xB7, S65_ZEROPAGE_Y,  },
    { "LAS!",    0xBB, S65_ABSOLUTE_Y,  },
    { "LAX!",    0xBF, S65_ABSOLUTE_Y,  },
    { "NOP!",    0xC2, S65_IMMEDIATE,   },
    { "DCP!",    0xC3, S65_INDIRECT_X,  },
    { "DCP!",    0xC7, S65_ZEROPAGE,    },
    { "SBX!",    0xCB, S65_IMMEDIATE,   },
    { "DCP!",    0xCF, S65_ABSOLUTE,    },
    { "JAM!",    0xD2, S65_IMPLIED,     },
    { "DCP!",    0xD3, S65_INDIRECT_Y,  },
    { "NOP!",    0xD4, S65_ZEROPAGE_X,  },
    { "DCP!",    0xD7, S65_ZEROPAGE_X,  },
    { "NOP!",    0xDA, S65_IMPLIED,     },
    { "DCP!",    0xDB, S65_ABSOLUTE_Y,  },
    { "NOP!",    0xDC, S65_ABSOLUTE_X,  },
    { "DCP!",    0xDF, S65_ABSOLUTE_X,  },
    { "NOP!",    0xE2, S65_IMMEDIATE,   },
    { "ISC!",    0xE3, S65_INDIRECT_X,  },
    { "ISC!",    0xE7, S65_ZEROPAGE,    },
    { "SBC!",    0xEB, S65_IMMEDIATE,   },
    { "ISC!",    0xEF, S65_ABSOLUTE,    },
    { "JAM!",    0xF2, S65_IMPLIED,     },
    { "ISC!",    0xF3, S65_INDIRECT_Y,  },
    { "NOP!",    0xF4, S65_ZEROPAGE_X,  },
    { "ISC!",    0xF7, S65_ZEROPAGE_X,  },
    { "NOP!",    0xFA, S65_IMPLIED,     },
    { "ISC!",    0xFB, S65_ABSOLUTE_Y,  },
    { "NOP!",    0xFC, S65_ABSOLUTE_X,  },
    { "ISC!",    0xFF, S65_ABSOLUTE_X,  }
};


/* Adds new operation at the end of a
 * result list.
 *
 * @param rop_dest      the struct to be edited
 * @param ot_type       operation type (S65_OP_*)
 * @param ad_first      first operand address
 * @param ad_secnd      second operand address
 * @param d_cycle       cycle number 
 * @param d_flags       flags (S65_OPFLAG_*)
 *
 * @returns The dest if succeeded, 
 * NULL otherwise.
 */
op_result_t *s65_op_add(op_result_t *rop_dest, optype_t ot_type, addr_t ad_first, addr_t ad_secnd, int d_cycle, int d_flags)
{
    assert(rop_dest);

    /* Trying to reallocate */
    operation_t *tmp_ptr = (operation_t *) realloc(rop_dest->op_list, (rop_dest->sz_count + 1u) * sizeof(operation_t));
    if(tmp_ptr == NULL)
    {
        /* Failed */ 
        return NULL;
    }

    /* Reassigning the pointer */
    rop_dest->op_list = tmp_ptr;

    /* Copying */
    /* Alias */
    operation_t *tmp_op = &(rop_dest->op_list[rop_dest->sz_count]);
    {
        tmp_op->ot_type = ot_type;
        tmp_op->ad_first = ad_first;
        tmp_op->ad_secnd = ad_secnd;
        tmp_op->d_cycle = d_cycle;
        tmp_op->d_flags = d_flags;
    }

    /* Incrementing the size member */
    ++(rop_dest->sz_count);
    return rop_dest;
}

/* Translates an instruction.
 *
 * @param b_opcode      the OPCODE
 * 
 * @returns Valid instruction type or
 * NULL if failed.
 */
const instruction_t *s65_decode(byte b_opcode)
{
    /* Target instruction */
    const instruction_t *tmp_inst = NULL;

    /* Finding the OPCODE (as 6500) */
    for(size_t i = 0; i < sizeof(gc_instructions) / sizeof(gc_instructions[0]); ++i)
    {
        if(gc_instructions[i].b_opcode == b_opcode)
        {
            tmp_inst = &gc_instructions[i];
            break;
        }
    }

    return tmp_inst;
}

/* Converts an instruction into a list
 * of operations.
 *
 * @param b_opcode      the OPCODE
 * 
 * @returns Valid pointer to op_result or 
 * NULL if failed.
 */
op_result_t *s65_convert(byte b_opcode)
{
    /* The instruction */
    const instruction_t *instr = s65_decode(b_opcode);

    /* The result */
    op_result_t *rop_result = s65_new_opresult(0u);

    /* Cycle counter */
    int d_cycle = 1;

    /************************************** 
     * Legal 6502 instruction timings are 
     * based on SY6502 datasheet - APPENDIX A.
     * Other instructions are based on
     * own observations.
     **************************************/

    /* Checking allocation */
    if(rop_result == NULL)
    {
        /* Fail */
        return NULL;
    }

    /* Operations common for all */

    /* Setting address bus to PC*/
    s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    0      , S65_OPFLAG_READ                        );
    s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    0      , S65_OPFLAG_READ                        );
    /* Fetching OPCODE */
    s65_op_add(rop_result, S65_OP_FETCH,        S65_REG_DATA,   S65_REG_NULL,                   0      , S65_OPFLAG_READ                        );

    /* Skipping, if illegal */
    if(S65_IS_ILLEGAL(instr->s_mnemo))
        goto ILLEGAL;

    /* JAM - CPU jamming */
    if(strcmpi(instr->s_mnemo, "JAM") == 0)
    {
        /* [T1] */

        /* Incrementing PC */
        s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
        /* Setting address bus to PC and discarding */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
        /* Putting unknown operation */
        s65_op_add(rop_result, S65_OP_UNKNOWN,      S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
        /* The end */
    }

    /* BRK - hardware interrupt */
    else if(strcmpi(instr->s_mnemo, "BRK") == 0) 
    {
        /* [T1] */

        /* Incrementing PC */
        s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
        /* Setting address bus to PC and discarding */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );

        /* [T2] */ d_cycle++;

        /* Incrementing PC again (exclusive for BRK) */
        s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_WRITE                       );
        /* Setting address bus to SP */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_SP,                     d_cycle, S65_OPFLAG_WRITE                       );
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x01,                           d_cycle, S65_OPFLAG_WRITE                       );
        /* Sending (pushing) PCH */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_DATA,   S65_REG_PCH,                    d_cycle, S65_OPFLAG_WRITE                       );

        /* [T3] */ d_cycle++;

        /* Decrementing SP */
        s65_op_add(rop_result, S65_OP_DEC,          S65_REG_SP,     S65_REG_NULL,                   d_cycle, S65_OPFLAG_WRITE                       );
        /* Setting address bus to SP */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_SP,                     d_cycle, S65_OPFLAG_WRITE                       );
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x01,                           d_cycle, S65_OPFLAG_WRITE                       );
        /* Sending (pushing) PCL */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_DATA,   S65_REG_PCL,                    d_cycle, S65_OPFLAG_WRITE                       );

        /* [T4] */ d_cycle++;

        /* Decrementing SP */
        s65_op_add(rop_result, S65_OP_DEC,          S65_REG_SP,     S65_REG_NULL,                   d_cycle, S65_OPFLAG_WRITE                       );
        /* Setting address bus to SP */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_SP,                     d_cycle, S65_OPFLAG_WRITE                       );
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x01,                           d_cycle, S65_OPFLAG_WRITE                       );
        /* Setting B bit flag */
        s65_op_add(rop_result, S65_OP_SET,          S65_REG_SREG,   S65_SREG_B,                     d_cycle, S65_OPFLAG_WRITE                       ); 
        /* Sending (pushing) SREG with B set */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_DATA,   S65_REG_SREG,                   d_cycle, S65_OPFLAG_WRITE                       );
        /* Clearing B bit flag */
        s65_op_add(rop_result, S65_OP_CLEAR,        S65_REG_SREG,   S65_SREG_B,                     d_cycle, S65_OPFLAG_WRITE                       ); 

        /* [T5] */ d_cycle++;

        /* Setting address high bus to BRK vector content */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_LOW(S65_VECTOR_BRK),        d_cycle, S65_OPFLAG_READ                        );
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_HIGH(S65_VECTOR_BRK),       d_cycle, S65_OPFLAG_READ                        );
        /* Reading new address low */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

        /* [T6] */ d_cycle++;
        
        /* Setting address bus to (BRK vector + 1) content */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_LOW(S65_VECTOR_BRK + 1u),   d_cycle, S65_OPFLAG_READ                        );
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_HIGH(S65_VECTOR_BRK + 1u),  d_cycle, S65_OPFLAG_READ                        );
        /* Reading new address high */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADH,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );   
        /* Setting I bit flag */
        s65_op_add(rop_result, S65_OP_SET,          S65_REG_SREG,   S65_SREG_I,                     d_cycle, S65_OPFLAG_READ                        );                    

        /* [T0] */

        /* Updating PC */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_PCL,    S65_REG_ADL,                    0,       S65_OPFLAG_READ                        ); 
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_PCH,    S65_REG_ADH,                    0,       S65_OPFLAG_READ                        ); 

        /* Next instruction will be executed from the new address */
    }

    /* RTI - return from interrupt */
    else if(strcmpi(instr->s_mnemo, "RTI") == 0)
    {
        /* [T1] */

        /* Incrementing PC */
        s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
        /* Setting address bus to PC and discarding */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );

        /* [T2] */ d_cycle++;

        /* Setting address to SP and discarding */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_SP,                     d_cycle, S65_OPFLAG_READ                        );
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x01,                           d_cycle, S65_OPFLAG_READ                        );

        /* [T3] */ d_cycle++;

        /* Incrementing SP */
        s65_op_add(rop_result, S65_OP_INC,          S65_REG_SP,     S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
        /* Setting address bus to SP */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_SP,                     d_cycle, S65_OPFLAG_READ                        );
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x01,                           d_cycle, S65_OPFLAG_READ                        );
        /* Reading (pulling) SREG */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_SREG,   S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

        /* [T4] */ d_cycle++;

        /* Incrementing SP */
        s65_op_add(rop_result, S65_OP_INC,          S65_REG_SP,     S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
        /* Setting address bus to SP */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_SP,                     d_cycle, S65_OPFLAG_READ                        );
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x01,                           d_cycle, S65_OPFLAG_READ                        );
        /* Reading (pulling) PCL */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_PCL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

        /* [T5] */ d_cycle++;

        /* Incrementing SP */
        s65_op_add(rop_result, S65_OP_INC,          S65_REG_SP,     S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
        /* Setting address bus to SP */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_SP,                     d_cycle, S65_OPFLAG_READ                        );
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x01,                           d_cycle, S65_OPFLAG_READ                        );
        /* Reading (pulling) PCH */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_PCH,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

        /* Next instruction will be executed from the pulled address */
    }

    /* RTS - return from subroutine */
    else if(strcmpi(instr->s_mnemo, "RTS") == 0)
    {
        /* [T1] */

        /* Incrementing PC */
        s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
        /* Setting address bus to PC and discarding */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );

        /* [T2] */ d_cycle++;

        /* Setting address to SP and discarding */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_SP,                     d_cycle, S65_OPFLAG_READ                        );
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x01,                           d_cycle, S65_OPFLAG_READ                        );

        /* [T3] */ d_cycle++;

        /* Incrementing SP */
        s65_op_add(rop_result, S65_OP_INC,          S65_REG_SP,     S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
        /* Setting address bus to SP */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_SP,                     d_cycle, S65_OPFLAG_READ                        );
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x01,                           d_cycle, S65_OPFLAG_READ                        );
        /* Reading (pulling) PCL */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_PCL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

        /* [T4] */ d_cycle++;

        /* Incrementing SP */
        s65_op_add(rop_result, S65_OP_INC,          S65_REG_SP,     S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
        /* Setting address bus to SP */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_SP,                     d_cycle, S65_OPFLAG_READ                        );
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x01,                           d_cycle, S65_OPFLAG_READ                        );
        /* Reading (pulling) PCH */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_PCH,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

        /* [T5] */ d_cycle++;

        /* Setting address bus to PC and discarding */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );

        /* [T0] */

        /* Incrementing PC */
        s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   0,       S65_OPFLAG_READ                        );

        /* Next instruction */
    }

    /* JSR - jump to subroutine */
    else if(strcmpi(instr->s_mnemo, "JSR") == 0)
    {
        /* [T1] */

        /* Incrementing PC */
        s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
        /* Setting address bus to PC */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
        /* Reading address low */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

        /* [T2] */ d_cycle++;

        /* Setting address to SP and discarding */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_SP,                     d_cycle, S65_OPFLAG_READ                        );
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x01,                           d_cycle, S65_OPFLAG_READ                        );

        /* [T3] */ d_cycle++;

        /* Sending (pushing) PCH */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_DATA,   S65_REG_PCH,                    d_cycle, S65_OPFLAG_WRITE                       );

        /* [T4] */ d_cycle++;

        /* Decrementing SP */
        s65_op_add(rop_result, S65_OP_DEC,          S65_REG_SP,     S65_REG_NULL,                   d_cycle, S65_OPFLAG_WRITE                       );
        /* Setting address bus to SP */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_SP,                     d_cycle, S65_OPFLAG_WRITE                       );
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x01,                           d_cycle, S65_OPFLAG_WRITE                       );
        /* Sending (pushing) PCL */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_DATA,   S65_REG_PCL,                    d_cycle, S65_OPFLAG_WRITE                       );

        /* [T5] */ d_cycle++;

        /* Incrementing PC */
        s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
        /* Setting address bus to PC */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
        /* Reading address high */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADH,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );
        /* Decrementing SP */
        s65_op_add(rop_result, S65_OP_DEC,          S65_REG_SP,     S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );

        /* [T0] */

        /* Updating PC */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_PCL,    S65_REG_ADL,                    0,       S65_OPFLAG_READ                        ); 
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_PCH,    S65_REG_ADH,                    0,       S65_OPFLAG_READ                        ); 

        /* Next instruction will be executed from the new address */

    }

    /* JMP - jump to location */
    else if(strcmpi(instr->s_mnemo, "JMP") == 0)
    {
        /* Absolute addressing */
        if(instr->am_mode == S65_ABSOLUTE)
        {
            /* [T1] */

            /* Incrementing PC */
            s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Reading address low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T2] */ d_cycle++;

            /* Incrementing PC */
            s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Reading address high */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADH,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T0] */

            /* Updating PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_PCL,    S65_REG_ADL,                    0,       S65_OPFLAG_READ                        ); 
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_PCH,    S65_REG_ADH,                    0,       S65_OPFLAG_READ                        ); 

            /* Next instruction will be executed from the new address */
        }

        /* Indirect adressing */
        else if(instr->am_mode == S65_INDIRECT)
        {
            /* [T1] */

            /* Incrementing PC */
            s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Reading indirect address low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T2] */ d_cycle++;

            /* Incrementing PC */
            s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Reading indirect address high */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADH,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T3] */ d_cycle++;

            /* Setting address bus to indirect address */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_ADH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Reading address low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T4] */ d_cycle++;

            /* Incrementing bus address low (JMP bug here) */
            s65_op_add(rop_result, S65_OP_INC,          S65_REG_ABL,    S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
            /* Reading address high */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADH,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T0] */

            /* Updating PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_PCL,    S65_REG_ADL,                    0,       S65_OPFLAG_READ                        ); 
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_PCH,    S65_REG_ADH,                    0,       S65_OPFLAG_READ                        ); 

            /* Next instruction will be executed from the new address */
        }
    }

    /* PHP, PHA, PHX, PHY - push into the stack */
    else if(strcmpi(instr->s_mnemo, "PHP") == 0 ||
            strcmpi(instr->s_mnemo, "PHA") == 0 ||
            strcmpi(instr->s_mnemo, "PHX") == 0 ||
            strcmpi(instr->s_mnemo, "PHY") == 0
        )
    {
        /* Obtaining the register (last letter) */
        const addr_t tmp_reg = s65_asci_to_reg(instr->s_mnemo + 2);

        /* [T1] */

        /* Incrementing PC */
        s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
        /* Setting address bus to PC and discarding */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );

        /* [T2] */ d_cycle++;

        /* Setting address bus to SP */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_SP,                     d_cycle, S65_OPFLAG_WRITE                       );
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x01,                           d_cycle, S65_OPFLAG_WRITE                       );
        /* Sending (pushing) appropiate register */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_DATA,   tmp_reg,                        d_cycle, S65_OPFLAG_WRITE                       );

        /* [T0] */

        /* Incrementing PC */
        s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   0,       S65_OPFLAG_READ                        );
        /* Decrementing SP */
        s65_op_add(rop_result, S65_OP_DEC,          S65_REG_SP,     S65_REG_NULL,                   0,       S65_OPFLAG_READ                        );
        
        /* New instruction starts here */
    }

    /* PLP, PLA, PLX, PLY - pull from the stack */
    else if(strcmpi(instr->s_mnemo, "PLP") == 0 ||
            strcmpi(instr->s_mnemo, "PLA") == 0 ||
            strcmpi(instr->s_mnemo, "PLX") == 0 ||
            strcmpi(instr->s_mnemo, "PLY") == 0
        )
    {
        /* Obtaining the register */
        const addr_t tmp_reg = s65_asci_to_reg(instr->s_mnemo + 2);

        /* [T1] */

        /* Incrementing PC */
        s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
        /* Setting address bus to PC and discarding */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );

        /* [T2] */ d_cycle++;

        /* Setting address bus to SP and discarding */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_SP,                     d_cycle, S65_OPFLAG_READ                        );
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x01,                           d_cycle, S65_OPFLAG_READ                        );

        /* [T3] */ d_cycle++;

        /* Incrementing SP */
        s65_op_add(rop_result, S65_OP_INC,          S65_REG_SP,     S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
        /* Setting address bus to SP */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_SP,                     d_cycle, S65_OPFLAG_READ                        );
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x01,                           d_cycle, S65_OPFLAG_READ                        );
        /* Reading (pulling) appropiate register */
        s65_op_add(rop_result, S65_OP_LOAD,         tmp_reg,        S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

        /* [T0] */

        /* Incrementing PC */
        s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   0,       S65_OPFLAG_READ                        );
        
        /* New instruction starts here */
    }

    /* STA, STX, STY, STZ - store value at memory */
    else if(strcmpi(instr->s_mnemo, "STA") == 0 ||
            strcmpi(instr->s_mnemo, "STX") == 0 ||
            strcmpi(instr->s_mnemo, "STY") == 0 ||
            strcmpi(instr->s_mnemo, "STZ") == 0
        )
    {
        /* Obtaining source value */
        /* The value */
        addr_t ad_src;
        {
            /* Z - zero constant */
            if(instr->s_mnemo[2] == 'Z')
            {
                ad_src = 0x00;
            }
            /* A, X, Y - source registers */
            else
            {
                ad_src = s65_asci_to_reg(instr->s_mnemo + 2);
            }
        }

        /* Zero page addressing (3.1) */
        if(instr->am_mode == S65_ZEROPAGE)
        {
            /* [T1] */

            /* Incrementing PC */
            s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Reading effective address low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T2] */ d_cycle++;

            /* Setting address bus to effective address low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_WRITE                       );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x00,                           d_cycle, S65_OPFLAG_WRITE                       );
            /* Writing the value to memory */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_DATA,   ad_src,                         d_cycle, S65_OPFLAG_WRITE                       );
        }

        /* Absolute addressing (3.2) */
        else if(instr->am_mode == S65_ABSOLUTE)
        {
            /* [T1] */

            /* Incrementing PC */
            s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Reading effective address low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T2] */ d_cycle++;

            /* Incrementing PC */
            s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Reading effective address high */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADH,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T3] */ d_cycle++;

            /* Setting address bus to effective address */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_WRITE                       );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_ADH,                    d_cycle, S65_OPFLAG_WRITE                       );
            /* Writing the value to memory */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_DATA,   ad_src,                         d_cycle, S65_OPFLAG_WRITE                       );

        }
    
        /* Indirect, X addressing (3.3) */
        else if(instr->am_mode == S65_INDIRECT_X)
        {
            /* [T1] */

            /* Incrementing PC */
            s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Reading base address low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T2] */ d_cycle++;

            /* Setting address bus to base address low and discarding */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x00,                           d_cycle, S65_OPFLAG_READ                        );

            /* [T3] */ d_cycle++;
            
            /* Adding X without C' to address bus low */
            s65_op_add(rop_result, S65_OP_ADD,          S65_REG_ABL,    S65_REG_X,                      d_cycle, S65_OPFLAG_READ                        );
            /* Reading effective addres low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T4] */ d_cycle++;

            /* Adding 1 without C' to address bus low */
            s65_op_add(rop_result, S65_OP_ADD,          S65_REG_ABL,    0x01,                           d_cycle, S65_OPFLAG_READ                        );
            /* Reading effective addres high */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADH,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T5] */ d_cycle++;

            /* Setting address bus to effective address */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_WRITE                       );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_ADH,                    d_cycle, S65_OPFLAG_WRITE                       );
            /* Writing the value to memory */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_DATA,   ad_src,                         d_cycle, S65_OPFLAG_WRITE                       );

        }
    
        /* Absolute, X or Absolute, Y addressing (3.4) */
        else if(instr->am_mode == S65_ABSOLUTE_X ||
                instr->am_mode == S65_ABSOLUTE_Y)
        {
            /* Obtaining index register */
            const addr_t ad_idx = (instr->am_mode == S65_ABSOLUTE_X) ? S65_REG_X : S65_REG_Y;

            /* [T1] */

            /* Incrementing PC */
            s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Reading base address low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T2] */ d_cycle++;

            /* Incrementing PC */
            s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Reading base address high */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADH,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T3] */ d_cycle++;

            /* Adding index register without C to base address low */
            s65_op_add(rop_result, S65_OP_ADD,          S65_REG_ADL,    ad_idx,                         d_cycle, S65_OPFLAG_READ                        );
            /* Adding C to base address high */
            s65_op_add(rop_result, S65_OP_ADC,          S65_REG_ADH,    0x00,                           d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to calculated address and discarding */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_ADH,                    d_cycle, S65_OPFLAG_READ                        );
            
            /* [T4] */ d_cycle++;

            /* Writing the value to memory */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_DATA,   ad_src,                         d_cycle, S65_OPFLAG_WRITE                       );

        }

        /* Zero page, X or Zero page, Y addressing (3.5) */
        else if(instr->am_mode == S65_ZEROPAGE_X ||
                instr->am_mode == S65_ZEROPAGE_Y)
        {
            /* Obtaining index register */
            const addr_t ad_idx = (instr->am_mode == S65_ZEROPAGE_X) ? S65_REG_X : S65_REG_Y;

            /* [T1] */

            /* Incrementing PC */
            s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Reading base address low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T2] */ d_cycle++;

            /* Setting address bus to base address low and discarding */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x00,                           d_cycle, S65_OPFLAG_READ                        );
            
            /* [T3] */ d_cycle++;

            /* Adding index register without C' to base address low */
            s65_op_add(rop_result, S65_OP_ADD,          S65_REG_ADL,    ad_idx,                         d_cycle, S65_OPFLAG_WRITE                       );
            /* Setting address bus to calculated address low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_WRITE                       );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x00,                           d_cycle, S65_OPFLAG_WRITE                       );
            /* Writing the value to memory */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_DATA,   ad_src,                         d_cycle, S65_OPFLAG_WRITE                       );

        }

        /* Indirect, Y addressing (3.6) */
        else if(instr->am_mode == S65_INDIRECT_Y)
        {
            /* [T1] */

            /* Incrementing PC */
            s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Reading indirect address low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T2] */ d_cycle++;

            /* Setting address bus to indirect address low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x00,                           d_cycle, S65_OPFLAG_READ                        );
            /* Reading base address low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T3] */ d_cycle++;
            
            /* Adding 1 without C to address bus low */
            s65_op_add(rop_result, S65_OP_ADD,          S65_REG_ABL,    0x01,                           d_cycle, S65_OPFLAG_READ                        );
            /* Reading base addres high */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADH,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T4] */ d_cycle++;

            /* Adding Y without C to address low */
            s65_op_add(rop_result, S65_OP_ADD,          S65_REG_ADL,    S65_REG_Y,                      d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to calculated address and discarding */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_ADH,                    d_cycle, S65_OPFLAG_READ                        );

            /* [T5] */ d_cycle++;

            /* Writing the value to memory */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_DATA,   ad_src,                         d_cycle, S65_OPFLAG_WRITE                       );

        }

        /* Invalid addressing mode */
        else
        {
            assert("This should not have happened.");
        }
        
        /* [T0] */

        /* Incrementing PC */
        s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   0,       S65_OPFLAG_READ                        );

        /* Next instruction */
    }

    /* Branching (8 for 6500, 9 for 65C00) */
    else if(instr->am_mode == S65_RELATIVE)
    {
        optype_t opt_cond = S65_OP_UNKNOWN;
        byte     b_flag = (byte) -1;

        /* The condition (negative, zero, positive etc.) 
         * is selected based on the mnemonic. 
         */

        /* C == 0  */
        if(strcmpi(instr->s_mnemo, "BCC") == 0)
        {
            opt_cond = S65_OP_BIT_SREG_NOT;
            b_flag = S65_SREG_C;
        }

        /* C == 1  */
        else if(strcmpi(instr->s_mnemo, "BCS") == 0)
        {
            opt_cond = S65_OP_BIT_SREG;
            b_flag = S65_SREG_C;
        }

        /* Z == 1  */
        else if(strcmpi(instr->s_mnemo, "BEQ") == 0)
        {
            opt_cond = S65_OP_BIT_SREG;
            b_flag = S65_SREG_Z;
        }

        /* N == 1  */
        else if(strcmpi(instr->s_mnemo, "BMI") == 0)
        {
            opt_cond = S65_OP_BIT_SREG;
            b_flag = S65_SREG_N;
        }

        /* Z == 0  */
        else if(strcmpi(instr->s_mnemo, "BNE") == 0)
        {
            opt_cond = S65_OP_BIT_SREG_NOT;
            b_flag = S65_SREG_Z;
        }

        /* N == 0  */
        else if(strcmpi(instr->s_mnemo, "BPL") == 0)
        {
            opt_cond = S65_OP_BIT_SREG_NOT;
            b_flag = S65_SREG_N;
        }

        /* Always */
        else if(strcmpi(instr->s_mnemo, "BRA") == 0)
        {
            /* No condition */
            opt_cond = S65_OP_NOP;
            b_flag = 0xFF;
        }

        /* V == 0  */
        else if(strcmpi(instr->s_mnemo, "BVC") == 0)
        {
            opt_cond = S65_OP_BIT_SREG_NOT;
            b_flag = S65_SREG_V;
        }

        /* V == 1  */
        else if(strcmpi(instr->s_mnemo, "BVS") == 0)
        {
            opt_cond = S65_OP_BIT_SREG;
            b_flag = S65_SREG_V;
        }

        /* Invalid */
        else
        {
            assert("This should not have happened.");
        }


        /* [T1] */

        /* Incrementing PC */
        s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
        /* Setting address bus to PC */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        ); 
        /* Now DATA contains the offset */
        /* Checking the condition */
        s65_op_add(rop_result, opt_cond,            b_flag,         S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );

        /* [T2] */ d_cycle++;
        /* Bypassed if condition not met */
        
        /* Incrementing PC */
        s65_op_add(rop_result, S65_OP_PC_INC,      S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_COND   );
        /* Adding U2 number (the offset) without C' to PC */
        s65_op_add(rop_result, S65_OP_AD2,         S65_REG_PCL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_COND   );                  

        /* [T3] */ d_cycle++;
        /* Bypassed if page not crossed */

        /* Adding C' to PC if crossed the page */
        s65_op_add(rop_result, S65_OP_AD2C,        S65_REG_PCH,    0x00,                           d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_COND | S65_OPFLAG_IF_CROSSED);

        /* [T0] */
        /* Bypassed if condition met */

        /* Incrementing PC */
        s65_op_add(rop_result, S65_OP_PC_INC,      S65_REG_NULL,   S65_REG_NULL,                   0,       S65_OPFLAG_READ | S65_OPFLAG_IF_NOT_COND);

        /* Next instruction */
    }

    /* Read - modify - write operations (6) */
    else if(s65_instr_size(instr) > 1u && (
            strcmpi(instr->s_mnemo, "ASL") == 0 ||
            strcmpi(instr->s_mnemo, "DEC") == 0 ||
            strcmpi(instr->s_mnemo, "INC") == 0 ||
            strcmpi(instr->s_mnemo, "LSR") == 0 ||
            strcmpi(instr->s_mnemo, "ROL") == 0 ||
            strcmpi(instr->s_mnemo, "ROR") == 0
        ))
    {
        /* Actual execution - obtaining single operation */
        operation_t *tmp_op = s65_unique_op(instr);
        assert(tmp_op && "This should not have happened.");

        /* Zero page addressing (4.1) */
        if(instr->am_mode == S65_ZEROPAGE)
        {
            /* [T1] */

            /* Incrementing PC */
            s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Reading effective address low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T2] */ d_cycle++;

            /* Setting address bus to effective address low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x00,                           d_cycle, S65_OPFLAG_READ                        );
            
            /* [T3] */ d_cycle++;

            /* Doing the same but as WRITE */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_WRITE                       );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x00,                           d_cycle, S65_OPFLAG_WRITE                       );
            /* Now DATA contains pre-modified value */

            /* [T4] */ d_cycle++;

            /* Modyfying the data */
            s65_op_add(rop_result, tmp_op->ot_type,     S65_REG_DATA,   tmp_op->ad_secnd,               d_cycle, S65_OPFLAG_WRITE | tmp_op->d_flags    );            
        }

        /* Absolute addressing (4.2) */
        else if(instr->am_mode == S65_ABSOLUTE)
        {
            /* [T1] */

            /* Incrementing PC */
            s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Reading effective address low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T2] */ d_cycle++;

            /* Incrementing PC */
            s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Reading effective address high */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADH,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T3] */ d_cycle++;

            /* Setting address bus to effective address */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_ADH,                    d_cycle, S65_OPFLAG_READ                        );

            /* [T4] */ d_cycle++;

            /* Doing the same but as WRITE */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_WRITE                       );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_ADH,                    d_cycle, S65_OPFLAG_WRITE                       );
            /* Now DATA contains pre-modified value */

            /* [T5] */ d_cycle++;

            /* Modyfying the data */
            s65_op_add(rop_result, tmp_op->ot_type,     S65_REG_DATA,   tmp_op->ad_secnd,               d_cycle, S65_OPFLAG_READ | tmp_op->d_flags     ); 
            /* Fake load to update the DATA */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_DATA,   S65_REG_DATA,                   d_cycle, S65_OPFLAG_WRITE                       ); 
        }

        /* Zero page, X addressing (4.3) */
        else if(instr->am_mode == S65_ZEROPAGE_X)
        {
            /* [T1] */

            /* Incrementing PC */
            s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Reading base address low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T2] */ d_cycle++;

            /* Setting address bus to base address low and discarding */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x00,                           d_cycle, S65_OPFLAG_READ                        );
            
            /* [T3] */ d_cycle++;

            /* Adding index register without C' to base address low */
            s65_op_add(rop_result, S65_OP_ADD,          S65_REG_ADL,    S65_REG_X,                      d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to calculated address low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x00,                           d_cycle, S65_OPFLAG_READ                        );
            
            /* [T4] */ d_cycle++;

            /* Doing the same but as WRITE */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_WRITE                       );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x00,                           d_cycle, S65_OPFLAG_WRITE                       );
            /* Now DATA contains pre-modified value */

            /* [T5] */ d_cycle++;

            /* Modyfying the data */
            s65_op_add(rop_result, tmp_op->ot_type,     S65_REG_DATA,   tmp_op->ad_secnd,               d_cycle, S65_OPFLAG_WRITE | tmp_op->d_flags    ); 
        }

        /* Absolute, X addressing (4.4) */
        else if(instr->am_mode == S65_ABSOLUTE_X)
        {

            /* [T1] */

            /* Incrementing PC */
            s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Reading base address low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T2] */ d_cycle++;

            /* Incrementing PC */
            s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Reading base address high */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADH,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T3] */ d_cycle++;

            /* Adding index register without C to base address low */
            s65_op_add(rop_result, S65_OP_ADD,          S65_REG_ADL,    S65_REG_X,                      d_cycle, S65_OPFLAG_READ                        );
            /* Adding C to base address high */
            s65_op_add(rop_result, S65_OP_ADC,          S65_REG_ADH,    0x00,                           d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to calculated address and discarding */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_ADH,                    d_cycle, S65_OPFLAG_READ                        );
        
            /* [T4] */ d_cycle++;

            /* Setting address bus to calculated address and discarding again */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_ADH,                    d_cycle, S65_OPFLAG_READ                        );

            /* [T5] */ d_cycle++;

            /* Doing the same but as WRITE */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_WRITE                       );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_ADH,                    d_cycle, S65_OPFLAG_WRITE                       );

            /* [T6] */ d_cycle++;

            /* Modyfying the data */
            s65_op_add(rop_result, tmp_op->ot_type,     S65_REG_DATA,   tmp_op->ad_secnd,               d_cycle, S65_OPFLAG_WRITE | tmp_op->d_flags    ); 
        }
        
        /* Invalid addressing mode */
        else
        {
            assert("This should not have happened.");
        }
        
        /* [T0] */

        /* Incrementing PC */
        s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   0,       S65_OPFLAG_READ                        );

        /* Next instruction */
        free(tmp_op);
    }

    /* Internal execution on memory data (12) */
    else if(s65_instr_size(instr) > 1u)
    {
        
        /* Immediate addressing (2.1) */
        if(instr->am_mode == S65_IMMEDIATE)
        {
            /* [T1] */

            /* Incrementing PC */
            s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Now DATA contains valid const operand */
        }

        /* Zero page addressing (2.2) */
        else if(instr->am_mode == S65_ZEROPAGE)
        {
            /* [T1] */

            /* Incrementing PC */
            s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Reading effective address low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T2] */ d_cycle++;

            /* Setting address bus to effective address low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x00,                           d_cycle, S65_OPFLAG_READ                        );
            /* Now DATA contains valid operand */
        }

        /* Absolute addressing (2.3) */
        else if(instr->am_mode == S65_ABSOLUTE)
        {
            /* [T1] */

            /* Incrementing PC */
            s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Reading effective address low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T2] */ d_cycle++;

            /* Incrementing PC */
            s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Reading effective address high */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADH,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T3] */ d_cycle++;

            /* Setting address bus to effective address */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_ADH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Now DATA contains valid operand */
        }
    
        /* Indirect, X addressing (2.4) */
        else if(instr->am_mode == S65_INDIRECT_X)
        {
            /* [T1] */
            
            /* Incrementing PC */
            s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Reading base address low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T2] */ d_cycle++;

            /* Setting address bus to base address low and discarding */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x00,                           d_cycle, S65_OPFLAG_READ                        );

            /* [T3] */ d_cycle++;
            
            /* Adding X without C' to address bus low */
            s65_op_add(rop_result, S65_OP_ADD,          S65_REG_ABL,    S65_REG_X,                      d_cycle, S65_OPFLAG_READ                        );

            /* [T4] */ d_cycle++;

            /* Reading effective addres low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );
            /* Adding 1 without C' to address bus low */
            s65_op_add(rop_result, S65_OP_ADD,          S65_REG_ABL,    0x01,                           d_cycle, S65_OPFLAG_READ                        );
            /* Reading effective addres high */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADH,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T5] */ d_cycle++;

            /* Setting address bus to effective address */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_ADH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Now DATA contains valid operand */
        }
    
        /* Absolute, X or Absolute, Y addressing (2.5) */
        else if(instr->am_mode == S65_ABSOLUTE_X ||
                instr->am_mode == S65_ABSOLUTE_Y)
        {
            /* Obtaining index register */
            const addr_t ad_idx = (instr->am_mode == S65_ABSOLUTE_X) ? S65_REG_X : S65_REG_Y;

            /* [T1] */

            /* Incrementing PC */
            s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Reading base address low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T2] */ d_cycle++;

            /* Incrementing PC */
            s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Reading base address high */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADH,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T3] */ d_cycle++;

            /* Adding index register without C' to base address low */
            s65_op_add(rop_result, S65_OP_ADD,          S65_REG_ADL,    ad_idx,                         d_cycle, S65_OPFLAG_READ                        );
            /* Adding C' to base address high */
            s65_op_add(rop_result, S65_OP_ADC,          S65_REG_ADH,    0x00,                           d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to calculated address */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_ADH,                    d_cycle, S65_OPFLAG_READ                        );
            
            /* [T4] */ d_cycle++;
            /* Bypassed, if page not crossed */

            /* Setting address bus to PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_CROSSED);
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_CROSSED);
            /* Reading base address high again */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADH,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_CROSSED);
            /* Adding 1 to base address high */
            s65_op_add(rop_result, S65_OP_ADD,          S65_REG_ADH,    0x01,                           d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_CROSSED);

            /* Now DATA contains valid operand */
        }

        /* Zero page, X or Zero page, Y addressing (2.6) */
        else if(instr->am_mode == S65_ZEROPAGE_X ||
                instr->am_mode == S65_ZEROPAGE_Y)
        {
            /* Obtaining index register */
            const addr_t ad_idx = (instr->am_mode == S65_ZEROPAGE_X) ? S65_REG_X : S65_REG_Y;

            /* [T1] */

            /* Incrementing PC */
            s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Reading base address low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T2] */ d_cycle++;

            /* Setting address bus to base address low and discarding */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x00,                           d_cycle, S65_OPFLAG_READ                        );
            
            /* [T3] */ d_cycle++;

            /* Adding index register without C' to base address low */
            s65_op_add(rop_result, S65_OP_ADD,          S65_REG_ADL,    ad_idx,                         d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to calculated address low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x00,                           d_cycle, S65_OPFLAG_READ                        );
            /* Now DATA contains valid operand */
        }

        /* Indirect, Y addressing (2.7) */
        else if(instr->am_mode == S65_INDIRECT_Y)
        {
            /* [T1] */

            /* Incrementing PC */
            s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
            /* Setting address bus to PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );
            /* Reading indirect address low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T2] */ d_cycle++;

            /* Setting address bus to indirect address low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x00,                           d_cycle, S65_OPFLAG_READ                        );
            /* Reading base address low */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        ); 

            /* [T3] */ d_cycle++;
            
            /* Adding 1 without C to address bus low */
            s65_op_add(rop_result, S65_OP_ADD,          S65_REG_ABL,    0x01,                           d_cycle, S65_OPFLAG_READ                        );
            /* Reading base addres high */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADH,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ                        );

            /* [T4] */ d_cycle++;

            /* Adding Y without C to address low */
            s65_op_add(rop_result, S65_OP_ADD,          S65_REG_ADL,    S65_REG_Y,                      d_cycle, S65_OPFLAG_READ                        );
            /* Adding C to address high */
            s65_op_add(rop_result, S65_OP_ADC,          S65_REG_ADH,    0x00,                           d_cycle, S65_OPFLAG_READ                        );

            /* Setting address bus to calculated address */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_READ                        );
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_ADH,                    d_cycle, S65_OPFLAG_READ                        );

            /* [T5] */ d_cycle++;
            /* Bypassed if page not crossed */

            /* Setting address bus to PC */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_CROSSED);
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_CROSSED);
            /* Reading indirect address low again */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_CROSSED);
            /* Setting address bus to indirect address low again */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_CROSSED);
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    0x00,                           d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_CROSSED);
            /* Reading base address low again */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADL,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_CROSSED);
            /* Adding 1 without C to address bus low again */
            s65_op_add(rop_result, S65_OP_ADD,          S65_REG_ABL,    0x01,                           d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_CROSSED);
            /* Reading base address high again */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ADH,    S65_REG_DATA,                   d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_CROSSED);
            /* Setting address bus to address and discarding */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_CROSSED);
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_ADH,                    d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_CROSSED);
            /* Adding Y without C to address low */
            s65_op_add(rop_result, S65_OP_ADD,          S65_REG_ADL,    S65_REG_Y,                      d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_CROSSED);
            /* Adding 1 to address high */
            s65_op_add(rop_result, S65_OP_ADD,          S65_REG_ADH,    0x01,                           d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_CROSSED);
            /* Setting address bus to calculated address */
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_ADL,                    d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_CROSSED);
            s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_ADH,                    d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_CROSSED);

            /* Now DATA contains valid operand */
        }

        /* Invalid addressing mode */
        else
        {
            assert("This should not have happened.");
        }

        /* [T0] */

        /* Actual execution - obtaining single operation */
        operation_t *tmp_op = s65_unique_op(instr);
        assert(tmp_op && "This should not have happened.");

        s65_op_add(rop_result, tmp_op->ot_type,   tmp_op->ad_first, S65_REG_DATA,                   0,          tmp_op->d_flags | S65_OPFLAG_READ      );

        /* Incrementing PC */
        s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   0,          S65_OPFLAG_READ                        );

        /* Next instruction */
        
    }

    /* Other single byte instructions (22 for 6500, 24 for 65C00) */
    else if(s65_instr_size(instr) == 1u)
    {
        /* [T1] */

        /* Incrementing PC */
        s65_op_add(rop_result, S65_OP_PC_INC,       S65_REG_NULL,   S65_REG_NULL,                   d_cycle, S65_OPFLAG_READ                        );
        /* Setting address bus to PC and discarding */
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABL,    S65_REG_PCL,                    d_cycle, S65_OPFLAG_READ                        );
        s65_op_add(rop_result, S65_OP_LOAD,         S65_REG_ABH,    S65_REG_PCH,                    d_cycle, S65_OPFLAG_READ                        );

        /* [T0] */

        /* Actual execution - obtaining single operation */
        operation_t *tmp_op = s65_unique_op(instr);
        assert(tmp_op && "This should not have happened.");

        s65_op_add(rop_result, tmp_op->ot_type,   tmp_op->ad_first, tmp_op->ad_secnd,               0,      tmp_op->d_flags | S65_OPFLAG_READ       );

        /* Next instruction from there */
        free(tmp_op);
    }

    /* Invalid */
    else
    {
        assert("This should not have happened.");
    }

    /* ILLEGAL INSTRUCTIONS */
    ILLEGAL:

    return rop_result;
}

/* Converts an interruption into a list
 * of operations.
 *
 * @param b_int      the interruption type (S65_INT_*)
 * 
 * @returns Valid pointer to op_result or 
 * NULL if failed.
 */
const op_result_t *s65_convert_int(byte b_int)
{
    /* RESET operation list */
    static op_result_t *rop_res = NULL;
    /* NMI operation list */
    static op_result_t *rop_nmi = NULL;
    /* IRQ operation list */
    static op_result_t *rop_irq = NULL;

    /* Filling the lists if it has not been
     * done yet
     */
    if(rop_res == NULL)
    {
        if((rop_res = s65_new_opresult(0u)) == NULL)
        {
            /* Fail */
            return NULL;
        }

        /* Filling RESET 
         * (based on vidual6502.org)
         */

        int d_cycle = 0;

        /* [T0] */

        /* Setting SP to 0x00 */
        s65_op_add(rop_res, S65_OP_LOAD,      S65_REG_SP,         0x00,           d_cycle, S65_OPFLAG_READ                   );
        /* Setting PC to 0x00FF */
        s65_op_add(rop_res, S65_OP_LOAD,      S65_REG_PCL,        0xFF,           d_cycle, S65_OPFLAG_READ                   );
        s65_op_add(rop_res, S65_OP_LOAD,      S65_REG_PCH,        0x00,           d_cycle, S65_OPFLAG_READ                   );
        /* Setting address bus to PC and discarding */
        s65_op_add(rop_res, S65_OP_LOAD,      S65_REG_ABL,        S65_REG_PCL,    d_cycle, S65_OPFLAG_READ                   );
        s65_op_add(rop_res, S65_OP_LOAD,      S65_REG_ABH,        S65_REG_PCH,    d_cycle, S65_OPFLAG_READ                   );

        /* [T1] */ d_cycle++;

        /* Setting address bus to PC and discarding again */
        s65_op_add(rop_res, S65_OP_LOAD,      S65_REG_ABL,        S65_REG_PCL,    d_cycle, S65_OPFLAG_READ                   );
        s65_op_add(rop_res, S65_OP_LOAD,      S65_REG_ABH,        S65_REG_PCH,    d_cycle, S65_OPFLAG_READ                   );

        /* [T2] */ d_cycle++;

        /* Setting address bus to PC and discarding again */
        s65_op_add(rop_res, S65_OP_LOAD,      S65_REG_ABL,        S65_REG_PCL,    d_cycle, S65_OPFLAG_READ                   );
        s65_op_add(rop_res, S65_OP_LOAD,      S65_REG_ABH,        S65_REG_PCH,    d_cycle, S65_OPFLAG_READ                   );

        /* [T3] */ d_cycle++;

        /* Setting address bus to 9-bit SP (0x100) and discarding */
        s65_op_add(rop_res, S65_OP_LOAD,      S65_REG_ABL,       S65_REG_SP,      d_cycle, S65_OPFLAG_READ                   );
        s65_op_add(rop_res, S65_OP_LOAD,      S65_REG_ABH,       0x01,            d_cycle, S65_OPFLAG_READ                   );

        /* [T4] */ d_cycle++;

        /* Decrementing SP (0x00 --> 0xFF / 0x100 --> 0x1FF) */
        s65_op_add(rop_res, S65_OP_DEC,       S65_REG_SP,        S65_REG_NULL,    d_cycle, S65_OPFLAG_READ                   );
        /* Setting address bus to 9-bit SP (0x1FF) and discarding */
        s65_op_add(rop_res, S65_OP_LOAD,      S65_REG_ABL,       S65_REG_SP,      d_cycle, S65_OPFLAG_READ                   );
        s65_op_add(rop_res, S65_OP_LOAD,      S65_REG_ABH,       0x01,            d_cycle, S65_OPFLAG_READ                   );

        /* [T5] */ d_cycle++;

        /* Decrementing SP (0xFF --> 0xFE / 0x1FF --> 0x1FE) */
        s65_op_add(rop_res, S65_OP_DEC,       S65_REG_SP,        S65_REG_NULL,    d_cycle, S65_OPFLAG_READ                   );
        /* Setting address bus to 9-bit SP (0x1FE) and discarding */
        s65_op_add(rop_res, S65_OP_LOAD,      S65_REG_ABL,       S65_REG_SP,      d_cycle, S65_OPFLAG_READ                   );
        s65_op_add(rop_res, S65_OP_LOAD,      S65_REG_ABH,       0x01,            d_cycle, S65_OPFLAG_READ                   );

        /* [T6] */ d_cycle++;

        /* Decrementing SP (0xFE --> 0xFD / 0x1FE --> 0x1FD) */
        s65_op_add(rop_res, S65_OP_DEC,       S65_REG_SP,        S65_REG_NULL,    d_cycle, S65_OPFLAG_READ                   );
        /* Setting address bus to reset vector (0xFFFC) */
        s65_op_add(rop_res, S65_OP_LOAD,      S65_REG_ABL,       S65_LOW(S65_VECTOR_RES),  d_cycle, S65_OPFLAG_READ          );
        s65_op_add(rop_res, S65_OP_LOAD,      S65_REG_ABH,       S65_HIGH(S65_VECTOR_RES), d_cycle, S65_OPFLAG_READ          );
        /* Reading address low */
        s65_op_add(rop_res, S65_OP_LOAD,      S65_REG_ADL,       S65_REG_DATA,    d_cycle, S65_OPFLAG_READ                   );

        /* [T7] */ d_cycle++;

        /* Setting address bus to reset vector + 1 (0xFFFD) */
        s65_op_add(rop_res, S65_OP_LOAD,      S65_REG_ABL,       S65_LOW(S65_VECTOR_RES + 1u),  d_cycle, S65_OPFLAG_READ     );
        s65_op_add(rop_res, S65_OP_LOAD,      S65_REG_ABH,       S65_HIGH(S65_VECTOR_RES + 1u), d_cycle, S65_OPFLAG_READ     );
        /* Reading address high */
        s65_op_add(rop_res, S65_OP_LOAD,      S65_REG_ADH,       S65_REG_DATA,    d_cycle, S65_OPFLAG_READ                   );
        /* Setting I flag */
        s65_op_add(rop_res, S65_OP_SET,       S65_REG_SREG,      S65_SREG_I,      d_cycle, S65_OPFLAG_READ                   );
        /* Clearing D flag */
        s65_op_add(rop_res, S65_OP_CLEAR,     S65_REG_SREG,      S65_SREG_D,      d_cycle, S65_OPFLAG_READ                   );

        /* [T0] */

        /* Updating PC */
        s65_op_add(rop_res, S65_OP_LOAD,      S65_REG_PCL,        S65_REG_ADL,    0      , S65_OPFLAG_READ                   );
        s65_op_add(rop_res, S65_OP_LOAD,      S65_REG_PCH,        S65_REG_ADH,    0      , S65_OPFLAG_READ                   );

        /* Instruction execution at PC */
    }

    if(rop_nmi == NULL)
    {
        if((rop_nmi = s65_new_opresult(0u)) == NULL)
        {
            /* Fail */
            return NULL;
        }

        /* Filling NMI */

        int d_cycle = 0;

        /* [T0] */

        /* Setting address bus to PC and discarding */
        s65_op_add(rop_nmi, S65_OP_LOAD,      S65_REG_ABL,        S65_REG_PCL,    d_cycle, S65_OPFLAG_READ                   );
        s65_op_add(rop_nmi, S65_OP_LOAD,      S65_REG_ABH,        S65_REG_PCH,    d_cycle, S65_OPFLAG_READ                   );

        /* [T1] */ d_cycle++;

        /* Setting address bus to 9-bit SP and discarding */
        s65_op_add(rop_nmi, S65_OP_LOAD,      S65_REG_ABL,       S65_REG_SP,      d_cycle, S65_OPFLAG_READ                   );
        s65_op_add(rop_nmi, S65_OP_LOAD,      S65_REG_ABH,       0x01,            d_cycle, S65_OPFLAG_READ                   );

        /* [T2] */ d_cycle++;

        /* Decrementing SP  */
        s65_op_add(rop_nmi, S65_OP_DEC,       S65_REG_SP,        S65_REG_NULL,    d_cycle, S65_OPFLAG_WRITE                  );
        /* Setting address bus to 9-bit SP */
        s65_op_add(rop_nmi, S65_OP_LOAD,      S65_REG_ABL,       S65_REG_SP,      d_cycle, S65_OPFLAG_WRITE                  );
        s65_op_add(rop_nmi, S65_OP_LOAD,      S65_REG_ABH,       0x01,            d_cycle, S65_OPFLAG_WRITE                  );
        /* Pushing PCH */
        s65_op_add(rop_nmi, S65_OP_LOAD,      S65_REG_DATA,      S65_REG_PCH,     d_cycle, S65_OPFLAG_WRITE                  );

        /* [T3] */ d_cycle++;

        /* Decrementing SP  */
        s65_op_add(rop_nmi, S65_OP_DEC,       S65_REG_SP,        S65_REG_NULL,    d_cycle, S65_OPFLAG_WRITE                  );
        /* Setting address bus to 9-bit SP */
        s65_op_add(rop_nmi, S65_OP_LOAD,      S65_REG_ABL,       S65_REG_SP,      d_cycle, S65_OPFLAG_WRITE                  );
        s65_op_add(rop_nmi, S65_OP_LOAD,      S65_REG_ABH,       0x01,            d_cycle, S65_OPFLAG_WRITE                  );
        /* Pushing PCL */
        s65_op_add(rop_nmi, S65_OP_LOAD,      S65_REG_DATA,      S65_REG_PCL,     d_cycle, S65_OPFLAG_WRITE                  );

        /* [T4] */ d_cycle++;

        /* Decrementing SP  */
        s65_op_add(rop_nmi, S65_OP_DEC,       S65_REG_SP,        S65_REG_NULL,    d_cycle, S65_OPFLAG_WRITE                  );
        /* Setting address bus to 9-bit SP */
        s65_op_add(rop_nmi, S65_OP_LOAD,      S65_REG_ABL,       S65_REG_SP,      d_cycle, S65_OPFLAG_WRITE                  );
        s65_op_add(rop_nmi, S65_OP_LOAD,      S65_REG_ABH,       0x01,            d_cycle, S65_OPFLAG_WRITE                  );
        /* Pushing SREG */
        s65_op_add(rop_nmi, S65_OP_LOAD,      S65_REG_DATA,      S65_REG_SREG,    d_cycle, S65_OPFLAG_WRITE                  );

        /* [T5] */ d_cycle++;

        /* Setting address bus to reset vector */
        s65_op_add(rop_nmi, S65_OP_LOAD,      S65_REG_ABL,       S65_LOW(S65_VECTOR_NMI),  d_cycle, S65_OPFLAG_READ     );
        s65_op_add(rop_nmi, S65_OP_LOAD,      S65_REG_ABH,       S65_HIGH(S65_VECTOR_NMI), d_cycle, S65_OPFLAG_READ     );
        /* Reading address low */
        s65_op_add(rop_nmi, S65_OP_LOAD,      S65_REG_ADL,       S65_REG_DATA,    d_cycle, S65_OPFLAG_READ                   );
        /* Setting I flag */
        s65_op_add(rop_nmi, S65_OP_SET,       S65_REG_SREG,      S65_SREG_I,      d_cycle, S65_OPFLAG_READ                   );
        /* Clearing D flag */
        s65_op_add(rop_nmi, S65_OP_CLEAR,     S65_REG_SREG,      S65_SREG_D,      d_cycle, S65_OPFLAG_READ                   );

        /* [T6] */ d_cycle++;

        /* Setting address bus to reset vector + 1 */
        s65_op_add(rop_nmi, S65_OP_LOAD,      S65_REG_ABL,       S65_LOW(S65_VECTOR_NMI + 1u),  d_cycle, S65_OPFLAG_READ     );
        s65_op_add(rop_nmi, S65_OP_LOAD,      S65_REG_ABH,       S65_HIGH(S65_VECTOR_NMI + 1u), d_cycle, S65_OPFLAG_READ     );
        /* Reading address high */
        s65_op_add(rop_nmi, S65_OP_LOAD,      S65_REG_ADH,       S65_REG_DATA,    d_cycle, S65_OPFLAG_READ                   );

        /* [T0] */ d_cycle++;

        /* Updating PC */
        s65_op_add(rop_nmi, S65_OP_LOAD,      S65_REG_PCL,        S65_REG_ADL,    d_cycle, S65_OPFLAG_READ                   );
        s65_op_add(rop_nmi, S65_OP_LOAD,      S65_REG_PCH,        S65_REG_ADH,    d_cycle, S65_OPFLAG_READ                   );

        /* Instruction execution at PC */
    }

    if(rop_irq == NULL)
    {
        if((rop_irq = s65_new_opresult(0u)) == NULL)
        {
            /* Fail */
            return NULL;
        }

        /* Filling NMI 
         * (very similar to NMI, but with I flag)
         */

        int d_cycle = 0;

        /* [T0] */

        /* Checking if I if off (interruptions enabled) */
        s65_op_add(rop_irq, S65_OP_BIT_SREG_NOT, S65_SREG_I,      S65_REG_NULL,   d_cycle, S65_OPFLAG_READ                   );

        /* All the operations below are skipped,
         * if I is set
         */

        /* Setting address bus to PC and discarding */
        s65_op_add(rop_irq, S65_OP_LOAD,      S65_REG_ABL,        S65_REG_PCL,    d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_COND);
        s65_op_add(rop_irq, S65_OP_LOAD,      S65_REG_ABH,        S65_REG_PCH,    d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_COND);

        /* [T1] */ d_cycle++;

        /* Setting address bus to 9-bit SP and discarding */
        s65_op_add(rop_irq, S65_OP_LOAD,      S65_REG_ABL,       S65_REG_SP,      d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_COND);
        s65_op_add(rop_irq, S65_OP_LOAD,      S65_REG_ABH,       0x01,            d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_COND);

        /* [T2] */ d_cycle++;

        /* Decrementing SP  */
        s65_op_add(rop_irq, S65_OP_DEC,       S65_REG_SP,        S65_REG_NULL,    d_cycle, S65_OPFLAG_WRITE | S65_OPFLAG_IF_COND);
        /* Setting address bus to 9-bit SP */
        s65_op_add(rop_irq, S65_OP_LOAD,      S65_REG_ABL,       S65_REG_SP,      d_cycle, S65_OPFLAG_WRITE | S65_OPFLAG_IF_COND);
        s65_op_add(rop_irq, S65_OP_LOAD,      S65_REG_ABH,       0x01,            d_cycle, S65_OPFLAG_WRITE | S65_OPFLAG_IF_COND);
        /* Pushing PCH */
        s65_op_add(rop_irq, S65_OP_LOAD,      S65_REG_DATA,      S65_REG_PCH,     d_cycle, S65_OPFLAG_WRITE | S65_OPFLAG_IF_COND);

        /* [T3] */ d_cycle++;

        /* Decrementing SP  */
        s65_op_add(rop_irq, S65_OP_DEC,       S65_REG_SP,        S65_REG_NULL,    d_cycle, S65_OPFLAG_WRITE | S65_OPFLAG_IF_COND);
        /* Setting address bus to 9-bit SP */
        s65_op_add(rop_irq, S65_OP_LOAD,      S65_REG_ABL,       S65_REG_SP,      d_cycle, S65_OPFLAG_WRITE | S65_OPFLAG_IF_COND);
        s65_op_add(rop_irq, S65_OP_LOAD,      S65_REG_ABH,       0x01,            d_cycle, S65_OPFLAG_WRITE | S65_OPFLAG_IF_COND);
        /* Pushing PCL */
        s65_op_add(rop_irq, S65_OP_LOAD,      S65_REG_DATA,      S65_REG_PCL,     d_cycle, S65_OPFLAG_WRITE | S65_OPFLAG_IF_COND);

        /* [T4] */ d_cycle++;

        /* Decrementing SP  */
        s65_op_add(rop_irq, S65_OP_DEC,       S65_REG_SP,        S65_REG_NULL,    d_cycle, S65_OPFLAG_WRITE | S65_OPFLAG_IF_COND);
        /* Setting address bus to 9-bit SP */
        s65_op_add(rop_irq, S65_OP_LOAD,      S65_REG_ABL,       S65_REG_SP,      d_cycle, S65_OPFLAG_WRITE | S65_OPFLAG_IF_COND);
        s65_op_add(rop_irq, S65_OP_LOAD,      S65_REG_ABH,       0x01,            d_cycle, S65_OPFLAG_WRITE | S65_OPFLAG_IF_COND);
        /* Pushing SREG */
        s65_op_add(rop_irq, S65_OP_LOAD,      S65_REG_DATA,      S65_REG_SREG,    d_cycle, S65_OPFLAG_WRITE | S65_OPFLAG_IF_COND);

        /* [T5] */ d_cycle++;

        /* Setting address bus to reset vector */
        s65_op_add(rop_irq, S65_OP_LOAD,      S65_REG_ABL,       S65_LOW(S65_VECTOR_BRK),  d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_COND);
        s65_op_add(rop_irq, S65_OP_LOAD,      S65_REG_ABH,       S65_HIGH(S65_VECTOR_BRK), d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_COND);
        /* Reading address low */
        s65_op_add(rop_irq, S65_OP_LOAD,      S65_REG_ADL,       S65_REG_DATA,    d_cycle, S65_OPFLAG_READ  | S65_OPFLAG_IF_COND);
        /* Setting I flag */
        s65_op_add(rop_irq, S65_OP_SET,       S65_REG_SREG,      S65_SREG_I,      d_cycle, S65_OPFLAG_READ  | S65_OPFLAG_IF_COND);
        /* Clearing D flag */
        s65_op_add(rop_irq, S65_OP_CLEAR,     S65_REG_SREG,      S65_SREG_D,      d_cycle, S65_OPFLAG_READ  | S65_OPFLAG_IF_COND);

        /* [T6] */ d_cycle++;

        /* Setting address bus to reset vector + 1 */
        s65_op_add(rop_irq, S65_OP_LOAD,      S65_REG_ABL,       S65_LOW(S65_VECTOR_BRK + 1u),  d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_COND);
        s65_op_add(rop_irq, S65_OP_LOAD,      S65_REG_ABH,       S65_HIGH(S65_VECTOR_BRK + 1u), d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_COND);
        /* Reading address high */
        s65_op_add(rop_irq, S65_OP_LOAD,      S65_REG_ADH,       S65_REG_DATA,    d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_COND);

        /* [T0] */ d_cycle++;

        /* Updating PC */
        s65_op_add(rop_irq, S65_OP_LOAD,      S65_REG_PCL,        S65_REG_ADL,    d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_COND);
        s65_op_add(rop_irq, S65_OP_LOAD,      S65_REG_PCH,        S65_REG_ADH,    d_cycle, S65_OPFLAG_READ | S65_OPFLAG_IF_COND);

        /* Instruction execution at PC */
    }

    /* Returning */
    switch (b_int)
    {
    case S65_INT_RESET:
        return rop_res;
    case S65_INT_NMI:
        return rop_nmi;
    default:
        return rop_irq;
    }
}

/* Gives one operation, unique for given 
 * instruction (ASL, CLI, TAY etc.).
 *
 * @param in_instr      the instruction
 * 
 * @returns Valid pointer to operation_t or
 * NULL if the instruction is ambigious.
 */
operation_t *s65_unique_op(const instruction_t *in_instr)
{
    assert(in_instr);

    /* Value to be returned */
    operation_t *tmp_op = NULL;

    /* Memory alloc */
    if((tmp_op = (operation_t *) calloc(1u, sizeof(operation_t))) == NULL)
    {
        /* Fail */
        return NULL;
    }

    /* ASL - arithmetic shift left with A */
    if(strcmpi(in_instr->s_mnemo, "ASL") == 0)
    {
        tmp_op->ot_type         = S65_OP_ASL;
        tmp_op->ad_first        = S65_REG_ACC;
        tmp_op->ad_secnd        = S65_REG_NULL;       
    }

    /* CLC, CLD, CLI, CLV - clear bit in SREG */
    else if(strcmpi(in_instr->s_mnemo, "CLC") == 0 ||
            strcmpi(in_instr->s_mnemo, "CLD") == 0 ||
            strcmpi(in_instr->s_mnemo, "CLI") == 0 ||
            strcmpi(in_instr->s_mnemo, "CLV") == 0)
    {
        /* Obtaining the bit */
        const byte tmp_bit = s65_char_to_sreg(in_instr->s_mnemo[2]);

        tmp_op->ot_type         = S65_OP_CLEAR;
        tmp_op->ad_first        = S65_REG_SREG;
        tmp_op->ad_secnd        = tmp_bit; 
    }

    /* DEX, DEY, DEA - decrement register */
    else if(strcmpi(in_instr->s_mnemo, "DEX") == 0 ||
            strcmpi(in_instr->s_mnemo, "DEY") == 0 ||
            strcmpi(in_instr->s_mnemo, "DEA") == 0)
    {
        /* Obtaining the register */
        const addr_t tmp_reg = s65_asci_to_reg(in_instr->s_mnemo + 2);

        tmp_op->ot_type         = S65_OP_DEC;
        tmp_op->ad_first        = tmp_reg;
        tmp_op->ad_secnd        = S65_REG_NULL; 
    }

    /* INX, INY, INA - increment register */
    else if(strcmpi(in_instr->s_mnemo, "INX") == 0 ||
            strcmpi(in_instr->s_mnemo, "INY") == 0 ||
            strcmpi(in_instr->s_mnemo, "INA") == 0)
    {
        /* Obtaining the register */
        const addr_t tmp_reg = s65_asci_to_reg(in_instr->s_mnemo + 2);

        tmp_op->ot_type         = S65_OP_INC;
        tmp_op->ad_first        = tmp_reg;
        tmp_op->ad_secnd        = S65_REG_NULL; 
    }

    /* LSR - logic shift right with A */
    else if(strcmpi(in_instr->s_mnemo, "LSR") == 0)
    {
        tmp_op->ot_type         = S65_OP_LSR;
        tmp_op->ad_first        = S65_REG_ACC;
        tmp_op->ad_secnd        = S65_REG_NULL;     
    }

    /* NOP - no operation */
    else if(strcmpi(in_instr->s_mnemo, "NOP") == 0)
    {
        tmp_op->ot_type         = S65_OP_NOP;
        tmp_op->ad_first        = S65_REG_NULL;
        tmp_op->ad_secnd        = S65_REG_NULL;
    }

    /* ROL - rotate left with A */
    else if(strcmpi(in_instr->s_mnemo, "ROL") == 0)
    {
        tmp_op->ot_type         = S65_OP_ROL;
        tmp_op->ad_first        = S65_REG_ACC;
        tmp_op->ad_secnd        = S65_REG_NULL;     
    }

    /* ROR - rotate right with A */
    else if(strcmpi(in_instr->s_mnemo, "ROR") == 0)
    {
        tmp_op->ot_type         = S65_OP_ROR;
        tmp_op->ad_first        = S65_REG_ACC;
        tmp_op->ad_secnd        = S65_REG_NULL;     
    }

    /* SEC, SED, SEI - set bit in SREG */
    else if(strcmpi(in_instr->s_mnemo, "SEC") == 0 ||
            strcmpi(in_instr->s_mnemo, "SED") == 0 ||
            strcmpi(in_instr->s_mnemo, "SEI") == 0)
    {
        /* Obtaining the bit */
        const byte tmp_bit = s65_char_to_sreg(in_instr->s_mnemo[2]);

        tmp_op->ot_type         = S65_OP_SET;
        tmp_op->ad_first        = S65_REG_SREG;
        tmp_op->ad_secnd        = tmp_bit; 
    }

    /* TAX, TAY, TSX, TXA, TXS, TYA - transfer */
    else if(strcmpi(in_instr->s_mnemo, "TAX") == 0 ||
            strcmpi(in_instr->s_mnemo, "TAY") == 0 ||
            strcmpi(in_instr->s_mnemo, "TSX") == 0 ||
            strcmpi(in_instr->s_mnemo, "TXA") == 0 ||
            strcmpi(in_instr->s_mnemo, "TXS") == 0 ||
            strcmpi(in_instr->s_mnemo, "TYA") == 0)
    {
        /* First register (destination) */
        addr_t tmp_dest;
        {
            /* 3rd char + NULL terminator */
            const char buf[2] = { in_instr->s_mnemo[2], '\0' };
            tmp_dest = s65_asci_to_reg(buf);
        }

        /* Second register (source) */
        addr_t tmp_src;
        {
            /* 2nd char + NULL terminator */
            const char buf[2] = { in_instr->s_mnemo[1], '\0' };
            tmp_src = s65_asci_to_reg(buf);
        }

        tmp_op->ot_type         = S65_OP_LOAD;
        tmp_op->ad_first        = tmp_dest;
        tmp_op->ad_secnd        = tmp_src;
    }

    /* Internal execution on memory data -
     * 2nd operand should be neglected here 
     * as it depends on the addressing mode 
     * (unknown here) 
     */

    /* ADC - adding to A with carry */
    else if(strcmpi(in_instr->s_mnemo, "ADC") == 0)
    {
        tmp_op->ad_first = S65_REG_ACC;
        tmp_op->ot_type = S65_OP_ADCC;
    }

    /* AND - and with A */
    else if(strcmpi(in_instr->s_mnemo, "AND") == 0)
    {
        tmp_op->ad_first = S65_REG_ACC;
        tmp_op->ot_type = S65_OP_AND;
    }

    /* BIT - AND with A without storing */
    else if(strcmpi(in_instr->s_mnemo, "BIT") == 0)
    {
        tmp_op->ad_first = S65_REG_ACC;
        tmp_op->ot_type = S65_OP_BIT;
    }

    /* CMP - comparing with A */
    else if(strcmpi(in_instr->s_mnemo, "CMP") == 0)
    {
        tmp_op->ad_first = S65_REG_ACC;
        tmp_op->ot_type = S65_OP_CMP;
    }

    /* CPX - comparing with X */
    else if(strcmpi(in_instr->s_mnemo, "CPX") == 0)
    {
        tmp_op->ad_first = S65_REG_X;
        tmp_op->ot_type = S65_OP_CMP;
    }

    /* CPY - comparing with Y */
    else if(strcmpi(in_instr->s_mnemo, "CPY") == 0)
    {
        tmp_op->ad_first = S65_REG_Y;
        tmp_op->ot_type = S65_OP_CMP;
    }

    /* EOR - exclusive OR with A */
    else if(strcmpi(in_instr->s_mnemo, "EOR") == 0)
    {
        tmp_op->ad_first = S65_REG_ACC;
        tmp_op->ot_type = S65_OP_EOR;
    }

    /* LDA, LDX, LDY - loading register */
    else if(strcmpi(in_instr->s_mnemo, "LDA") == 0 ||
            strcmpi(in_instr->s_mnemo, "LDX") == 0 ||
            strcmpi(in_instr->s_mnemo, "LDY") == 0)
    {
        tmp_op->ad_first = s65_asci_to_reg(in_instr->s_mnemo + 2);
        tmp_op->ot_type = S65_OP_LOAD;
    }

    /* ORA - OR with A */
    else if(strcmpi(in_instr->s_mnemo, "ORA") == 0)
    {
        tmp_op->ad_first = S65_REG_ACC;
        tmp_op->ot_type = S65_OP_OR;
    }

    /* SBC - substracting from A with carry */
    else if(strcmpi(in_instr->s_mnemo, "SBC") == 0)
    {
        tmp_op->ad_first = S65_REG_ACC;
        tmp_op->ot_type = S65_OP_SBC;
    }

    /* Read - modify - write operation        */
    /* 1st operand should be neglected here   */
    /* as it depends on addressing mode       */
    /* (unknown here)                         */

    /* DEC - decrementing memory */
    else if(strcmpi(in_instr->s_mnemo, "DEC") == 0)
    {
        tmp_op->ad_secnd = S65_REG_NULL;
        tmp_op->ot_type = S65_OP_DEC;
    }

    /* INC - incrementing memory */
    else if(strcmpi(in_instr->s_mnemo, "INC") == 0)
    {
        tmp_op->ad_secnd = S65_REG_NULL;
        tmp_op->ot_type = S65_OP_INC;
    }

    /* Invalid */
    else
    {
        free(tmp_op);
        return NULL;
    }

    return tmp_op;
}

/* Calculates # of cycles for given list
 * of operations.
 *
 * @param rop_list      the list
 * 
 * @returns Positive number of cycles if
 * succeeded, zero otherwise.
 */
size_t s65_instr_cycles(const op_result_t *rop_list)
{
    assert(rop_list);

    /* Number of the cycles is equal: 1 + number of times 
     * the 'd_cycle' member has increased. 
     */
    if(rop_list->sz_count == 0u)
        return 0u;
    if(rop_list->sz_count == 1u)
        return 1u;

    /* The counter */
    size_t tmp_counter = 1u;

    for(size_t i = 1u; i < rop_list->sz_count; ++i)
    {
        if(rop_list->op_list[i - 1].d_cycle < rop_list->op_list[i].d_cycle)
            ++tmp_counter;
    }

    return tmp_counter;
}

/* Translates an operation into ASM-like pseudo code. 
 *
 * @param op_oper       the operation
 * @param d_flags       options (S65_ASM_)
 * 
 * @returns Valid string if succeeded, NULL 
 * otherwise.
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
        snprintf(tmp_buffer, _BUFFER_SIZE, (d_flags & S65_ASM_LOWERCASE) ? "Fetch: %02x" : "Fetch: %02X", op_oper->ad_first);
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
    if(S65_IS_REG(op_oper->ad_first))
        snprintf(tmp_buffer, _BUFFER_SIZE, "%s ", s65_reg_to_asci(op_oper->ad_first, (d_flags & S65_ASM_SHORTEN)));

    /* Not a register - physical address */
    else if(d_flags & S65_ASM_AS_HEXADECIMAL)
        snprintf(tmp_buffer, _BUFFER_SIZE, (d_flags & S65_ASM_LOWERCASE) ? "%04x" : "%04X", op_oper->ad_first);
    else if(d_flags & S65_ASM_AS_OCTAL)
        snprintf(tmp_buffer, _BUFFER_SIZE, "%04o", op_oper->ad_first);
    else
        snprintf(tmp_buffer, _BUFFER_SIZE, "%04u", op_oper->ad_first);

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
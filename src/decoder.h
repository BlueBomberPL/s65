/*
 *  decoder.h
 *
 *  The decoder is responsible for translating instruction 
 *  OPCODE into a list of smaller operations, which
 *  cannot be further divided. Each operation takes one 
 *  clock cycle. Basing on OPCODE the decoder also knows, 
 *  how many bytes each instruction takes, therefore
 *  it can correctly read arguments (if any).
 * 
 *  The original 6502 and 65C02 have some OPCODEs that
 *  are not officialy documented - they either do something
 *  useful or halt the CPU. If the latter, the chip must be
 *  restarted. WDC65C02 model is lacking these, as they all
 *  are implemented as NOP.
 */

#ifndef _S65_DECODER_H_FILE_
#define _S65_DECODER_H_FILE_

#define S65_OPFLAG_DISCARD      (1 << 0)    /* The result is discarded          */

#define S65_ASM_LOWERCASE       (1 << 0)    /* Using lowercase for values       */
#define S65_ASM_AS_HEXADECIMAL  (1 << 1)    /* Writing values in hex system     */
#define S65_ASM_AS_OCTAL        (1 << 2)    /* Writing values in octal system   */
#define S65_ASM_SHORTEN         (1 << 3)    /* Using register abbreviations     */

#include "memory.h"
#include "types.h"

#include <assert.h>

/* Instruction sets */
typedef enum _s65_instr_set
{
    S65_SET_6500,                           /* Original NMOS 6500 set     */
    S65_SET_65C00,                          /* Original 65C00 set         */
    S65_SET_WDC65C00,                       /* WDC 65C00 set (the biggest)*/

    S65_SET_TOTAL

} instruction_set_t;

/* Addressing modes */
typedef enum _s65_addrmode
{
    /*  type               id    # of bytes */
    /* 1 byte  */
    S65_IMPLIED         = (0x01 | (1 << 6)),

    /* 2 bytes */
    S65_IMMEDIATE       = (0x02 | (2 << 6)),
    S65_INDIRECT,
    S65_INDIRECT_X,
    S65_INDIRECT_Y,
    S65_RELATIVE,
    S65_ZEROPAGE,
    S65_ZEROPAGE_X,
    S65_ZEROPAGE_Y,
    S65_ZEROPAGE_INDIRECT,

    /* 3 bytes */
    S65_ABSOLUTE        = (0x0A | (3 << 6)),
    S65_ABSOLUTE_X,
    S65_ABSOLUTE_Y,
    S65_ABSOLUTE_INDIRECT_X, /* new */

    S65_ADDRMODES_TOTAL = 14

} addrmode_t;

/* Operation types */
typedef enum _s65_optype
{
    S65_OP_FETCH        = 0x00,         /* OPCODE Fetch                */
    S65_OP_LOAD,                        /* A  <-- B                    */
    S65_OP_PC_INC,                      /* PC <-- PC + 1               */
    S65_OP_ADD,                         /* A  <-- A + B                */
    S65_OP_ADD_ONE,                     /* A  <-- A + B + 1            */
    S65_OP_ADC,                         /* A  <-- A + B + C            */ 
    S65_OP_INC,                         /* A  <-- A + 1                */
    S65_OP_DEC,                         /* A  <-- A - 1                */
    S65_OP_ASL,                         /* A  <-- A << 1               */ 
    S65_OP_LSR,                         /* A  <-- A >> 1               */
    S65_OP_ROL,                         /* A  <-- A <r< 1              */
    S65_OP_ROR,                         /* A  <-- A >r> 1              */
    S65_OP_SET,                         /* A  <-- A | B                */
    S65_OP_CLEAR,                       /* A  <-- A & ~B               */
    S65_OP_AND,                         /* A  <-- A && B               */
    S65_OP_OR,                          /* A  <-- A || B               */
    S65_OP_EOR,                         /* A  <-- A ^ B                */
    S65_OP_NOP,                         /* Nothing                     */
    S65_OP_UNKNOWN,                     /* ???                         */

    S65_OP_TOTAL
           
} optype_t;

/* An instruction instance */
typedef struct _s65_instr
{
    char       *s_mnemo;                    /* Assembly mnemonic           */
    byte        b_opcode;                   /* The OPCODE value            */
    addrmode_t  am_mode;                    /* The addressing mode         */
    instruction_set_t st_set;               /* The instruction set         */

} instruction_t;

/* An operation instance */
typedef struct _s65_op
{
    optype_t    ot_type;                    /* The type of operation       */
    dword       b_first;                    /* First argument value        */
    dword       b_secnd;                    /* Second argument value       */
    int         d_cycle;                    /* Cycle ID (T0-Tn)            */
    int         d_flags;                    /* Flags (S65_OPFLAG_*)        */

} operation_t;

/* A result of executing
 * an instruction
 */
typedef struct _s65_opresult
{
    operation_t *op_list;                   /* List of operations          */
    size_t       sz_count;                  /* List size                   */

} op_result_t;

/* Queries size of an instruction.
 *
 * @param it_instr      the instruction
 * 
 * @returns The size in bytes.
 */
inline size_t       s65_instr_size(const instruction_t *in_instr)
{
    assert(in_instr);
    return (size_t) (in_instr->am_mode >> 6);
}

/* Allocates new op result.
 *
 * @param sz_len        the list size
 *
 * @returns Valid struct pointer or 
 * NULL if failed.
 */
inline op_result_t *s65_new_opresult(size_t sz_len)
{
    op_result_t *tmp_res = (op_result_t *) malloc(sizeof(op_result_t));
    if(tmp_res == NULL || (tmp_res->op_list = (operation_t *) malloc(sizeof(operation_t) * sz_len)) == NULL)
        return NULL;
    tmp_res->sz_count = sz_len;
    return tmp_res;
}

/* Translates an instruction.
 *
 * @param dt_memory     program memory data
 * @param wd_pc         program counter value
 * @param st_set        instruction set
 * 
 * @returns Valid instruction type or
 * NULL if failed.
 */
const instruction_t *s65_decode(const data_t *dt_memory, word wd_pc, instruction_set_t st_set);

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
op_result_t        *s65_convert(const data_t *dt_memory, word wd_pc, instruction_set_t st_set);

/* Translates an operation into assembly. 
 *
 * @param op_oper       the operation
 * @param d_flags       options (S65_ASM_)
 */
char               *s65_to_asm(const operation_t *op_oper, int d_flags);

 #endif /* _S65_DECODER_H_FILE_ */
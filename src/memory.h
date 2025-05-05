/*
 *  memory.h
 *
 *  This file consists of most things
 *  required to manage 6500's memory structure,
 *  including both internal registers (and buffers) and 
 *  external addressable pool. The registers are referenced via
 *  their virtual addresses to simplify data management.
 */

#ifndef _S65_MEMORY_H_FILE_
#define _S65_MEMORY_H_FILE_

#include "types.h"

#include <assert.h>

/* Address alias */
typedef uint32_t            addr_t;

/* Registers */
#define S65_REG_NULL     ((addr_t) 0xFFFFF) /* NULL replacement if no reg. used */   
#define S65_REG_ACC      ((addr_t) 0x10000) /* Accumulator's virtual address    */
#define S65_REG_SREG     ((addr_t) 0x10001) /* Status register's virtual addr.  */
#define S65_REG_X        ((addr_t) 0x10002) /* Index X's virtual addr.          */
#define S65_REG_Y        ((addr_t) 0x10003) /* Index Y's virtual addr.          */
#define S65_REG_PCL      ((addr_t) 0x10004) /* Program counter's low v.a.       */
#define S65_REG_PCH      ((addr_t) 0x10005) /* Program counter's high v.a.      */
#define S65_REG_SP       ((addr_t) 0x10006) /* Stack pointer's v.a.             */
#define S65_REG_DATA     ((addr_t) 0x10007) /* Data bus' (D0-D7) v.a.           */
#define S65_REG_ADL      ((addr_t) 0x10008) /* Internal address' low v.a.       */
#define S65_REG_ADH      ((addr_t) 0x10009) /* Internal address' high v.a.      */
#define S65_REG_ABL      ((addr_t) 0x1000A) /* Address bus' low (A0-A7) v.a.    */
#define S65_REG_ABH      ((addr_t) 0x1000B) /* Address bus' high (A8-A15) v.a.  */
#define S65_MEM_SPACE    ((addr_t) 0x1000C) /* Size of full memory space        */

/* Size of register memory space        */
#define S65_MEM_REGSPACE ((addr_t) S65_MEM_SPACE - 65536)  

/* Vectors */
#define S65_VECTOR_BRK   ((word) 0xFFFE)    /* Break interrupt vector (2 words) */
#define S65_VECTOR_NMI   ((word) 0xFFFA)    /* Non maskable int. vec. (2 words) */
#define S65_VECTOR_RES   ((word) 0xFFFC)    /* Reset int. vec. (2 words)        */

/* Bits */
#define S65_SREG_N       (7)                /* N flag                           */
#define S65_SREG_V       (6)                /* V flag                           */
#define S65_SREG_B       (4)                /* B flag                           */
#define S65_SREG_D       (3)                /* D flag                           */
#define S65_SREG_I       (2)                /* I flag                           */
#define S65_SREG_Z       (1)                /* Z flag                           */
#define S65_SREG_C       (0)                /* C flag                           */

/* Other */
#define S65_PAGE_SIZE    (256)              /* Memory page size (in bytes)      */

/* Checks, if the address is a register */
#define S65_IS_REG(addr)   ((addr) > (word)-1)

/* Converts virtual address to register address */
#define S65_ADDR_TO_REG(addr) ((addr) - (word)-1)

/* Gets lower byte of a word */
#define S65_LOW(x)         ((x) & 0xFF)

/* Gets higher byte of a word */
#define S65_HIGH(x)        (((x) >> 8) & 0xFF)  

/* Packs two bytes into a word */
#define S65_PACK(h, l)     (((h) << 8) | (l))

/* Checks, if bit is set */
#define S65_IS_SET(x, b)   (!!((x) & (1 << (b))))


/* Converts an ASCII char to a flag.
 *
 * @param c_c       the char
 * 
 * @returns An SREG flag or invalid.
 */
static inline byte s65_char_to_sreg(char c)
{
    switch(c)
    {
        case 'N':
            return S65_SREG_N;
        case 'V':
            return S65_SREG_V;
        case 'B':
            return S65_SREG_B;
        case 'D':
            return S65_SREG_D;
        case 'I':
            return S65_SREG_I;
        case 'Z':
            return S65_SREG_Z;
        case 'C':
            return S65_SREG_C;
        default:
            return (byte) -1;
    }
}

/* Converts a flag to an ASCII char.
 *
 * @param b_flag    the flag
 * 
 * @returns A char or invalid.
 */
static inline char s65_sreg_to_char(byte b_flag)
{
    switch(b_flag)
    {
        case S65_SREG_N:
            return 'N';
        case S65_SREG_V:
            return 'V';
        case S65_SREG_B:
            return 'B';
        case S65_SREG_D:
            return 'D';
        case S65_SREG_I:
            return 'I';
        case S65_SREG_Z:
            return 'Z';
        case S65_SREG_C:
            return 'C';
        default:
            return (char) 0;
    }
}

/* Checks, if two addresses belong
 * to different memory pages.
 *
 * @param r_a1      first address
 * @param r_a2      second address
 * 
 * @returns True if boundary is crossed,
 * false otherwise or if any of the addresses
 * is invalid (> 65535).
 */
static inline bool s65_is_page_crossed(addr_t r_a1, addr_t r_a2)
{
    if(S65_IS_REG(r_a1) || S65_IS_REG(r_a2))
        return 0;
    return ((r_a1 % S65_PAGE_SIZE) != (r_a2 % S65_PAGE_SIZE));
}

/* Converts an ASCII into a register. 
 *
 * @param s_str     the register's name
 * 
 * @returns Valid register virtual address
 * or invalid.
 */
addr_t          s65_asci_to_reg(const char *s_str);

/* Converts a register into ASCII.
 *
 * @param r_reg     the register
 * @param b_alt     if an alternative (shorter) string
 * 
 * @returns Valid string or empty if failed.
 */
const char      *s65_reg_to_asci(addr_t r_reg, byte b_alt);

#endif /* _S65_MEMORY_H_FILE_ */
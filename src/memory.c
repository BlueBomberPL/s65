/*
 *  memory.c
 *
 *  Implements 'memory.h'.
 */

 #include "memory.h"

/* Converts an ASCII into a register. 
 *
 * @param s_str     the register's name
 * 
 * @returns Valid register virtual address
 * or invalid.
 */
addr_t s65_asci_to_reg(const char *s_str)
{
    assert(s_str);

    /* Accumulator (universal      /        alternative) */
    if(strcmp(s_str, "A") == 0 || strcmp(s_str, "ACC") == 0)
        return S65_REG_ACC;
    /* SREG */
    if(strcmp(s_str, "P") == 0 || strcmp(s_str, "SREG") == 0)
        return S65_REG_SREG;
    /* Index X */
    if(strcmp(s_str, "X") == 0 || strcmp(s_str, "IX") == 0)
        return S65_REG_X;
    /* Index Y */
    if(strcmp(s_str, "Y") == 0 || strcmp(s_str, "IY") == 0)
        return S65_REG_Y;
    /* Program counter low */
    if(strcmp(s_str, "PCL") == 0)
        return S65_REG_PCL;
    /* Program counter high */
    if(strcmp(s_str, "PCH") == 0)
        return S65_REG_PCH;
    /* Stack pointer */
    if(strcmp(s_str, "S") == 0 || strcmp(s_str, "SP") == 0)
        return S65_REG_SP;
    /* Data buffer */
    if(strcmp(s_str, "DATA") == 0)
        return S65_REG_DATA;
    /* Address low */
    if(strcmp(s_str, "ADL") == 0)
        return S65_REG_ADL;
    /* Address high */
    if(strcmp(s_str, "ADH") == 0)
        return S65_REG_ADH;
    /* Address bus low */
    if(strcmp(s_str, "ABL") == 0)
        return S65_REG_ABL;
    /* Address bus high */
    if(strcmp(s_str, "ABH") == 0)
        return S65_REG_ABH;

    /* Bad */
    return S65_REG_NULL;
}

/* Converts a register into ASCII.
 *
 * @param r_reg     the register
 * @param b_alt     if an alternative (shorter) string
 * 
 * @returns Valid string or empty if failed.
 */
const char *s65_reg_to_asci(addr_t r_reg, byte b_alt)
{
    /* If not a register, skip */
    if(! S65_IS_REG(r_reg))
        return "";

    switch(r_reg)
    {
        case S65_REG_ACC:
            return (b_alt) ? "A" : "ACC";
        case S65_REG_SREG:
            return (b_alt) ? "R" : "SREG";
        case S65_REG_X:
            return (b_alt) ? "X" : "IX";
        case S65_REG_Y:
            return (b_alt) ? "Y" : "IY";
        case S65_REG_PCL:
            return "PCL";
        case S65_REG_PCH:
            return "PCH";
        case S65_REG_SP:
            return "SP";
        case S65_REG_DATA:
            return "DATA";
        case S65_REG_ADL:
            return (b_alt) ? "ADL" : "ADDRL";
        case S65_REG_ADH:
            return (b_alt) ? "ADH" : "ADDRH";
        case S65_REG_ABL:
            return (b_alt) ? "ABL" : "ADBUSL";
        case S65_REG_ABH:
            return (b_alt) ? "ABH" : "ADBUSH";
        default:
            return "";
    }
}
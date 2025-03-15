/*
 *  types.h
 *
 *  This header offers basic aliases and init func.
 *  used across the whole project. It includes
 *  6500 native integer types.
 */

#ifndef _S65_H_FILE_INCLUDED_
#define _S65_H_FILE_INCLUDED_

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 8 bits signed (for relative branching) */
typedef int8_t      SBYTE;

/* 8 bits unsigned */
typedef uint8_t     BYTE;

/* 16 bits unsigned */
typedef uint16_t    WORD;

/* 32 bits unsigned (extended) */
typedef uint32_t    DWORD;

/* 64 bits unsigned (extended) */
typedef uint64_t    QWORD;

/* Data array */
typedef struct _s65_byte_block
{
    BYTE    *pb_block;                      /* Pointer to the memory block */
    size_t   sz_bksize;                     /* Number of members           */

} data_t;

/* Creates new zero'ed data block. 
 *
 * @param sz_bksize     number of members
 * @param b_reset       value to be filled with
 * 
 * @returns Valid pointer or NULL if failed.
 */
data_t          *s65_new_block(size_t sz_bksize, BYTE b_reset);


#endif /* _S65_H_FILE_INCLUDED_ */
/*
 *  types.h
 *
 *  This header offers basic aliases and init func.
 *  used across the whole project. It includes
 *  6500 native integer types.
 */

#ifndef _S65_H_FILE_INCLUDED_
#define _S65_H_FILE_INCLUDED_

#define S65_NORMAL_BUFSIZE      (16)        /* General purpose buffer size */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 8 bits signed (for relative branching) */
typedef int8_t      sbyte;

/* 8 bits unsigned */
typedef uint8_t     byte;

/* 16 bits unsigned */
typedef uint16_t    word;

/* 32 bits unsigned (extended) */
typedef uint32_t    dword;

/* 64 bits unsigned (extended) */
typedef uint64_t    qword;

/* Data array */
typedef struct _s65_byte_block
{
    byte        *pb_block;                  /* Pointer to the memory block */
    size_t      sz_bksize;                  /* Number of members           */

} data_t;

/* Creates new data block. 
 *
 * @param sz_bksize     number of members
 * @param b_reset       value to be filled with
 * 
 * @returns Valid pointer or NULL if failed.
 */
data_t          *s65_new_block(size_t sz_bksize, byte b_reset);


#endif /* _S65_H_FILE_INCLUDED_ */
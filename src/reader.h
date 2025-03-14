/*
 *  reader.h
 *
 *  Objects put here are responsible for
 *  reading programs binary data and converting
 *  it into a raw block of values, which can be 
 *  then passed to the CPU's decoder.
 */

#ifndef _S65_READER_H_FILE_
#define _S65_READER_H_FILE_

#define S65_IHEX_BAD_CSUM       (-1)        /* IHex: checksum mismatch          */
#define S65_IHEX_DATA           (0)         /* IHex record type: data           */
#define S65_IHEX_END            (1)         /* IHex record type: eof            */
#define S65_IHEX_UNKNOWN        (-1)        /* IHex record type: unsupported    */

#define S65_READER_AUTO         (0)         /* Deduces file type from its ext.  */
#define S65_READER_BIN          (1 << 1)    /* Opens file as a raw byte file    */
#define S65_READER_HEX          (1 << 2)    /* Opens file as Intel .hex         */
#define S65_READER_PEDANTIC     (1 << 4)    /* Checks checksums and other rules */

#define S65_LINE_LEN            64          /* Max length of line in file       */

#include "types.h"
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* IHex record */
typedef struct _s65_ihex_rec
{
    size_t   sz_address;                    /* Adress of the data               */
    data_t  *pd_data;                       /* The data itself                  */
    int      d_type;                        /* Record type                      */
    int      b_csum;                        /* Checksum, negative if bad        */

} ihex_rec_t;

/* Calculates the length of a number (in digits).
 *
 * @param n             the number
 * 
 * @returns Number of digits. 
 */
inline size_t       s65_dlen(unsigned long n)
{
    size_t tmp_cnt = 1u;
    while(n > 0lu)
    {
        ++tmp_cnt;
        n >>= 1;
    }
    return tmp_cnt;
}

/* Reads data from a given file. 
 * Converts it to a data block.
 * 
 * @param s_filename    name of the file
 * @param d_settings    optional flags (S65_READER_*)
 * 
 * @returns Pointer to a valid struct or NULL if failed.
 */
data_t              *s65_read_file(const char *s_filename, int d_settings);

 #endif /* _S65_READER_H_FILE_ */
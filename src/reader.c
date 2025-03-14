/*
 *  reader.c
 *
 *  Implements 'reader.h'.
 */

#include "reader.h"

/* Converts a line of text into IHex record.
 * Ignores semicolons and checksums.
 * [INTERNAL]
 *
 * @param s_str         the text
 * 
 * @returns Pointer to a valid STATIC struct or
 * NULL if failed. The pointer is overwritten with
 * each function call, must not be freed.
 */
static ihex_rec_t *_s65_parse_ihex(char *s_str)
{
    assert(s_str);

    /* Data to be returned */
    static ihex_rec_t *phx_result = NULL;
    /* Temporary iteration pointer */
    char *tmp_it = s_str;
    /* Temporary swap character */
    char tmp_c;
    /* Temporary data block info */
    size_t tmp_bytes = 0u;
    
#define _IHEX_BYTE_LEN       2
#define _IHEX_ADDR_LEN       4
#define _IHEX_TYPE_LEN       2
#define _IHEX_CSUM_LEN       2
    
    /* Minimum length: 2 + 4 + 2 */
    /* (bytes + addr + type) */
    if(strlen(s_str) < _IHEX_BYTE_LEN + _IHEX_ADDR_LEN + _IHEX_TYPE_LEN)
    {
        /* 100% invalid */
        return NULL;
    }

    /* Alloc if needed */
    if(phx_result == NULL && (phx_result = (ihex_rec_t *) calloc(1u, sizeof(ihex_rec_t))) == NULL)
    {
        /* Fail */
        return NULL;
    }
    else
    {
        /* Erasing the data block */
        free(phx_result->pd_data);
    }

    /* Skipping the colon */
    if(s_str[0] == ':')
        tmp_it = s_str + 1;

    /* Reading data */
    
    /* Bytes (first 2 digits) */
    tmp_c = tmp_it[_IHEX_BYTE_LEN];
    tmp_it[_IHEX_BYTE_LEN] = '\0';
    if(sscanf_s(tmp_it, "%x", &tmp_bytes) < 1)
    {
        /* Fail */
        return NULL;
    }
    /* Now data block alloc is possible */
    else if((phx_result->pd_data = s65_new_block(tmp_bytes)) == NULL)
    {
        /* Fail */
        return NULL;
    }
    else
    {
        /* Variable assigment */
        phx_result->pd_data->sz_bksize = tmp_bytes;
    }

    /* Address (next 4 digits) */
    tmp_it[_IHEX_BYTE_LEN] = tmp_c;
    tmp_it += _IHEX_BYTE_LEN;
    tmp_c = tmp_it[_IHEX_ADDR_LEN];
    tmp_it[_IHEX_ADDR_LEN] = '\0';
    if(sscanf_s(tmp_it, "%x", &phx_result->sz_address) < 1)
    {
        /* Fail */
        return NULL;
    }

    /* Type */
    tmp_it[_IHEX_ADDR_LEN] = tmp_c;
    tmp_it += _IHEX_ADDR_LEN;
    tmp_c = tmp_it[_IHEX_TYPE_LEN];
    tmp_it[_IHEX_TYPE_LEN] = '\0';
    if(sscanf_s(tmp_it, "%x", &phx_result->d_type) < 1)
    {
        /* Fail */
        return NULL;
    }
    /* If type == EOF, ending */
    else if(phx_result->d_type == S65_IHEX_END)
    {
        /* OK */
        return phx_result;
    }

    /* Data (next n * 2 digits) */
    tmp_it[_IHEX_TYPE_LEN] = tmp_c;
    tmp_it += _IHEX_TYPE_LEN;
    for(size_t i = 0, k = 0; i < (tmp_bytes * 2u - 1u); ++i)
    {
        /* Temp hex byte buffer */
        char tmp_buffer[3] = {0, };
            tmp_buffer[0] = tmp_it[i];
            tmp_buffer[1] = tmp_it[i + 1];

        /* Converting to decimal */
        if(sscanf_s(tmp_buffer, "%x", &((BYTE *)phx_result->pd_data->pb_block)[k++]) < 1)
        {
            /* Failed */
            return NULL;
        }
    }

    /* Checksum (last 2 digits) */
    tmp_it += (tmp_bytes * 2u);
    if((tmp_it - s_str) >= (strlen(s_str) - (s_str[0] == ':')) || 
        sscanf_s(tmp_it, "%x", &phx_result->b_csum) < 1u)
    {
        /* Failed */
        phx_result->b_csum = S65_IHEX_BAD_CSUM;
    }

    /* Calculating valid checksum */
    BYTE tmp_validcsum = 0u;
    /* Summing every byte  */
    tmp_validcsum += tmp_bytes + 
                    (phx_result->sz_address & 0xFF) +
                    (phx_result->sz_address >> 8) +
                    (phx_result->d_type);
    /* Adding data bytes */
    for(size_t i = 0u; i < phx_result->pd_data->sz_bksize; ++i)
    {
        tmp_validcsum += ((BYTE *)(phx_result->pd_data))[i];
    }
    /* Complementary */
    tmp_validcsum = 256 - tmp_validcsum;
    /* Comparing */
    if(phx_result->b_csum != tmp_validcsum)
    {
        /* Mismatch */
        phx_result->b_csum = S65_IHEX_BAD_CSUM;
    }

    /* The end */
    return phx_result;

#undef _IHEX_BYTE_LEN
#undef _IHEX_ADDR_LEN
#undef _IHEX_TYPE_LEN
#undef _IHEX_CSUM_LEN       

}

/* Reads data from a given stream. 
 * Converts BINARY to a data block.
 * [INTERNAL]
 * 
 * @param f_file        the stream
 * @param d_settings    optional flags (S65_READER_*)
 * 
 * @returns Pointer to a valid struct or NULL if failed.
 */
static data_t *_s65_read_bin(FILE *f_file, int d_settings)
{
    assert(f_file);
    
    /* The block to be returned */
    data_t *pdt_data = NULL;
    /* Temp. value */
    size_t tmp_bksize = 0u;


    /* Calculating file size (# of block members) */
    /* One character == one byte */
    fseek(f_file, 0l, SEEK_END);
    tmp_bksize = (((size_t) ftell(f_file)));
    fseek(f_file, 0l, SEEK_SET);

    /* Filling to even number */
    if(tmp_bksize % 2u != 0u)
        tmp_bksize += 1u;
    
    /* Creating the block */
    if((pdt_data = s65_new_block(tmp_bksize)) == NULL)
    {
        /* Failed */
        return NULL;
    }

    pdt_data->sz_bksize = tmp_bksize;

    /* Filling the block with useful data */
    const size_t tmp_result = fread(
        pdt_data->pb_block, 
        sizeof(BYTE), 
        tmp_bksize,
        f_file);
    
    /* Veryfing */
    if(tmp_result < tmp_bksize)
    {
        /* Failed */
        return NULL;
    }
    
    return pdt_data;
}

/* Reads data from a given stream. 
 * Converts INTEL HEX to a data block.
 * [INTERNAL]
 * 
 * @param f_file        the stream
 * @param d_settings    optional flags (S65_READER_*)
 * 
 * @returns Pointer to a valid struct or NULL if failed.
 */
static data_t *_s65_read_hex(FILE *f_file, int d_settings)
{
    assert(f_file);

    /* The block to be returned */
    data_t *pdt_data = NULL;
    /* The block temporary size */
    size_t tmp_bksize = 0u;

    /* Calculating number of elements */
    /* (the biggest address + bytes sum) */
    /* Also, veryfying the rules */
    while(! feof(f_file))
    {
        /* For each line */
        static char tmp_buffer[S65_LINE_LEN] = "";
        fread((void *) tmp_buffer, sizeof(char), S65_LINE_LEN, f_file);

        /* Temp. record struct */
        const ihex_rec_t *tmp_record = _s65_parse_ihex(tmp_buffer);
        if(tmp_record == NULL)
        {
            /* Failed */
            return NULL;
        }
#if 1
        printf("Bytes:     %zu\n", tmp_record->pd_data->sz_bksize);
        printf("Address:   %zu\n", tmp_record->sz_address);
        printf("Type:      %d\n", tmp_record->d_type);
        printf("Csum:      %d\n", tmp_record->b_csum);
#endif
    
        /* Comparing occupied location */
        /* Alias, tmp_curloc is 0 only if the address */
        /* is 0 and numer of bytes is 0 */
        const size_t tmp_curloc = tmp_record->sz_address + tmp_record->pd_data->sz_bksize;
        if(tmp_curloc > 0u && tmp_bksize < tmp_curloc)
            tmp_bksize = tmp_curloc;  

        /* Veryfying */
        /* Skipping if not wanted */
        if(! (d_settings & S65_READER_PEDANTIC))
            continue;

        /* First character must be a colon */
        if(tmp_buffer[0] != ':')
        {
            /* Failed */
            return NULL;
        } 
        /* The checksum must match */
        else if(tmp_record->b_csum == S65_IHEX_BAD_CSUM)
        {
            /* Failed */
            return NULL;
        }
        /* Last record type must be EOF (0x01) */
        else if(feof(f_file) && tmp_record->d_type != S65_IHEX_END)
        {
            /* Failed */
            return NULL;
        }
    }

    fseek(f_file, 0l, SEEK_SET);
    printf(":|");
    /* Now the block can be allocated */
    if((s65_new_block(tmp_bksize)) == NULL)
    {
        /* Failed */
        return NULL;
    }
    printf(":()");
    /* Scanning again, but saving data bytes */
    /* Veryfying can be omitted now */
    while(! feof(f_file))
    {
        /* For each line */
        static char tmp_buffer[S65_LINE_LEN] = "";
        fread((void *) tmp_buffer, sizeof(char), S65_LINE_LEN, f_file);

        /* Temp. record struct */
        const ihex_rec_t *tmp_record = _s65_parse_ihex(tmp_buffer);
        if(tmp_record == NULL)
        {
            /* Failed */
            return NULL;
        }

        /* Copying bytes */
        if(memcpy(
            ((BYTE *) pdt_data->pb_block) + tmp_record->sz_address,              /* destination */
            tmp_record->pd_data->pb_block,                                       /* source      */
            pdt_data->sz_bksize * sizeof(BYTE)                                   /* size in B   */
        ) == NULL)
        {
            /* Failed */
            return NULL;
        }
    }

    return pdt_data;
}

/* Reads data from a given file. 
 * Converts it to a data block.
 * 
 * @param s_filename    name of the file
 * @param d_settings    optional flags (S65_READER_*)
 * 
 * @returns Pointer to a valid struct or NULL if failed.
 */
data_t *s65_read_file(const char *s_filename, int d_settings)
{
    assert(s_filename);

    /* Main file struct */
    FILE *f_file = NULL;
    /* Pointer to be returned */
    data_t *pdt_data = NULL;
    /* File data type */
    int d_format = S65_READER_AUTO;
    /* Local name copy */
    char *s_fname_copy = NULL;

    /* Copying filename */
    if((s_fname_copy = (char *) malloc(strlen(s_filename) + 1u)) == NULL ||
        strcpy(s_fname_copy, s_filename) == NULL)
    {
        /* Failed */
        free(s_fname_copy);
        return NULL;
    }

    /* Converting to lower */
    for(size_t i = 0u; i < strlen(s_fname_copy); ++i)
    {
        s_fname_copy[i] = (char) tolower(s_fname_copy[i]);
    }

    /* Managing file extension */

    /* Forced IHex */
    if(d_settings & S65_READER_HEX)
    {
        d_format = S65_READER_HEX;
    }
    /* Forced binary */
    if(d_settings & S65_READER_BIN)
    {
        d_format = S65_READER_BIN;
    }
    /* Deducing */
    else
    {
        /* Trying to find appropiate ext. */
        if(strstr(s_fname_copy, ".hex"))
        {
            d_format = S65_READER_HEX;
        }
        else if(strstr(s_fname_copy, ".bin"))
        {
            d_format = S65_READER_BIN;
        }
        else
        {
            /* Failed */
            free(s_fname_copy);
            return NULL;
        }
    }

    /* Opening the file (binary read) */
    if((f_file = fopen(s_fname_copy, "rb")) == NULL)
    {
        /* Failed */
        
        return NULL;
    }
    
    /* End of life for the string copy */
    free(s_fname_copy);

    /* Reading the file */
    if(d_format == S65_READER_BIN)
    {
        pdt_data = _s65_read_bin(f_file, d_settings);
    }
    else if(d_format == S65_READER_HEX)
    {
        pdt_data = _s65_read_hex(f_file, d_settings);
    }

    //fclose(f_file);
    return pdt_data;
}


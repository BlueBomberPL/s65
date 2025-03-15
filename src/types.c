/*
 *  types.c
 *
 *  Implements 'types.h'.
 */

 #include "types.h"

/* Creates new zero'ed data block. 
 *
 * @param sz_bksize     number of members
 * @param b_reset       value to be filled with
 * 
 * @returns Valid pointer or NULL if failed.
 */
data_t *s65_new_block(size_t sz_bksize, BYTE b_reset)
{
    /* Pointer to be returned */
    data_t *pdt_result = NULL;

    /* Struct alloc */
    if((pdt_result = (data_t *) malloc(sizeof(data_t))) == NULL)
    {
        /* Failed */
        return NULL;
    }
    /* Block alloc only if not 0 */
    if(b_reset == 0u && sz_bksize > 0u)
    {
        /* Using calloc if filled with 0 */
        if((pdt_result->pb_block = (BYTE *) calloc(sz_bksize, sizeof(BYTE))) == NULL)
        {
            /* Failed */
            free(pdt_result);
            return NULL;
        }
    }
    else if(sz_bksize > 0u)
    {
        /* Using malloc if filled != 0 */
        if((pdt_result->pb_block = (BYTE *) malloc(sz_bksize * sizeof(BYTE))) == NULL)
        {
            /* Failed */
            free(pdt_result);
            return NULL;
        }
        /* Setting value */
        for(size_t i = 0u; i < sz_bksize; ++i)
            pdt_result->pb_block[i] = b_reset;
    }

    pdt_result->sz_bksize = sz_bksize;
    return pdt_result;
}
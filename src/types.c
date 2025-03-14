/*
 *  types.c
 *
 *  Implements 'types.h'.
 */

 #include "types.h"


/* Creates new zero'ed data block. 
 *
 * @param sz_bksize     number of members
 * 
 * @returns Valid pointer or NULL if failed.
 */
data_t *s65_new_block(size_t sz_bksize)
{
    /* Pointer to be returned */
    data_t *pdt_result = NULL;
    printf("bksize=%zu\n", sz_bksize);

    /* Struct alloc */
    if((pdt_result = (data_t *) malloc(sizeof(data_t))) == NULL)
    {
        /* Failed */
        return NULL;
    }

    /* Block alloc */
    if((pdt_result->pb_block = (BYTE *) calloc(sz_bksize, sizeof(BYTE))) == NULL)
    {
        /* Failed */
        //free(pdt_result);
        return NULL;
    }

    pdt_result->sz_bksize = sz_bksize;
    return pdt_result;
}
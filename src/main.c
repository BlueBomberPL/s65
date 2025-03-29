/*
 *  main.c
 *
 *  Entrance point for the program.
 *  Launches modules.
 * 
 */

 /* TODO
  *
  *   KONTYNUOWAĆ S65_CONVERT(),
  *   DODAĆ FUNKCJĘ OPERATION -> TEKST */

#include "decoder.h"
#include "reader.h"

int main(int argc, char **argv)
{
   data_t *block = s65_new_block(65536, 0x1B);
   if(block)
   {
      block->pb_block[0] = 0xE8;

      op_result_t *op = s65_convert(block, 0u, S65_SET_65C00);
      if(op)
      {
         for(size_t i = 0; i < op->sz_count; ++i)
         {
            printf("%u:\tT%d:\t%d\tv1=%02X\tv2=%02X\n", i, op->op_list[i].d_cycle, op->op_list[i].ot_type, op->op_list[i].b_first, op->op_list[i].b_secnd);
         }
         printf("end");
      }
      printf("end");
   }
}

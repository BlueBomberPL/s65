/*
 *  main.c
 *
 *  Entrance point for the program.
 *  Launches modules.
 * 
 *    TODO: Napraw ihex_parse(), nie działa
 *    s65_new_block (linia 74)
 * 
 */

#include "reader.h"

int main(int argc, char **argv)
{
   data_t *d = s65_read_file("../res/file.hex", S65_READER_AUTO);
   if(! d)
      printf("(null)");

   for(size_t i = 0; i < d->sz_bksize; ++i)
      printf("%04x:\t\t%02x\n", i, d->pb_block[i]);
   printf("end");
}

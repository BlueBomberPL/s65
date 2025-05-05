/*
 *  main.c
 *
 *  Entrance point for the program.
 *  Launches modules.
 * 
 */

 /* TODO
  *
  *   NIEKTÓRE INSTRUKCJE MAJĄ 1 CYKL - NAPRAWIĆ
  *   DOKOŃCZYĆ PODPUNKT 4 z APPENDIX A (TESTOWANIE)
  *   DODAC OBSŁUGĘ DLA INSTRUKCJI 65C00
  */

#include "decoder.h"
#include "reader.h"
#include "cpu.h"


void test(void);

void exec(byte **block, const op_result_t *opr);

int main(int argc, char **argv)
{
   test();
}


void test(void)
{
   byte a, b;
   
   while(true)
   {

      scanf("%d %d", &a, &b);

      bool v = (int)a + (int)b != (byte)((word)a + (word)b);
      printf("%u + %u = %u\tv = %d\n",a, b, (byte)((word) a + (word) b), v);
   }
   return;
   s65_cpu_init();
   s65_cpu_reset();
}
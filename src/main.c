/*
 *  main.c
 *
 *  Entrance point for the program.
 *  Launches the modules.
 * 
 */

 /* TODO
  *
  *   POSZUKAĆ BUGÓW
  *   NAPISAĆ SAM SYMULATOR
  *   PRZETESTOWAĆ NMI, IRQ, RES
  *   PRZETESTOWAĆ JSR, RTI, RTS
  */

#include "decoder.h"
#include "reader.h"
#include "cpu.h"

data_t *memo;

void test(void);

void print(void);

void exec(byte **block, const op_result_t *opr);

int main(int argc, char **argv)
{
   s65_cpu_init();

   s65_cpu_clr_flag(S65_SREG_N);
   s65_cpu_clr_flag(S65_SREG_V);
   s65_cpu_clr_flag(S65_SREG_C);
   s65_cpu_clr_flag(S65_SREG_Z);

   test();
}


void test(void)
{
   memo = s65_new_block(110000u, 0x00);

   print();

   while(true)
   {
      word opcode, b1, b2;
      size_t cnt = 0;
      scanf("%hu %hu %hu", &opcode, &b1, &b2);

      *s65_cpu_reg(S65_REG_PCH) = *s65_cpu_reg(S65_REG_PCL) = 0x00;

      memo->pb_block[0x0000] = (byte) opcode;
      memo->pb_block[0x0001] = (byte) b1;
      memo->pb_block[0x0002] = (byte) b2;
      memo->pb_block[S65_VECTOR_RES] = 0x10;
      memo->pb_block[S65_VECTOR_RES + 1] = 0x11;

      const instruction_t *in = s65_decode((byte) opcode);
      const op_result_t *rop = s65_convert_int(S65_INT_RESET);

      word pc = S65_PACK(*s65_cpu_reg(S65_REG_PCH), *s65_cpu_reg(S65_REG_PCL));
      word ab = 0;

      printf("INSTRUCTION: %s\t[0x%02x]\t\n", in->s_mnemo, memo->pb_block[0x0000]);
      
      int cycle = 0; int i = 0;
      for(int cycle = 0, i = 0; i < rop->sz_count;)
      {
         printf("CYCLE: %d\n", cycle);
         
         /* Executing ONE cycle */
         while(rop->op_list[i].d_cycle == cycle && i < rop->sz_count)
         {
            /* USING DATA: update DATA */
            if((rop->op_list[i].ad_first == S65_REG_DATA || rop->op_list[i].ad_secnd == S65_REG_DATA))
            {
               /* Getting AB */
               ab = S65_PACK(*s65_cpu_reg(S65_REG_ABH), *s65_cpu_reg(S65_REG_ABL));
               *s65_cpu_reg(S65_REG_DATA) = memo->pb_block[ab];
               printf("DATA %02x -> %04x\n", memo->pb_block[ab], ab);
            }

            if(s65_cpu_exe(&rop->op_list[i]) == S65_CPURET_SKIPPED)
               ++cnt;

            /* DATA HAS BEEN MODIFIED: update memory */
            if(rop->op_list[i].ad_first == S65_REG_DATA && (rop->op_list[i].d_flags & S65_OPFLAG_WRITE))
            {
               /* Getting AB */
               ab = S65_PACK(*s65_cpu_reg(S65_REG_ABH), *s65_cpu_reg(S65_REG_ABL));
               memo->pb_block[ab] = *s65_cpu_reg(S65_REG_DATA);
               printf("%04x -> DATA: %02x\n", ab, memo->pb_block[ab]);
            }

            ++i;
         }

         if(i < rop->sz_count)
            cycle = rop->op_list[i].d_cycle;
         else
            break;
               
      }

      printf("[OPERATIONS SKIPPED]:\t%d\n", cnt);
      print();
      fflush(stdin);
   }

}

void print(void)
{
   word pc = S65_PACK(*s65_cpu_reg(S65_REG_PCH), *s65_cpu_reg(S65_REG_PCL));
      word ab = 0;
         /* INFO */
      printf("     PC      |     AB    |  D  |  A  |  X  |  Y  |  S  |        P        |\n");
      printf("   $%04x     |   $%04x   | $%02x | $%02x | $%02x | $%02x | $%02x | %c %c - %c %c %c %c %c |\n", pc, ab, *s65_cpu_reg(S65_REG_DATA), *s65_cpu_reg(S65_REG_ACC), *s65_cpu_reg(S65_REG_X), *s65_cpu_reg(S65_REG_Y), *s65_cpu_reg(S65_REG_SP),
      s65_cpu_is_flag(S65_SREG_N) ? 'N' : 'n', s65_cpu_is_flag(S65_SREG_V) ? 'V' : 'v', s65_cpu_is_flag(S65_SREG_B) ? 'B' : 'b', s65_cpu_is_flag(S65_SREG_D) ? 'D' : 'd', s65_cpu_is_flag(S65_SREG_I) ? 'I' : 'i',
      s65_cpu_is_flag(S65_SREG_Z) ? 'Z' : 'z', s65_cpu_is_flag(S65_SREG_C) ? 'C' : 'c');

      for(int i = 0; i < 16; ++i)
         printf("%02x ", memo->pb_block[i]);

      printf(" ... ");
      
      for(int i = 0; i < 16; ++i)
         printf("%02x ", memo->pb_block[S65_PACK(1, 240 + i)]);

      printf("\n");
}
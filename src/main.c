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
   /*

   for(size_t i = 0; i < 256; ++i)
   {
      d->pb_block[S65_REG_PCL] = d->pb_block[S65_REG_PCH] = 0x00;
      d->pb_block[0x0000] = i;
      const instruction_t *in = s65_decode(d, S65_SET_6500);
      if(S65_IS_ILLEGAL(in->s_mnemo))
         continue;

      printf("%s: ", in->s_mnemo);

      op_result_t *opr = s65_convert(d, S65_SET_6500);    
      {
         printf("%02x:\t%d\t%s\t%u\t%u\n", i, (in->am_mode), in->s_mnemo, s65_instr_cycles(opr), opr->sz_count);
      }

      
      free(opr->op_list);
      free(opr);
   }

   */

   s65_cpu_init();
   s65_cpu_reset();
}

void exec(byte **block, const op_result_t *opr)
{
   addr_t a, b;

   for(size_t i = 0; i < opr->sz_count; ++i)
   {
      a = opr->op_list[i].ad_first;
      if(opr->op_list[i].d_flags & S65_OPFLAG_CONST)
         b = opr->op_list[i].ad_secnd;
      else if(opr->op_list[i].ad_secnd != S65_REG_NULL)
         b = (*block)[opr->op_list[i].ad_secnd];
      else
         b = 0;

      /* Updating data bus */
      addr_t address = S65_PACK((*block)[S65_REG_ABH], (*block)[S65_REG_ABL]);

      if(opr->op_list[i].d_flags & S65_OPFLAG_READ)
      {
         byte data = (*block)[address];
         (*block)[S65_REG_DATA] = data;
      }

      /* First operand to text */
      char strop[32] = {0, };

      if(S65_IS_REG(a))
      {
         strcpy_s(strop, 30, s65_reg_to_asci(a, 0));
      }
      else
      {
         sprintf_s(strop, 30, "%04x", a);
      }

      switch (opr->op_list[i].ot_type)
      {
         case S65_OP_PC_INC:
         {
            printf("PC ++\n");
            word val = S65_PACK(
               (*block)[S65_REG_PCH],
               (*block)[S65_REG_PCL]
            );
            ++val;
            (*block)[S65_REG_PCH] = S65_HIGH(val);
            (*block)[S65_REG_PCL] = S65_LOW(val);

            break;
         }
         case S65_OP_LOAD:
         {
            printf("%s <- %02x\n", strop, b);
            (*block)[a] = b;

            /* If loading to DATA, update the data */
            if(a == S65_REG_DATA)
            {
               (*block)[address] = b;
               printf("OUTPUT: VAL(%02x) at ADR(%04x)\n", b, address);
            }
            break;
         }
         case S65_OP_INC:
         {
            printf("%s ++\n", strop);
            (*block)[a]++;
            break;
         }
         case S65_OP_DEC:
         {
            printf("%s --\n", strop);
            (*block)[a]--;
            break;
         }
         case S65_OP_ADD:
         {
            printf("%s += %02x\n", strop, b);
            word temp = (*block)[a] + (word)b;
            /* setting C */
            if(temp > 255)
               (*block)[S65_REG_SREG] |= (1 << S65_SREG_C);
            else
               (*block)[S65_REG_SREG] &= ~(1 << S65_SREG_C);

            (*block)[a] = (temp & 0xFF);
            break;
         }
         case S65_OP_ADC:
         {
            printf("%s += %02x + C\n", strop, b);
            word temp = (*block)[a] + (word)b + !!((*block)[S65_REG_SREG] & (1 << S65_SREG_C));
            /* setting C */
            if(temp > 255)
               (*block)[S65_REG_SREG] |= (1 << S65_SREG_C);
            else
               (*block)[S65_REG_SREG] &= ~(1 << S65_SREG_C);

            (*block)[a] = (temp & 0xFF);
            break;
         }
         default:
         {
            break;
         }
      }
   }
}
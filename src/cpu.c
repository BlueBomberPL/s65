/*
 *  cpu.c
 *
 *  Implements 'cpu.h'.
 */

 #include "cpu.h"

/* Current pin (CPU_PIN) state (1-HIGH, 0-LOW) */
static int g_pin_state = 0;

/* Current CPU state */
static cpu_state_t g_state = S65_PREINIT;

/* The internal memory space */
/* Contains the registers only */
static data_t *g_registers;

/* Sets up CPU variables. 
 *
 * @returns Positive value if
 * succeeded.
 */
int s65_cpu_init(void)
{
    /* Allocating register memory */
    if((g_registers = s65_new_block(S65_MEM_REGSPACE, 0xFF)) == NULL)
    {
        /* Failed */
        return false;
    }
}

/* Resets the CPU.
 * Engages RESET sequence.
 *
 * @returns Number of cycles executed.
 */
int s65_cpu_reset(void)
{
    assert(g_registers);

    /* Creating new operation list */
    /* Once it's created it can be used */
    /* many times */
    static op_result_t *rop_reset = NULL;
    if(rop_reset == NULL && (rop_reset = s65_new_opresult(0u)) == NULL)
    {
        /* Failed */
        return 0;
    }

    /* Filling the list if not done yet */
    if(rop_reset->sz_count == 0u)
    {
        /* Based on visual6502.org */

        int d_cycle = 0;

        /* [T0] */

        /* Setting SP to 0x00 */
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_SP,         0x00,           d_cycle, S65_OPFLAG_READ | S65_OPFLAG_CONST);
        /* Setting PC to 0x00FF */
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_PCL,        0xFF,           d_cycle, S65_OPFLAG_READ | S65_OPFLAG_CONST);
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_PCH,        0x00,           d_cycle, S65_OPFLAG_READ | S65_OPFLAG_CONST);
        /* Setting address bus to PC and discarding */
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_ABL,        S65_REG_PCL,    d_cycle, S65_OPFLAG_READ                   );
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_ABH,        S65_REG_PCH,    d_cycle, S65_OPFLAG_READ                   );

        /* [T1] */ d_cycle++;

        /* Setting address bus to PC and discarding again */
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_ABL,        S65_REG_PCL,    d_cycle, S65_OPFLAG_READ                   );
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_ABH,        S65_REG_PCH,    d_cycle, S65_OPFLAG_READ                   );

        /* [T2] */ d_cycle++;

        /* Setting address bus to PC and discarding again */
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_ABL,        S65_REG_PCL,    d_cycle, S65_OPFLAG_READ                   );
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_ABH,        S65_REG_PCH,    d_cycle, S65_OPFLAG_READ                   );

        /* [T3] */ d_cycle++;

        /* Setting address bus to 9-bit SP (0x100) and discarding */
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_ABL,       S65_REG_SP,      d_cycle, S65_OPFLAG_READ                   );
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_ABH,       0x01,            d_cycle, S65_OPFLAG_READ | S65_OPFLAG_CONST);

        /* [T4] */ d_cycle++;

        /* Decrementing SP (0x00 --> 0xFF / 0x100 --> 0x1FF) */
        s65_op_add(rop_reset, S65_OP_DEC,       S65_REG_SP,        S65_REG_NULL,    d_cycle, S65_OPFLAG_READ                   );
        /* Setting address bus to 9-bit SP (0x1FF) and discarding */
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_ABL,       S65_REG_SP,      d_cycle, S65_OPFLAG_READ                   );
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_ABH,       0x01,            d_cycle, S65_OPFLAG_READ | S65_OPFLAG_CONST);

        /* [T5] */ d_cycle++;

        /* Decrementing SP (0xFF --> 0xFE / 0x1FF --> 0x1FE) */
        s65_op_add(rop_reset, S65_OP_DEC,       S65_REG_SP,        S65_REG_NULL,    d_cycle, S65_OPFLAG_READ                   );
        /* Setting address bus to 9-bit SP (0x1FE) and discarding */
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_ABL,       S65_REG_SP,      d_cycle, S65_OPFLAG_READ                   );
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_ABH,       0x01,            d_cycle, S65_OPFLAG_READ | S65_OPFLAG_CONST);

        /* [T6] */ d_cycle++;

        /* Decrementing SP (0xFE --> 0xFD / 0x1FE --> 0x1FD) */
        s65_op_add(rop_reset, S65_OP_DEC,       S65_REG_SP,        S65_REG_NULL,    d_cycle, S65_OPFLAG_READ                   );
        /* Setting address bus to reset vector (0xFFFC) */
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_ABL,       S65_LOW(S65_VECTOR_RES),  d_cycle, S65_OPFLAG_READ | S65_OPFLAG_CONST);
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_ABH,       S65_HIGH(S65_VECTOR_RES), d_cycle, S65_OPFLAG_READ | S65_OPFLAG_CONST);
        /* Reading address low */
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_ADL,       S65_REG_DATA,    d_cycle, S65_OPFLAG_READ                   );

        /* [T7] */ d_cycle++;

        /* Setting address bus to reset vector + 1 (0xFFFD) */
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_ABL,       S65_LOW(S65_VECTOR_RES + 1u),  d_cycle, S65_OPFLAG_READ | S65_OPFLAG_CONST);
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_ABH,       S65_HIGH(S65_VECTOR_RES + 1u), d_cycle, S65_OPFLAG_READ | S65_OPFLAG_CONST);
        /* Reading address high */
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_ADH,       S65_REG_DATA,    d_cycle, S65_OPFLAG_READ                   );

        /* [T0] */

        /* Updating PC */
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_PCL,        S65_REG_ADL,    0      , S65_OPFLAG_READ                   );
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_PCH,        S65_REG_ADH,    0      , S65_OPFLAG_READ                   );

        /* Instruction execution at PC */
    }

    /* RESET procedure is defined */
    /* it can be executed */
    for(size_t i = 0; i < rop_reset->sz_count; ++i)
    {
        printf("%zu: \t", i);
        s65_cpu_exe(&rop_reset->op_list[i]);
    }

    /* Updating the state */
    g_state = S65_RUNNING;

    /* Returning # of cycles of RESET */
    return rop_reset->sz_count;
}

/* Executes an operation.
 *
 * op           the operation
 * 
 * @returns Operation ID if succeeded,
 * zero value otherwise.
 */
int s65_cpu_exe(const operation_t *op)
{
    /* This function implementation is CRUCIAL */

    assert(op);
    assert(g_registers);

    /* Indicates if previous condition has been met */
    /* used for S65_OP_BIT_SREG and S65_OP_BIT_SREG_NOT */
    /* to branch or not, true by default */
    static bool is_cond_met = true; 

    /* Indicates, if previous operation crossed pages, */
    /* false by default */
    static bool is_page_crossed = false;

    /* First operand (A) */
    const addr_t ad_op_a = op->ad_first;
    /* Second operand (B) */
    const addr_t ad_op_b = op->ad_secnd;

    /* Executing */

    /* Checking conditions */
    if(op->d_flags & S65_OPFLAG_IF_COND && ! is_cond_met)
    {
        /* Condition not met */
        goto END;
    }
    else if(op->d_flags & S65_OPFLAG_IF_CROSSED && ! is_page_crossed)
    {
        /* Condition not met */
        goto END;
    }

    switch(op->ot_type)
    {
        /* Fetching OPCODE */
        case S65_OP_FETCH:
        {
            /* The OPCODE value */
            byte tmp_opcode = S65_IS_REG(ad_op_a) ? *s65_cpu_reg(ad_op_a) : (byte) ad_op_a;

            /* Doing nothing */

            /* Conditions */
            is_cond_met = true;
            is_page_crossed = false;
            break;
        }

        /* Loading A with B */
        /* Does not set flags */
        /* Cannot cross pages */
        /* Condition always true */
        case S65_OP_LOAD:
        {
            /* Value to load */
            byte tmp_val = 0;

            /* Interpreting B */
            tmp_val = S65_IS_REG(ad_op_b) ? *s65_cpu_reg(ad_op_b) : ad_op_b;

            /* Operation (A is a register) */
            *s65_cpu_reg(ad_op_a) = tmp_val;

            /* Conditions */
            is_cond_met = true;
            is_page_crossed = false;
            break;
        }

        /* PC (2-byte) incrementation */
        /* Does not set flags */
        /* Can cross pages */
        /* Condition always true */
        case S65_OP_PC_INC:
        {
            /* Obtaining current PC value */
            word tmp_pc = S65_PACK(*s65_cpu_reg(S65_REG_PCH), *s65_cpu_reg(S65_REG_PCL));
            /* Old PC value */
            const word tmp_old_pc = tmp_pc;

            /* Incrementing */
            tmp_pc++;
            /* Putting the value back */
            *s65_cpu_reg(S65_REG_PCH) = S65_HIGH(tmp_pc);
            *s65_cpu_reg(S65_REG_PCL) = S65_LOW(tmp_pc);

            /* Conditions */
            is_cond_met = true;
            is_page_crossed = s65_is_page_crossed(tmp_pc, tmp_old_pc);
            break;
        }

        /* PC (2-byte) U2 addition without C */
        /* Does not set flags */
        /* Can cross pages */
        /* Condition always true */
        case S65_OP_PC_AD2:
        {
            /* Obtaining current PC value */
            word tmp_pc = S65_PACK(*s65_cpu_reg(S65_REG_PCH), *s65_cpu_reg(S65_REG_PCL));
            /* Old PC value */
            const word tmp_old_pc = tmp_pc;

            /* Value to add (-128; 127) == A */
            int tmp_val = S65_IS_REG(ad_op_a) ? (int) *s65_cpu_reg(ad_op_a) : (int) ad_op_a;

            /* Translating the value U2 -> +-decimal */
            tmp_val = (tmp_val >= 128) ? (tmp_val - 256) : tmp_val;

            /* Operation */
            tmp_pc += tmp_val;
            
            /* Putting the value back */
            *s65_cpu_reg(S65_REG_PCH) = S65_HIGH(tmp_pc);
            *s65_cpu_reg(S65_REG_PCL) = S65_LOW(tmp_pc);

            /* Conditions */
            is_cond_met = true;
            is_page_crossed = s65_is_page_crossed(tmp_pc, tmp_old_pc);
            break;
        }

        /* PC (2-byte) U2 addition with C */
        /* Does not set flags */
        /* Can cross pages */
        /* Condition always true */
        case S65_OP_PC_AD2C:
        {
            /* Obtaining current PC value */
            word tmp_pc = S65_PACK(*s65_cpu_reg(S65_REG_PCH), *s65_cpu_reg(S65_REG_PCL));
            /* Old PC value */
            const word tmp_old_pc = tmp_pc;

            /* Value to add (-128; 127) == A */
            int tmp_val = S65_IS_REG(ad_op_a) ? (int) *s65_cpu_reg(ad_op_a) : (int) ad_op_a;

            /* Translating the value U2 -> +- decimal */
            tmp_val = (tmp_val >= 128) ? (tmp_val - 256) : tmp_val;

            /* Operation */
            tmp_pc += tmp_val + !!s65_cpu_is_flag(S65_SREG_C);
            
            /* Putting the value back */
            *s65_cpu_reg(S65_REG_PCH) = S65_HIGH(tmp_pc);
            *s65_cpu_reg(S65_REG_PCL) = S65_LOW(tmp_pc);

            /* Conditions */
            is_cond_met = true;
            is_page_crossed = s65_is_page_crossed(tmp_pc, tmp_old_pc);
            break;
        }

        /* Adding B to A with C */
        /* Manages: C, N, V, Z */
        /* Can cross pages */
        /* Condition always true */
        case S65_OP_ADC:
        {
            /* Old A value */
            const byte tmp_old_a = *s65_cpu_reg(ad_op_a);

            /* Value to be added (B) */
            byte tmp_val = S65_IS_REG(ad_op_b) ? *s65_cpu_reg(ad_op_b) : (byte) ad_op_b;

            /* Temporary 2-byte addition value */
            word tmp_result = 0;

            /* Converting from BCD if D set */
            if(s65_cpu_is_flag(S65_SREG_D))
            {
                tmp_val                 = S65_FROM_BCD(tmp_val);
                *s65_cpu_reg(ad_op_a)   = S65_FROM_BCD(*s65_cpu_reg(ad_op_a));
            }

            /* Operation */
            tmp_result = (word) *s65_cpu_reg(ad_op_a) + (word) tmp_val + (word) !!s65_cpu_is_flag(S65_SREG_C);
            //*s65_cpu_reg(ad_op_a) += tmp_val + !!s65_cpu_is_flag(S65_SREG_C);

            /* Managing flags */

            /* Carry flag */
            if(tmp_result > (byte) -1)
                s65_cpu_set_flag(S65_SREG_C);
            else
                s65_cpu_clr_flag(S65_SREG_C);

            /* Zero flag */
            if(tmp_result == 0u)
                s65_cpu_set_flag(S65_SREG_Z);
            else
                s65_cpu_cls_flag(S65_SREG_Z);

            /* Negative flag */
            if(tmp_result & (1 << 7))
                s65_cpu_cls_flag(S65_SREG_Z);

            

            else if((int) tmp_old_a + (int) tmp_val != (int) tmp_result)
            {
                /* Overflow flag */
                s65_cpu_set_flag(S65_SREG_N);
            }

            /* Converting back to BCD if D set */
            if(s65_cpu_is_flag(S65_SREG_D))
            {
                tmp_val                 = S65_TO_BCD(tmp_val);
                *s65_cpu_reg(ad_op_a)   = S65_TO_BCD(*s65_cpu_reg(ad_op_a));
            }

            break;
        }

        /* ??? */
        default:
        {
            assert("");
        }
    }

    END:;

    return 0;
}

/* Gets a register.
 *
 * @param ad_reg        the register virtual address 
 * 
 * @returns The register.
 */
byte *s65_cpu_reg(addr_t ad_reg)
{
    assert(S65_IS_REG(ad_reg));
    assert(g_registers);

    return &g_registers->pb_block[S65_ADDR_TO_REG(ad_reg)];
}

/* Gets flag value in SREG. 
 *
 * @param b_flag        the flag (S65_SREG_*)
 * 
 * @returns True if flag set (1), false
 * otherwise.
 */
bool s65_cpu_is_flag(byte b_flag)
{
    assert(g_registers);
    return (*s65_cpu_reg(S65_REG_SREG) & (1 << b_flag));
}

/* Sets flag in SREG.
 *
 * @param b_flag        the flag (S65_SREG_*)
 */
void s65_cpu_set_flag(byte b_flag)
{
    assert(g_registers);
    *s65_cpu_reg(S65_REG_SREG) |= (1 << b_flag);
}

/* Clears flag in SREG.
 *
 * @param b_flag        the flag (S65_SREG_*)
 */
void s65_cpu_clr_flag(byte b_flag)
{
    assert(g_registers);
    *s65_cpu_reg(S65_REG_SREG) &= ~(1 << b_flag);
}
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
 * @returns Zero value if
 * succeeded.
 */
int s65_cpu_init(void)
{
    /* Allocating register memory */
    if((g_registers = s65_new_block(S65_MEM_REGSPACE, 0xFF)) == NULL)
    {
        /* Failed */
        return EXIT_FAILURE;
    }

    /* OK */
    return EXIT_SUCCESS;
}

/* Resets the CPU.
 * Engages RESET sequence.
 *
 * @returns Number of cycles executed.
 */
size_t s65_cpu_reset(void)
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
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_SP,         0x00,           d_cycle, S65_OPFLAG_READ                   );
        /* Setting PC to 0x00FF */
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_PCL,        0xFF,           d_cycle, S65_OPFLAG_READ                   );
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_PCH,        0x00,           d_cycle, S65_OPFLAG_READ                   );
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
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_ABH,       0x01,            d_cycle, S65_OPFLAG_READ                   );

        /* [T4] */ d_cycle++;

        /* Decrementing SP (0x00 --> 0xFF / 0x100 --> 0x1FF) */
        s65_op_add(rop_reset, S65_OP_DEC,       S65_REG_SP,        S65_REG_NULL,    d_cycle, S65_OPFLAG_READ                   );
        /* Setting address bus to 9-bit SP (0x1FF) and discarding */
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_ABL,       S65_REG_SP,      d_cycle, S65_OPFLAG_READ                   );
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_ABH,       0x01,            d_cycle, S65_OPFLAG_READ                   );

        /* [T5] */ d_cycle++;

        /* Decrementing SP (0xFF --> 0xFE / 0x1FF --> 0x1FE) */
        s65_op_add(rop_reset, S65_OP_DEC,       S65_REG_SP,        S65_REG_NULL,    d_cycle, S65_OPFLAG_READ                   );
        /* Setting address bus to 9-bit SP (0x1FE) and discarding */
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_ABL,       S65_REG_SP,      d_cycle, S65_OPFLAG_READ                   );
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_ABH,       0x01,            d_cycle, S65_OPFLAG_READ                   );

        /* [T6] */ d_cycle++;

        /* Decrementing SP (0xFE --> 0xFD / 0x1FE --> 0x1FD) */
        s65_op_add(rop_reset, S65_OP_DEC,       S65_REG_SP,        S65_REG_NULL,    d_cycle, S65_OPFLAG_READ                   );
        /* Setting address bus to reset vector (0xFFFC) */
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_ABL,       S65_LOW(S65_VECTOR_RES),  d_cycle, S65_OPFLAG_READ          );
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_ABH,       S65_HIGH(S65_VECTOR_RES), d_cycle, S65_OPFLAG_READ          );
        /* Reading address low */
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_ADL,       S65_REG_DATA,    d_cycle, S65_OPFLAG_READ                   );

        /* [T7] */ d_cycle++;

        /* Setting address bus to reset vector + 1 (0xFFFD) */
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_ABL,       S65_LOW(S65_VECTOR_RES + 1u),  d_cycle, S65_OPFLAG_READ     );
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_ABH,       S65_HIGH(S65_VECTOR_RES + 1u), d_cycle, S65_OPFLAG_READ     );
        /* Reading address high */
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_ADH,       S65_REG_DATA,    d_cycle, S65_OPFLAG_READ                   );
        /* Setting I flag */
        s65_op_add(rop_reset, S65_OP_SET,       S65_REG_SREG,      S65_SREG_I,      d_cycle, S65_OPFLAG_READ                   );
        /* Clearing D flag */
        s65_op_add(rop_reset, S65_OP_CLEAR,     S65_REG_SREG,      S65_SREG_D,      d_cycle, S65_OPFLAG_READ                   );

        /* [T0] */

        /* Updating PC */
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_PCL,        S65_REG_ADL,    0      , S65_OPFLAG_READ                   );
        s65_op_add(rop_reset, S65_OP_LOAD,      S65_REG_PCH,        S65_REG_ADH,    0      , S65_OPFLAG_READ                   );

        /* Instruction execution at PC */
    }

    /* RESET procedure is defined */
    /* it can be executed */

    for(size_t i = 0u; i < rop_reset->sz_count; ++i)
    {
        s65_cpu_exe(&rop_reset->op_list[i]);
    }

    /* Updating the state */
    g_state = S65_RUNNING;

    /* Returning # of cycles of RESET */
    return s65_instr_cycles(rop_reset);
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

    /* Internal carry flag, used to calculate addresses,
     * unavailable for a programmer; marked as C'
     */
    static int d_internal_c = 0; 

    /* First operand (A) */
    const addr_t ad_op_a = op->ad_first;
    /* Second operand (B) */
    const addr_t ad_op_b = op->ad_secnd;

    /* Executing */

    /* Checking conditions */
    if((op->d_flags & S65_OPFLAG_IF_COND && ! is_cond_met) ||
       (op->d_flags & S65_OPFLAG_IF_NOT_COND && is_cond_met))
    {
        /* Skipping */
        return S65_CPURET_SKIPPED;
    }
    if((op->d_flags & S65_OPFLAG_IF_CROSSED && ! is_page_crossed) ||
       (op->d_flags & S65_OPFLAG_IF_NOT_CROSSED && is_page_crossed))
    {
        /* Skipping */
        return S65_CPURET_SKIPPED;
    }

    /* Selecting the operation type */
    switch(op->ot_type)
    {
        /* Fetching OPCODE.
         *
         * Flags:               none
         * Condition:           no
         * Page crossing:       no
         */
        case S65_OP_FETCH:
        {
            /* The OPCODE value */
            /* byte tmp_opcode = S65_IS_REG(ad_op_a) ? *s65_cpu_reg(ad_op_a) : (byte) ad_op_a; */

            /* Doing nothing */

            break;
        }

        /* A (reg) is loaded with B (reg/val).
         *
         * Flags:               N, Z, if (A == ACC/X/Y)
         * Condition:           no
         * Page crossing:       no
         */
        case S65_OP_LOAD:
        {
            /* Value to load */
            byte tmp_val = 0;

            /* Interpreting B */
            tmp_val = S65_IS_REG(ad_op_b) ? *s65_cpu_reg(ad_op_b) : ad_op_b;
            printf("%d: LOADING %s with %u\n", op->d_cycle, s65_reg_to_asci(ad_op_a, 1), tmp_val);

            /* Operation (A is a register) */
            *s65_cpu_reg(ad_op_a) = tmp_val;

            /* Managing N, Z */
            if(ad_op_a == S65_REG_ACC ||
               ad_op_a == S65_REG_X   ||
               ad_op_a == S65_REG_Y)
            {
                /* N flag (B < 0) */
                if((sbyte) tmp_val < 0)
                    s65_cpu_set_flag(S65_SREG_N);
                else
                    s65_cpu_clr_flag(S65_SREG_N);

                /* Z flag (B == 0) */
                if(tmp_val == 0)
                    s65_cpu_set_flag(S65_SREG_Z);
                else
                    s65_cpu_clr_flag(S65_SREG_Z);
            }

            break;
        }

        /* PC (2 bytes) is incremented.
         *
         * Flags:               none
         * Condition:           no
         * Page crossing:       yes
         */
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
            is_page_crossed = s65_is_page_crossed(tmp_pc, tmp_old_pc);
            break;
        }

        /* B (val/reg in U2) added to A (reg) without/with C'.
         * Used for address calculation (branching).
         *
         * Flags:               C'
         * Condition:           no
         * Page crossing:       yes
         */
        case S65_OP_AD2:
        case S65_OP_AD2C:
        {
            /* Obtaining current A value */
            word tmp_a = *s65_cpu_reg(ad_op_a);

            /* Old A value */
            const word tmp_old_a = tmp_a;

            /* Value to add (-128; 127) == B */
            int tmp_b = S65_IS_REG(ad_op_b) ? (int) *s65_cpu_reg(ad_op_b) : (int) ad_op_b;

            /* Translating the value U2 -> +- decimal */
            tmp_b = (tmp_b >= 128) ? (tmp_b - 256) : tmp_b;

            /* Operation (adding C' only if OP_AD2C) */
            tmp_a += tmp_b + (op->ot_type == S65_OP_AD2C) ? d_internal_c : 0;
            
            /* Putting the A value back */
            *s65_cpu_reg(ad_op_a) = (byte) tmp_a;

            /* Calculating C' */
            d_internal_c = !!s65_is_page_crossed(tmp_old_a, tmp_a);

            /* Conditions */
            is_page_crossed = (d_internal_c == 1);
            break;
        }

        /* B (val/reg) added to A (reg) without/with C'.
         * Used for address calculation.
         *
         * Flags:               C'
         * Condition:           no
         * Page crossing:       yes
         */
        case S65_OP_ADD:
        case S65_OP_ADC:
        {
            /* Old A value */
            const byte tmp_old_a = *s65_cpu_reg(ad_op_a);

            /* Value to be added (B) */
            byte tmp_val = S65_IS_REG(ad_op_b) ? *s65_cpu_reg(ad_op_b) : (byte) ad_op_b;

            /* Temporary 2-byte addition value */
            word tmp_result = 0;

            /* Operation (adding C' if OP_ADC) */
            tmp_result = ((word) *s65_cpu_reg(ad_op_a)) + ((word) tmp_val) + (((op->ot_type == S65_OP_ADC) ? ((word) d_internal_c) : 0u));
            
            /* Calculating C' */
            d_internal_c = !!s65_is_page_crossed(tmp_old_a, tmp_result);

            /* Storing back */
            *s65_cpu_reg(ad_op_a) = (byte) tmp_result;

//            printf("ADDING %s ($%02x) to %s ($%02x) = $%02x/$%02x\n", s65_reg_to_asci(ad_op_b, 0), tmp_val, s65_reg_to_asci(ad_op_a, 0), tmp_old_a, *s65_cpu_reg(ad_op_a), tmp_result);

            /* Conditions */
            is_page_crossed = (d_internal_c == 1);
            break;
        }

        /* B (val/reg) added to A (reg) without/with C.
         * Used only by ADC instruction. BCD compatible.
         *
         * Flags:               N, Z, C, V
         * Condition:           no
         * Page crossing:       no
         */
        case S65_OP_ADCC:
        {
            /* Old A value */
            const byte tmp_old_a = *s65_cpu_reg(ad_op_a);

            /* Value to be added (B) */
            byte tmp_val = S65_IS_REG(ad_op_b) ? *s65_cpu_reg(ad_op_b) : (byte) ad_op_b;

            /* Temporary 2-byte addition value */
            word tmp_result = 0;

            /* Operation (adding C too) */
            tmp_result = (word) *s65_cpu_reg(ad_op_a) + (word) tmp_val + (word) !!s65_cpu_is_flag(S65_SREG_C); 

            /* BCD correction if D set */
            if(s65_cpu_is_flag(S65_SREG_D))
            {
                /* Checking if valid (BCD cannot be bigger than 99) */
                if(S65_FROM_BCD(tmp_result) >= 100u)
                {
                    /* Adding 6 to correct it */
                    tmp_result += 6u;
                }
            }

            /* Managing flags */

            /* Carry flag */
            if(tmp_result > 255u)
                s65_cpu_set_flag(S65_SREG_C);
            else 
                s65_cpu_clr_flag(S65_SREG_C);

            /* Zero flag */
            if((byte) tmp_result == 0u)
                s65_cpu_set_flag(S65_SREG_Z);
            else
                s65_cpu_clr_flag(S65_SREG_Z);

            /* Negative flag */
            if((sbyte)((byte) tmp_result) < 0)
                s65_cpu_set_flag(S65_SREG_N);
            else
                s65_cpu_clr_flag(S65_SREG_N);

            /* Overflow flag 
             * (the result sign differs from both operands sign) 
             */
            if((! ((tmp_old_a >> 7) ^ (tmp_val >> 7)) && (tmp_old_a >> 7) != (tmp_result >> 7)))
                s65_cpu_set_flag(S65_SREG_V);
            else
                s65_cpu_clr_flag(S65_SREG_V);

            /* Storing back the value */
            *s65_cpu_reg(ad_op_a) = (byte) tmp_result;
            break;
        }

        /* A (reg) AND B (reg/val) without storing.
         * Used only by BIT instruction.
         *
         * Flags:               N, Z, V
         * Condition:           no
         * Page crossing:       no
         */
        case S65_OP_BIT:
        {
            /* Mask (B) value */
            const byte tmp_val = S65_IS_REG(ad_op_b) ? *s65_cpu_reg(ad_op_b) : (byte) ad_op_b;
            /* Operation result */
            const byte tmp_result = (*s65_cpu_reg(ad_op_a)) & tmp_val;

            /* Managing flags */

            /* Zero flag */
            if(tmp_result == 0u)
                s65_cpu_set_flag(S65_SREG_Z);
            else
                s65_cpu_clr_flag(S65_SREG_Z);

            /* Negative flag
             * (bit 7 of the mask) 
             */
            if(tmp_val & (1 << 7))
                s65_cpu_set_flag(S65_SREG_N);
            else
                s65_cpu_clr_flag(S65_SREG_N);

            /* Overflow flag 
             * (bit 6 of the mask) 
             */
            if(tmp_val & (1 << 6))
                s65_cpu_set_flag(S65_SREG_V);
            else
                s65_cpu_clr_flag(S65_SREG_V);

            break;
        }

        /* Checking if A flag is set in SREG.
         * Used for branching conditions.
         *
         * Flags:               none
         * Condition:           yes
         * Page crossing:       no
         */
        case S65_OP_BIT_SREG:
        {
            /* Checking the flag */
            is_cond_met = s65_cpu_is_flag(ad_op_a);
            break;
        }

        /* Checking if A flag is cleared in SREG.
         * Used for branching conditions.
         *
         * Flags:               none
         * Condition:           yes
         * Page crossing:       no
         */
        case S65_OP_BIT_SREG_NOT:
        {
            /* Checking the flag */
            is_cond_met = ! s65_cpu_is_flag(ad_op_a);
            break;
        }

        /* Comparing A (reg) with B (reg/val).
         * Used by CMP, CPX, CPY instructions.
         *
         * Flags:               N, Z, C 
         * Condition:           no
         * Page crossing:       no
         */
        case S65_OP_CMP:
        {
            /* A value */
            const byte tmp_a = *s65_cpu_reg(ad_op_a);
            /* B value */
            const byte tmp_b = S65_IS_REG(ad_op_b) ? *s65_cpu_reg(ad_op_b) : ad_op_b;

            /* Managing flags */

            /* Carry flag */
            if(tmp_a >= tmp_b)
                s65_cpu_set_flag(S65_SREG_C);
            else 
                s65_cpu_clr_flag(S65_SREG_C);

            /* Zero flag */
            if(tmp_a - tmp_b == 0u)
                s65_cpu_set_flag(S65_SREG_Z);
            else
                s65_cpu_clr_flag(S65_SREG_Z);

            /* Negative flag */
            if((sbyte)(tmp_a - tmp_b) < 0)
                s65_cpu_set_flag(S65_SREG_N);
            else
                s65_cpu_clr_flag(S65_SREG_N);

            break;
        }

        /* Increments/decrements A (reg).
         *
         * Flags:               N, Z if (A != SP)
         * Condition:           no
         * Page crossing:       no            
         */
        case S65_OP_INC:
        case S65_OP_DEC:
        {
            /* Temporary A value */
            byte tmp_a = *s65_cpu_reg(ad_op_a);

            /* Operation (++/--) */
            if(op->ot_type == S65_OP_INC)
                tmp_a++;
            else
                tmp_a--;

            /* Managing flags (skipping if SP) */
            if(ad_op_a == S65_REG_SP)
                goto INC_DEC_SKIP;

            /* Zero flag */
            if(tmp_a == 0u)
                s65_cpu_set_flag(S65_SREG_Z);
            else
                s65_cpu_clr_flag(S65_SREG_Z);

            /* Negative flag */
            if((sbyte) tmp_a < 0)
                s65_cpu_set_flag(S65_SREG_N);
            else
                s65_cpu_clr_flag(S65_SREG_N);

            INC_DEC_SKIP:

            /* Storing back */
            *s65_cpu_reg(ad_op_a) = tmp_a;

            break;
        }

        /* Shifts A (reg) left, bit 7 is put in C.
         * Bit 0 is set to zero. Used by ASL instr.
         *
         * Flags:               N, Z, C
         * Condition:           no
         * Page crossing:       no            
         */
        case S65_OP_ASL:
        {
            /* Temporary A value */
            byte tmp_a = *s65_cpu_reg(ad_op_a);

            /* Managing carry (bit 7) */
            if(tmp_a & (1 << 7))
                s65_cpu_set_flag(S65_SREG_C);
            else
                s65_cpu_clr_flag(S65_SREG_C);

            /* Operation */
            tmp_a <<= 1;

            /* Managing flags */

            /* Zero flag */
            if(tmp_a == 0u)
                s65_cpu_set_flag(S65_SREG_Z);
            else
                s65_cpu_clr_flag(S65_SREG_Z);

            /* Negative flag */
            if((sbyte) tmp_a < 0)
                s65_cpu_set_flag(S65_SREG_N);
            else
                s65_cpu_clr_flag(S65_SREG_N);

            /* Storing back */
            *s65_cpu_reg(ad_op_a) = tmp_a;

            break;
        }

        /* Shifts A (reg) right, bit 0 is put in C.
         * Bit 7 is set to zero. Used by LSR instr.
         *
         * Flags:               N = 0, Z, C
         * Condition:           no
         * Page crossing:       no            
         */
        case S65_OP_LSR:
        {
            /* Temporary A value */
            byte tmp_a = *s65_cpu_reg(ad_op_a);

            /* Managing carry (bit 0) */
            if(tmp_a & (1 << 0))
                s65_cpu_set_flag(S65_SREG_C);
            else
                s65_cpu_clr_flag(S65_SREG_C);

            printf("%d: LSR %s ($%02x) = ", op->d_cycle, s65_reg_to_asci(ad_op_a, 0), tmp_a);

            /* Operation */
            tmp_a >>= 1;

            /* Managing flags */

            /* Zero flag */
            if(tmp_a == 0u)
                s65_cpu_set_flag(S65_SREG_Z);
            else
                s65_cpu_clr_flag(S65_SREG_Z);

            /* Negative flag (always 0) */
            s65_cpu_clr_flag(S65_SREG_N);

            /* Storing back */
            *s65_cpu_reg(ad_op_a) = tmp_a;

            printf("$%02x\n", *s65_cpu_reg(ad_op_a));
            break;
        }

        /* Rotates A (reg) left/right, bit 7/0 is put in C.
         * Bit 0/7 is set to C. Used by ROL/ROR instr.
         *
         * Flags:               N, Z, C
         * Condition:           no
         * Page crossing:       no            
         */
        case S65_OP_ROL:
        case S65_OP_ROR:
        {
            /* Temporary A value */
            byte tmp_a = *s65_cpu_reg(ad_op_a);

            /* Preserving old C before modifying */
            byte old_c = s65_cpu_is_flag(S65_SREG_C) ? 1 : 0;

            /* ROL */
            if (op->ot_type == S65_OP_ROL)
            {
                /* Setting C based on bit 7 (before shift) */
                if (tmp_a & (1 << 7))
                    s65_cpu_set_flag(S65_SREG_C);
                else
                    s65_cpu_clr_flag(S65_SREG_C);

                /* Rotating left, inserting old C into bit 0 */
                tmp_a <<= 1;
                tmp_a |= old_c;
            }

            /* ROR */
            else
            {
                /* Setting C based on bit 0 (before shift) */
                if (tmp_a & (1 << 0))
                    s65_cpu_set_flag(S65_SREG_C);
                else
                    s65_cpu_clr_flag(S65_SREG_C);

                /* Rotating right, inserting old C into bit 7 */
                tmp_a >>= 1;
                tmp_a |= (old_c << 7);
            }

            /* Zero flag */
            if (tmp_a == 0u)
                s65_cpu_set_flag(S65_SREG_Z);
            else
                s65_cpu_clr_flag(S65_SREG_Z);

            /* Negative flag */
            if ((sbyte)tmp_a < 0)
                s65_cpu_set_flag(S65_SREG_N);
            else
                s65_cpu_clr_flag(S65_SREG_N);

            /* Storing back */
            *s65_cpu_reg(ad_op_a) = tmp_a;
            break;
        }

        /* Sets flag B in A (reg). Used by SEC, SED, SEI.
         *
         * Flags:               C, D, I if (A == SREG)
         * Condition:           no
         * Page crossing:       no            
         */
        case S65_OP_SET:
        {
            /* Temporary A value */
            byte tmp_a = *s65_cpu_reg(ad_op_a);
            /* Temporary B value (mask) */
            byte tmp_b = 1 << (S65_IS_REG(ad_op_b) ? *s65_cpu_reg(ad_op_b) : ad_op_b);

            /* Operation */
            tmp_a |= tmp_b;

            /* Storing back */
            *s65_cpu_reg(ad_op_a) = tmp_a;

            break;
        }

        /* Clears flag B in A (reg). Used by CLC, CLD, CLI, CLV.
         *
         * Flags:               C, D, I, V if (A == SREG)
         * Condition:           no
         * Page crossing:       no            
         */
        case S65_OP_CLEAR:
        {
            /* Temporary A value */
            byte tmp_a = *s65_cpu_reg(ad_op_a);
            /* Temporary B value (mask) */
            byte tmp_b = 1 << (S65_IS_REG(ad_op_b) ? *s65_cpu_reg(ad_op_b) : ad_op_b);

            /* Operation */
            tmp_a &= ~tmp_b;

            /* Storing back */
            *s65_cpu_reg(ad_op_a) = tmp_a;

            break;
        }

        /* Substracts B from A with borrow (~C). 
         * Used only by SBC instruction. BCD compatible.
         *
         * Flags:               N, Z, C, V
         * Condition:           no
         * Page crossing:       no
         */
        case S65_OP_SBC:
        {
                /* Temp. A value */
                byte tmp_a = *s65_cpu_reg(ad_op_a);
                /* Old A value */
                const byte tmp_old_a = tmp_a;

                /* Value to be subtracted (B) */
                byte tmp_val = S65_IS_REG(ad_op_b) ? *s65_cpu_reg(ad_op_b) : (byte) ad_op_b;

                /* Temporary 2-byte subtraction value */
                word tmp_result = 0;

                /* Borrow value (1 if C is clear, 0 if set) */
                byte tmp_borrow = !s65_cpu_is_flag(S65_SREG_C);

            /* Decimal mode check */
            if (s65_cpu_is_flag(S65_SREG_D))
            {
                /* Decimal */

                /* Splitting digits into lower and upper nibbles */
                byte al = tmp_a & 0x0F;
                byte ah = (tmp_a >> 4) & 0x0F;
                byte bl = tmp_val & 0x0F;
                byte bh = (tmp_val >> 4) & 0x0F;

                /* Decimal subtraction on lower nibble */
                int dl = al - bl - tmp_borrow;
                int dh = ah - bh;

                /* Correction if lower nibble underflowed */
                if (dl < 0)
                {
                    dl += 10;
                    dh -= 1;
                }

                /* Correction if upper nibble underflowed */
                if (dh < 0)
                {
                    dh += 10;
                    s65_cpu_clr_flag(S65_SREG_C);
                }
                else
                {
                    s65_cpu_set_flag(S65_SREG_C);
                }

                /* Combine result nibbles into one byte */
                tmp_result = ((dh << 4) & 0xF0) | (dl & 0x0F);

            }

            else
            {
                /* Binary */

                /* Subtraction (with borrow) */
                tmp_result = (word) tmp_a - (word) tmp_val - (word) tmp_borrow;

                /* Carry flag */
                if (tmp_a >= (tmp_val + tmp_borrow))
                    s65_cpu_set_flag(S65_SREG_C);
                else 
                    s65_cpu_clr_flag(S65_SREG_C);
            }

            /* Zero flag */
            if((byte) tmp_result == 0u)
                s65_cpu_set_flag(S65_SREG_Z);
            else
                s65_cpu_clr_flag(S65_SREG_Z);

            /* Negative flag */
            if((sbyte)((byte) tmp_result) < 0)
                s65_cpu_set_flag(S65_SREG_N);
            else
                s65_cpu_clr_flag(S65_SREG_N);

            /* Overflow flag 
             * (the result sign differs from A sign, but is the same as B) 
             */
            if (((tmp_a >> 7) != (tmp_val >> 7)) && ((tmp_a >> 7) != (tmp_result >> 7)))
                s65_cpu_set_flag(S65_SREG_V);
            else
                s65_cpu_clr_flag(S65_SREG_V);

            /* Storing back the value */
            *s65_cpu_reg(ad_op_a) = (byte) tmp_result;
            break;
        }

        /* A (reg) AND/OR/XOR B (reg/val) with storing to A. 
         *
         * Flags:               N, Z
         * Condition:           no
         * Page crossing:       no
         */
        case S65_OP_AND:
        case S65_OP_OR:
        case S65_OP_EOR:
        {
            /* Operation result */
            byte tmp_result = 0;
            /* Temp. A value */
            const byte tmp_a = *s65_cpu_reg(ad_op_a);
            /* Mask (B) value */
            const byte tmp_val = S65_IS_REG(ad_op_b) ? *s65_cpu_reg(ad_op_b) : (byte) ad_op_b;
            

            /* Operation (AND/OR/XOR) */
            if(op->ot_type == S65_OP_AND)
                tmp_result = tmp_a & tmp_val;
            else if(op->ot_type == S65_OP_OR)
                tmp_result = tmp_a | tmp_val;
            else
                tmp_result = tmp_a ^ tmp_val;

            /* Managing flags */

            /* Zero flag */
            if(tmp_result == 0u)
                s65_cpu_set_flag(S65_SREG_Z);
            else
                s65_cpu_clr_flag(S65_SREG_Z);

            /* Negative flag
             * (bit 7 of the result) 
             */
            if(tmp_result & (1 << 7))
                s65_cpu_set_flag(S65_SREG_N);
            else
                s65_cpu_clr_flag(S65_SREG_N);

            /* Storing back */
            *s65_cpu_reg(ad_op_a) = tmp_result;
            break;
        }

        /* Dummy operation, no effects.
         *
         * Flags:               none
         * Condition:           no
         * Page crossing:       no
         */
        case S65_OP_NOP:
        {
            break;
        }

        /* Blocks the CPU, used by JAM instruction.
         *
         * Flags:               none
         * Condition:           no
         * Page crossing:       no
         */
        case S65_OP_UNKNOWN:
        {
            /* Updating the state */
            g_state = S65_JAMMED;
            break;
        }

        /* ??? */
        default:
        {
            assert("This should not have happened.");
            break;
        }

        
    }

    return S65_CPURET_EXECUTED;
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
/*
 *  cpu.h
 *
 *  The CPU's role here is to take a list
 *  of operations and execute them one by one,
 *  modyfing the memory space. The list should be
 *  decoded first (s65_decode()). The CPU can both
 *  read and write to external (non-register) memory
 *  space by loading/reading DATA buffer.
 */

#ifndef _S65_CPU_H_FILE_
#define _S65_CPU_H_FILE_

#include "decoder.h"
#include "memory.h"
#include "types.h"

/* CPU useful IO pins */
#define S65_CPUPIN_SO           (1 << 0)    /* Set Overflow flag pin (IN)       */
#define S65_CPUPIN_SYNC         (1 << 1)    /* SYNC pin (OUT)                   */
#define S65_CPUPIN_RW           (1 << 2)    /* Read/Write pin (OUT)             */

/* Pin state manipulators */
#define S65_PIN_SET(reg, p)     ((reg) |= (1 << (p)))
#define S65_PIN_CLR(reg, p)     ((reg) &= ~(1 << (p)))
#define S65_PIN_TGL(reg, p)     ((reg) ^= (1 << (p)))

/* CPU state */
typedef enum _65_cpustate
{
    S65_PREINIT,                            /* Before RESET                     */
    S65_RUNNING,                            /* Normal working state             */
    S65_JAMMED                              /* After executing JAM instruction  */

} cpu_state_t;

/* Sets up CPU variables. 
 *
 * @returns Positive value if
 * succeeded.
 */
int                s65_cpu_init(void);

/* Resets the CPU.
 * Engages RESET sequence.
 *
 * @returns Number of cycles executed.
 */
int                s65_cpu_reset(void);

/* Executes an operation.
 *
 * @param op            the operation
 * 
 * @returns Operation ID if succeeded,
 * zero value otherwise.
 */
int                s65_cpu_exe(const operation_t *op);

/* Gets a register.
 *
 * @param ad_reg        the register virtual address 
 * 
 * @returns The register.
 */
byte              *s65_cpu_reg(addr_t ad_reg);

/* Gets flag value in SREG. 
 *
 * @param b_flag        the flag (S65_SREG_*)
 * 
 * @returns True if flag set (1), false
 * otherwise.
 */
bool               s65_cpu_is_flag(byte b_flag);

/* Sets flag in SREG.
 *
 * @param b_flag        the flag (S65_SREG_*)
 */
void               s65_cpu_set_flag(byte b_flag);

/* Clears flag in SREG.
 *
 * @param b_flag        the flag (S65_SREG_*)
 */
void               s65_cpu_clr_flag(byte b_flag);

#endif /* _S65_CPU_H_FILE_ */
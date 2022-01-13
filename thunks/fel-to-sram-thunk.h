	0xe59f8048, /*        0:    ldr        r8, [pc, #72]                */
	0xe10f2000, /*        4:    mrs        r2, CPSR                     */
	0xe38220c0, /*        8:    orr        r2, r2, #192                 */
	0xe121f002, /*        c:    msr        CPSR_c, r2                   */
	0xe28f403c, /*       10:    add        r4, pc, #60                  */
	0xe4940004, /*       14:    ldr        r0, [r4], #4                 */
	0xe4941004, /*       18:    ldr        r1, [r4], #4                 */
	0xe4946004, /*       1c:    ldr        r6, [r4], #4                 */
	0xe3560000, /*       20:    cmp        r6, #0                       */
	0x0a000006, /*       24:    beq        44 <launch_spl>              */
	0xe5902000, /*       28:    ldr        r2, [r0]                     */
	0xe5913000, /*       2c:    ldr        r3, [r1]                     */
	0xe2566004, /*       30:    subs       r6, r6, #4                   */
	0xe4812004, /*       34:    str        r2, [r1], #4                 */
	0xe4803004, /*       38:    str        r3, [r0], #4                 */
	0x1afffff9, /*       3c:    bne        28 <swap_next_word>          */
	0xeafffff3, /*       40:    b          14 <swap_next_buffer>        */
	0xf57ff04f, /*       44:    dsb        sy                           */
	0xf57ff06f, /*       48:    isb        sy                           */
	0xe12fff18, /*       4c:    bx         r8                           */

/*
 * Copyright © 2015 Siarhei Siamashka <siarhei.siamashka@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*************************************************************************/
/* Usage instructions: "ruby -x fel-to-spl-thunk.S > fel-to-spl-thunk.h" */
/*************************************************************************/

/* Open a comment for gas.

   Do not close the comment until after the Ruby code terminator (__END__).
   Write the '*' '/' sequence of characters as "\x2a/" in string literals to
   avoid doing so.

#!/usr/bin/env ruby

def tool_exists(tool_name)
    `which #{tool_name} > /dev/null 2>&1`
    return $?.to_i == 0
end

toolchains = [
  "arm-none-eabi-",
  "arm-linux-gnueabihf-",
  "arm-none-linux-gnueabi-",
  "armv7a-hardfloat-linux-gnueabi-",
  "armhf-linux-gnueabi-",
  "arm-slackware-linux-gnueabihf-",
]

toolchain = toolchains.find { |toolchain| tool_exists("#{toolchain}as") }
abort "Can't find any ARM crosscompiler\n" unless toolchain

system("#{toolchain}as -o #{$PROGRAM_NAME}.o #{$PROGRAM_NAME}")
exit($?.to_i) if $?.to_i != 0

`#{toolchain}objdump -d #{$PROGRAM_NAME}.o`.each_line {|l|
    next unless l =~ /(\h+)\:\s+(\h+)\s+(\S+)\s+([^;]*)/
    printf("\t0x%s, /* %8s:    %-10s %-28s \x2a/\n", $2, $1, $3, $4.strip)
}

__END__
*/

/*************************************************************************/

BUF1		.req	r0
BUF2		.req	r1
TMP1		.req	r2
TMP2		.req	r3
SWAPTBL		.req	r4
FULLSIZE	.req	r5
BUFSIZE		.req	r6
CHECKSUM	.req	r7
SPL_ADDR	.req	r8

entry_point:
	ldr	SPL_ADDR,  appended_data

	/* Disable IRQ and FIQ */
	mrs	TMP1,      cpsr
	orr	TMP1,      #0xc0
	msr	cpsr_c,    TMP1

	/* A function, which walks the table and swaps all buffers */
swap_all_buffers:
	adr	SWAPTBL,   appended_data + 4
swap_next_buffer:
	ldr	BUF1,      [SWAPTBL],  #4
	ldr	BUF2,      [SWAPTBL],  #4
	ldr	BUFSIZE,   [SWAPTBL],  #4
	cmp	BUFSIZE,   #0
	beq	launch_spl
swap_next_word:
	ldr	TMP1,      [BUF1]
	ldr	TMP2,      [BUF2]
	subs	BUFSIZE,   BUFSIZE,    #4
	str	TMP1,      [BUF2],     #4
	str	TMP2,      [BUF1],     #4
	bne	swap_next_word
	b	swap_next_buffer

launch_spl:
	/* Call the SPL code */
	dsb
	isb
	bx	SPL_ADDR

appended_data:
/*
 * The appended data uses the following format:
 *
 *     struct {
 *         uint32_t          spl_addr;
 *         sram_swap_buffers swaptbl[];
 *     };
 *
 * More details about the 'spl_addr' variable and the 'sram_swap_buffers'
 * struct can be found in the 'fel.c' source file.
 */

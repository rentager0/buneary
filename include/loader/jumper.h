#ifndef _INCLUDE_LOADER_JUMPER_H_
#define _INCLUDE_LOADER_JUMPER_H_


#ifndef __ASSEMBLER__
#  include <loader/gcc.h>
#  include <loader/mistral.h>
#  include <loader/types.h>
#endif


/*
 * The assembly code directly load these constants into segment selector,
 * whereas the C version use array indexes which are transformed by load_xs()
 * functions.
 */

#ifndef __ASSEMBLER__
#  define JUMPER_GDT_ZERO     0
#  define JUMPER_GDT_CODE64   1
#  define JUMPER_GDT_DATA     2
#  define JUMPER_GDT_CODE32   3
#else
#  define JUMPER_GDT_ZERO    (0 << 3)
#  define JUMPER_GDT_CODE64  (1 << 3)
#  define JUMPER_GDT_DATA    (2 << 3)
#  define JUMPER_GDT_CODE32  (3 << 3)
#endif


#ifndef __ASSEMBLER__

__noreturn paddr_t jump_to_payload(paddr_t addr);

void relocate_jumper(paddr_t addr);

size_t jumper_size(void);

#endif


#endif

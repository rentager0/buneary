#ifndef _INCLUDE_LOADER_GCC_H_
#define _INCLUDE_LOADER_GCC_H_


#define __jumper_text    __attribute__ ((section(".jumper-text")))
#define __jumper_data    __attribute__ ((section(".jumper-data")))

#define __mistral        __attribute__ ((section(".mistral")))

#define __packed         __attribute__ ((packed))

#define __noreturn       __attribute__ ((noreturn))


#endif

#ifndef _INCLUDE_LOADER_TYPES_H_
#define _INCLUDE_LOADER_TYPES_H_


/*
 * Some headers are shared between kernel mode and userland.
 * To avoid type multiple definitions, the file compiler for userlands define
 * the __USERLAND__ macro which is detected here.
 */


#ifndef __USERLAND__

  typedef   signed           char   int8_t;
  typedef unsigned           char  uint8_t;
  typedef   signed short     int   int16_t;
  typedef unsigned short     int  uint16_t;
  typedef   signed           int   int32_t;
  typedef unsigned           int  uint32_t;
  typedef   signed long long int   int64_t;
  typedef unsigned long long int  uint64_t;

  typedef uint32_t   size_t;

  #define NULL     ((void *) 0)

#else

  #include <stdint.h>
  #include <stdlib.h>

#endif


typedef uint8_t    bool_t;
#define TRUE     ((bool_t) 1)
#define FALSE    ((bool_t) 0)


typedef uint64_t   vaddr_t;
typedef uint32_t   paddr_t;


typedef uint16_t   port_t;


#endif

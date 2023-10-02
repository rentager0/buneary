#ifndef _INCLUDE_LOADER_PRINTK_H_
#define _INCLUDE_LOADER_PRINTK_H_


#include <loader/stdarg.h>
#include <loader/types.h>


size_t printk(const char *format, ...);

size_t snprintk(char *buffer, size_t len, const char *format, ...);

size_t hprintk(bool_t h(void *, char), void *u, const char *format, ...);


size_t vprintk(const char *format, va_list ap);

size_t vsnprintk(char *buffer, size_t len, const char *format, va_list ap);

size_t vhprintk(bool_t h(void *, char), void *u, const char *fmt, va_list ap);


#endif

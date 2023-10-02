#ifndef _INCLUDE_LOADER_VGA_H_
#define _INCLUDE_LOADER_VGA_H_


#include <loader/types.h>


#define VGA_DEFAULT_VADDR     ((void *) 0xb8000)
#define VGA_DEFAULT_LINES     ((size_t) 25)
#define VGA_DEFAULT_COLUMNS   ((size_t) 80)
#define VGA_DEFAULT_ADDRESS   ((port_t) 0x3d4)
#define VGA_DEFAULT_DATA      ((port_t) 0x3d5)


struct vga
{
	uint16_t  *buffer;
	size_t     lines;
	size_t     columns;
	size_t     cursor;
	port_t     address;
	port_t     data;
};


void vga_initialize(struct vga *screen);

void vga_clear(struct vga *screen);

void vga_putc(struct vga *screen, char c);

void vga_puts(struct vga *screen, const char *str, size_t len);


#endif

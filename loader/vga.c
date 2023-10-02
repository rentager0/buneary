#include <loader/asm.h>
#include <loader/vga.h>


void vga_initialize(struct vga *screen)
{
	screen->buffer = VGA_DEFAULT_VADDR;
	screen->lines = VGA_DEFAULT_LINES;
	screen->columns = VGA_DEFAULT_COLUMNS;
	screen->address = VGA_DEFAULT_ADDRESS;
	screen->data = VGA_DEFAULT_DATA;

	vga_clear(screen);
}

void vga_clear(struct vga *screen)
{
	size_t i, total = screen->lines * screen->columns;
	uint16_t *buffer = screen->buffer;

	for (i = 0; i < total; i++)
		buffer[i] = 0x700;

	screen->cursor = 0;
}

static void vga_update(struct vga *screen)
{
	out8(screen->address, 0x0f);
	out8(screen->data, screen->cursor & 0xff);

	out8(screen->address, 0x0e);
	out8(screen->data, (screen->cursor >> 8) & 0xff);
}

static void vga_scroll(struct vga *screen)
{
	uint16_t *buffer = screen->buffer;
	size_t columns = screen->columns;
	size_t lines = screen->lines;
	size_t line, cursor, end;

	for (line = 0; line < (lines - 1); line++) {
		cursor = line * columns;
		end = cursor + columns;
		while (cursor < end) {
			buffer[cursor] = buffer[cursor + columns];
			cursor++;
		}
	}

	cursor = line * columns;
	end = cursor + columns;

	while (cursor < end) {
		buffer[cursor] = 0x700;
		cursor++;
	}
}

void vga_putc(struct vga *screen, char c)
{
	size_t columns = screen->columns;
	size_t total = screen->lines * columns;
	size_t cursor = screen->cursor;
	uint16_t *buffer = screen->buffer;

	switch (c) {
	case '\n':
		cursor += columns;
	case '\r':
		cursor = (cursor / columns) * columns;
		break;
	case '\t':
		cursor = ((cursor + 1) & (~0x7)) + 8;
		break;
	default:
		buffer[cursor] = 0x0700 | c;
		cursor++;
		break;
	}

	if (cursor >= total) {
		vga_scroll(screen);
		cursor -= columns;
	}

	screen->cursor = cursor;
	vga_update(screen);
}

void vga_puts(struct vga *screen, const char *str, size_t len)
{
	size_t i;

	for (i = 0; i < len; i++)
		vga_putc(screen, str[i]);
}

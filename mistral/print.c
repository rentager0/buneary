#include "mistral/print.h"

#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>


static void print_prefixed(const char *prefix, const char *format, va_list ap)
{
	printf("%s", prefix);
	vprintf(format, ap);
	printf("\n");
}

static const char *title_prefix(void)
{
	if (isatty(STDOUT_FILENO))
		return "\033[37;1m::\033[0m ";
	return ":: ";
}

static const char *subtitle_prefix(void)
{
	if (isatty(STDOUT_FILENO))
		return "\033[34;1m==>\033[0m ";
	return "==> ";
}

static const char *entry_prefix(void)
{
	if (isatty(STDOUT_FILENO))
		return "\033[32;1m  ->\033[0m ";
	return "  -> ";
}


void print_title(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	print_prefixed(title_prefix(), format, ap);
	va_end(ap);
}

void print_subtitle(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	print_prefixed(subtitle_prefix(), format, ap);
	va_end(ap);
}

void print_entry(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	print_prefixed(entry_prefix(), format, ap);
	va_end(ap);
}

void print_subentry(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	print_prefixed("     ", format, ap);
	va_end(ap);
}


void print_loader_scheme(const struct loader_scheme *scheme)
{
	print_subtitle("Loader scheme");
	print_entry("%-30s%s", "Image path", scheme->image_path);
	print_entry("%-30s0x%lx -> 0x%lx", "Load start",
		    scheme->load_offset, scheme->hkaddr.load_addr);
	print_entry("%-30s0x%lx -> 0x%lx", "Mistral header",
		    scheme->mistral_offset, scheme->mistral_offset
		    - scheme->load_offset + scheme->hkaddr.load_addr);
	print_entry("%-30s0x%lx -> 0x%lx", "Multiboot2 header",
		    scheme->header_offset, scheme->hkaddr.header_addr);
	print_entry("%-30s0x%lx -> 0x%lx", "Load end",
		    scheme->load_end_offset, scheme->hkaddr.load_end_addr);
	print_entry("%-30s0x%lx -> 0x%lx", "Entry address",
		    scheme->hentry.entry_addr
		    - scheme->hkaddr.load_addr
		    + scheme->load_offset,
		    scheme->hentry.entry_addr);
}

void print_payload_scheme(const struct payload_scheme *scheme)
{
	print_subtitle("Payload scheme");
	print_entry("%-30s%s", "Image path", scheme->image_path);
	print_entry("%-30s0x%lx -> 0x%lx", "Load start",
		    scheme->load_offset, scheme->hkaddr.load_addr);
	print_entry("%-30s0x%lx -> 0x%lx (%lu bytes)", "Multiboot2 header",
		    scheme->header_offset, scheme->hkaddr.header_addr,
		    scheme->header_length);
	print_entry("%-30s0x%lx -> 0x%lx", "Load end",
		    scheme->load_end_offset, scheme->hkaddr.load_end_addr);
	print_entry("%-30s0x%lx -> 0x%lx", "Entry address",
		    scheme->hentry.entry_addr
		    - scheme->hkaddr.load_addr
		    + scheme->load_offset,
		    scheme->hentry.entry_addr);
	print_entry("%-30s0x%lx", "Bss end",
		    scheme->hkaddr.bss_end_addr);
}

void print_basic_scheme(const struct basic_scheme *scheme)
{
	off_t memory_shift = scheme->hkaddr.load_addr;
	
	print_subtitle("Output scheme");
	print_entry("%-30s0x%lx -> 0x%lx", "Loader start",
		    0, memory_shift);
	print_entry("%-30s0x%lx -> 0x%lx", "Mistral header",
		    scheme->mistral_offset,
		    scheme->mistral_offset + memory_shift);
	print_entry("%-30s0x%lx -> 0x%lx (%lu bytes)", "Multiboot2 header",
		    scheme->header_offset, scheme->hkaddr.header_addr,
		    scheme->header_length);
	print_entry("%-30s0x%lx -> 0x%lx (%lu bytes)", "Payload start",
		    scheme->payload_offset,
		    scheme->payload_offset + memory_shift,
		    scheme->payload_length);
	print_entry("%-30s0x%lx -> 0x%lx", "Load end",
		    scheme->payload_offset + scheme->payload_length,
		    scheme->hkaddr.load_end_addr);
	print_entry("%-30s0x%lx -> 0x%lx", "Entry address",
		    scheme->hentry.entry_addr - memory_shift,
		    scheme->hentry.entry_addr);
	print_entry("%-30s0x%lx", "Stack top address",
		    scheme->mistral.stack_addr);
}

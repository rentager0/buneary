#ifndef _INCLUDE_MISTRAL_PAYLOAD_H_
#define _INCLUDE_MISTRAL_PAYLOAD_H_


#include <unistd.h>

#include "mistral/multiboot2.h"


struct payload_scheme
{
	const char  *image_path;    /* file path */
	
	off_t   header_offset;      /* multiboot2 header offset */
	off_t   hkaddr_offset;      /* multiboot2 address tag offset */
	off_t   hentry_offset;      /* multiboot2 entry tag offset */
	size_t  header_length;      /* multiboot2 header size */
	off_t   load_offset;        /* file offset for hkaddr.load_addr */
	off_t   load_end_offset;    /* file offset for hkaddr.load_end_addr */

	struct multiboot2_tag_address  hkaddr;  /* address tag */
	struct multiboot2_tag_entry    hentry;  /* entry tag */
};


/*
 * Fill the dest payload scheme with information from the parsed file at the
 * given path.
 * Return 0 if all information has been found, -1 otherwise.
 */
int parse_payload(struct payload_scheme *dest, const char *path);

/*
 * Display the payload scheme on the standard output.
 */
void display_payload(const struct payload_scheme *scheme);


#endif

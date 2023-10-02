#ifndef _INCLUDE_MISTRAL_BASIC_H_
#define _INCLUDE_MISTRAL_BASIC_H_


#include <unistd.h>

#include "mistral/loader.h"
#include "mistral/multiboot2.h"
#include "mistral/pagetable.h"
#include "mistral/payload.h"


struct basic_scheme
{
	const struct payload_scheme  *payload;
	const struct loader_scheme   *loader;
	
	/*
	 * Loader starts at offset 0, and contains the mistral and multiboot2
	 * headers.
	 * The multiboot2 header is the copy of the payload header except for
	 * the address and entry tags, it replaces the loader header.
	 *
	 * TODO:
	 * Because the payload header can be larger than the loader header,
	 * the header_length bytes starting at header_offset are copied in
	 * a safe zone and are restored by the loader bootstrap code (which is
	 * located before the header_offset)
	 */

	off_t   mistral_offset;  /* offset of the mistral header */

	off_t   header_offset;   /* offset of the header */
	off_t   hkaddr_offset;   /* offset of the header address tag */
	off_t   hentry_offset;   /* offset of the header entry tag */
	size_t  header_length;   /* header size */

	/*
	 * Payload is loaded right after the loader.
	 * It is loaded by the bootloader as a data of the loader.
	 */

	off_t   payload_offset;   /* offset of the payload */
	size_t  payload_length;   /* size of the payload */

	struct mistral_header          mistral;  /* mistral header */
	struct multiboot2_tag_address  hkaddr;   /* address tag */
	struct multiboot2_tag_entry    hentry;   /* entry tag */
};


int initialize_basic(struct basic_scheme *self,
		     const struct payload_scheme *payload,
		     const struct loader_scheme *loader);

int write_basic(const struct basic_scheme *self, int fd);


#endif

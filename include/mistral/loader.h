#ifndef _INCLUDE_MISTRAL_LOADER_H_
#define _INCLUDE_MISTRAL_LOADER_H_


#include <unistd.h>

#include "mistral/mistral.h"
#include "mistral/multiboot2.h"


#define LOADER_BASENAME  "loader"


struct loader_scheme
{
	const char  *image_path;  /* file path */

	off_t  mistral_offset;     /* mistral header offset */
	off_t  header_offset;      /* multiboot2 header offset */
	off_t  load_offset;        /* file offset for hkaddr.load_addr */
	off_t  load_end_offset;    /* file offset for hkaddr.load_end_addr */

	struct multiboot2_tag_address  hkaddr;  /* address tag */
	struct multiboot2_tag_entry    hentry;  /* entry tag */
};


/*
 * Find the loader path.
 * Search in the given search directory list (each directory is separated by
 * a colon) a file named LOADER_BASENAME and return the first found one.
 * Return NULL if no loader is found. If found, the returned string is
 * allocated on the head and should be free by the caller.
 */
char *find_loader(const char *search);

/*
 * Fill the dest loader scheme with information from the parsed file at the
 * given path.
 * Return 0 if all information has been found, -1 otherwise.
 */
int parse_loader(struct loader_scheme *dest, const char *path);


#endif

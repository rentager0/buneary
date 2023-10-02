#include "mistral/mistral.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


static off_t mistral_search(const char *buffer, size_t len)
{
	struct mistral_header *header;
	size_t i;

	for (i = 0; i <= len - sizeof (*header); i += MISTRAL_ALIGN) {
		header = (struct mistral_header *) (buffer + i);

		if (header->magic != MISTRAL_MAGIC)
			continue;
		if (header->payload_addr != 0)
			continue;
		if (header->load_addr != 0)
			continue;
		if (header->load_end_addr != 0)
			continue;
		if (header->bss_end_addr != 0)
			continue;
		if (header->payload_entry_addr != 0)
			continue;

		return (off_t) i;
	}

	return -1;
}

off_t mistral_lseek(int fd)
{
	off_t off, ret = -1;
	char *buffer;
	ssize_t rd;
	size_t rem;

	buffer = malloc(MISTRAL_SEARCH);
	if (buffer == NULL)
		goto out;

	off = 0;
	rem = MISTRAL_SEARCH;
	while (rem > 0) {
		rd = pread(fd, buffer, rem, off);
		if (rd == 0)
			break;
		if (rd == -1)
			goto out_free;
		off += rd;
		rem -= rd;
	}

	ret = mistral_search(buffer, MISTRAL_SEARCH - rem);
 out_free:
	free(buffer);
 out:
	return ret;
}

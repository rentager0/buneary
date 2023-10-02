#define _GNU_SOURCE

#include "mistral/loader.h"

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "mistral/mistral.h"
#include "mistral/multiboot2.h"


static char *find_loader_dir(const char *dir)
{
	DIR *fh = opendir(dir);
	struct dirent *dirent;
	char *ret = NULL;
	size_t dlen, klen;

	if (fh == NULL)
		return ret;

	while ((dirent = readdir(fh)) != NULL) {
		if (dirent->d_type != DT_REG)
			continue;
		if (strcmp(dirent->d_name, LOADER_BASENAME))
			continue;

		dlen = strlen(dir);
		klen = strlen(LOADER_BASENAME);
		ret = malloc(dlen + 1 + klen + 1);
		if (ret == NULL) {
			perror("");
			exit(EXIT_FAILURE);
		}

		memcpy(ret, dir, dlen);
		ret[dlen] = '/';
		memcpy(ret + dlen + 1, LOADER_BASENAME, klen);
		ret[dlen + 1 + klen] = '\0';

		break;
	}

	closedir(fh);
	return ret;
}

char *find_loader(const char *search)
{
	const char *dir;
	char *copy = strdupa(search);
	char *str, *save;
	char *ret = NULL;

	str = copy;
	while ((dir = strtok_r(str, ":", &save)) != NULL) {
		str = NULL;
		ret = find_loader_dir(dir);
		if (ret != NULL)
			break;
	}

	return ret;
}

static int parse_multiboot2(struct loader_scheme *dest, const char *buffer)
{
	const struct multiboot2_tag *tag;
	const struct multiboot2_tag_address *hkaddr = NULL;
	const struct multiboot2_tag_entry *hentry = NULL;
	const struct multiboot2_header *header =
		(const struct multiboot2_header *) buffer;
	size_t off, len = header->header_length;

	off = sizeof (*header);
	while (off + sizeof (*tag) <= len) {
		tag = (const struct multiboot2_tag *) (buffer + off);
		if (off + tag->size > len)
			break;

		switch (tag->type) {
		case MULTIBOOT2_TAG_ADDRESS:
			hkaddr = (const struct multiboot2_tag_address *) tag;
			dest->hkaddr = *hkaddr;
			break;
		case MULTIBOOT2_TAG_ENTRY:
			hentry = (const struct multiboot2_tag_entry *) tag;
			dest->hentry = *hentry;
			break;
		}

		off += tag->size;
		if ((off & 0x7) != 0)
			off = (off & ~0x7) + 8;
	}

	if (hkaddr == NULL)
		return -1;
	if (hentry == NULL)
		return -1;
	return 0;
}

int parse_loader(struct loader_scheme *dest, const char *path)
{
	struct multiboot2_header header;
	char *full_header;
	int ret = -1;
	off_t off;
	int fd;

	fd = open(path, O_RDONLY);
	if (fd == -1)
		goto err;

	dest->image_path = path;

	dest->mistral_offset = mistral_lseek(fd);
	if (dest->mistral_offset == -1)
		goto err_close;

	dest->header_offset = multiboot2_lseek(fd, &header, sizeof (header));
	if (dest->header_offset == -1)
		goto err_close;

	full_header = malloc(header.header_length);
	if (full_header == NULL) {
		perror("");
		exit(EXIT_FAILURE);
	}

	off = multiboot2_lseek(fd, full_header, header.header_length);
	if (off == -1)
		goto err_free;

	ret = parse_multiboot2(dest, full_header);
	if (ret != 0)
		goto err_free;

	dest->load_offset = dest->header_offset
		+ dest->hkaddr.load_addr
		- dest->hkaddr.header_addr;

	dest->load_end_offset = dest->header_offset
		+ dest->hkaddr.load_end_addr
		- dest->hkaddr.header_addr;

 err_free:
	free(full_header);
 err_close:
	close(fd);
 err:
	return ret;
}

#include "mistral/basic.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "loader/mistral.h"

#include "mistral/loader.h"
#include "mistral/multiboot2.h"
#include "mistral/payload.h"


#define IO_BUFFER_SIZE       4096


static void initialize_memory_layout(struct basic_scheme *self)
{
	const struct payload_scheme *payload = self->payload;
	const struct loader_scheme *loader = self->loader;
	
	size_t payload_size = payload->hkaddr.load_end_addr
		- payload->hkaddr.load_addr;

	/* Use the same memory layout that the loader */
	self->hkaddr = loader->hkaddr;
	self->hentry = loader->hentry;

	/* Then add the payload at the end of the loader */
	self->mistral.payload_addr = self->hkaddr.load_end_addr;
	self->hkaddr.load_end_addr += payload_size;
	self->hkaddr.bss_end_addr = self->hkaddr.load_end_addr;

	/* Then add the aligned loader stack */
	self->hkaddr.bss_end_addr += LOADER_STACK_SIZE;
	self->hkaddr.bss_end_addr =
		LOADER_STACK_ALIGN(self->hkaddr.bss_end_addr);
	self->mistral.stack_addr = self->hkaddr.bss_end_addr;
}

static void initialize_file_layout(struct basic_scheme *self)
{
	const struct loader_scheme *loader = self->loader;
	const struct payload_scheme *payload = self->payload;
	
	off_t loader_shift = loader->load_offset;
	off_t memory_shift = loader->hkaddr.load_addr;
	size_t payload_size = payload->hkaddr.load_end_addr
		- payload->hkaddr.load_addr;

	self->mistral_offset = loader->mistral_offset - loader_shift;
	self->header_offset = loader->header_offset - loader_shift;
	self->header_length = payload->header_length;
	self->payload_offset = loader->hkaddr.load_end_addr - memory_shift;
	self->payload_length = payload_size;

	self->hkaddr_offset = self->header_offset
		+ payload->hkaddr_offset
		- payload->header_offset;
	
	self->hentry_offset = self->header_offset
		+ payload->hentry_offset
		- payload->header_offset;
}

static void initialize_mistral_header(struct basic_scheme *self)
{
	const struct payload_scheme *payload = self->payload;

	self->mistral.magic = MISTRAL_MAGIC;
	self->mistral.load_addr = payload->hkaddr.load_addr;
	self->mistral.load_end_addr = payload->hkaddr.load_end_addr;
	self->mistral.bss_end_addr = payload->hkaddr.bss_end_addr;
	self->mistral.payload_entry_addr = payload->hentry.entry_addr;
	self->mistral.payload_vaddr = payload->hkaddr.load_addr;
}

int initialize_basic(struct basic_scheme *self,
		     const struct payload_scheme *payload,
		     const struct loader_scheme *loader)
{
	self->payload = payload;
	self->loader = loader;

	initialize_memory_layout(self);
	initialize_file_layout(self);
	initialize_mistral_header(self);
	
	return 0;
}

static ssize_t fdcpy(int dest, int src, size_t len)
{
	char buffer[IO_BUFFER_SIZE];
	ssize_t done, ret = -1;
	ssize_t written = 0;
	size_t off, l;

	while (len > 0) {
		l = IO_BUFFER_SIZE;
		if (l > len)
			l = len;

		off = 0;
		while (off < l) {
			done = read(src, buffer + off, l - off);
			if (done == -1) {
				if (errno == EINTR)
					continue;
				goto err;
			}
			if (done == 0)
				break;
			off += (size_t) done;
		}

		off = 0;
		while (off < l) {
			done = write(dest, buffer + off, l - off);
			if (done <= 0) {
				if (errno == EINTR)
					continue;
				goto err;
			}
			off += (size_t) done;
		}

		written += l;
		len -= l;
	}

	ret = written;
 err:
	return ret;
}

int write_basic(const struct basic_scheme *self, int ofd)
{
	int pfd, lfd;
	int ret = -1;

	pfd = open(self->payload->image_path, O_RDONLY);
	if (pfd == -1)
		goto err;

	lfd = open(self->loader->image_path, O_RDONLY);
	if (lfd == -1)
		goto err_close_pfd;

	/* Copy the loader at offset 0 */
	lseek(lfd, self->loader->load_offset, SEEK_SET);
	fdcpy(ofd, lfd, self->payload_offset);

	/* Then concatenate the payload */
	lseek(pfd, self->payload->load_offset, SEEK_SET);
	fdcpy(ofd, pfd, self->payload_length);

	/* Come back to set mistral header fields */
	lseek(ofd, self->mistral_offset, SEEK_SET);
	write(ofd, &self->mistral, sizeof (self->mistral));

	/* Come back to multiboot2 header offset and copy payload's */
	lseek(ofd, self->header_offset, SEEK_SET);
	lseek(pfd, self->payload->header_offset, SEEK_SET);
	fdcpy(ofd, pfd, self->header_length);

	/* Come back to address and entry tags and modify them */
	lseek(ofd, self->hkaddr_offset, SEEK_SET);
	write(ofd, &self->hkaddr, sizeof (self->hkaddr));
	lseek(ofd, self->hentry_offset, SEEK_SET);
	write(ofd, &self->hentry, sizeof (self->hentry));

	close(lfd);
 err_close_pfd:
	close(pfd);
 err:
	return ret;
}

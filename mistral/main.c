#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "mistral/basic.h"
#include "mistral/loader.h"
#include "mistral/multiboot2.h"
#include "mistral/payload.h"
#include "mistral/print.h"


#define DEFAULT_LOADER_PATH   (			    \
	"/usr/share/mistral"			    \
	":/usr/local/share/mistral"		    \
	":.mistral")


const char  *progname = "mistral";


struct args
{
	int          print_schemes;
	int          payload_vaddr_set;
	vaddr_t      payload_vaddr;
	const char  *input;
	const char  *output;
	const char  *loader_path;
};


static void usage(void)
{
	printf("Usage: %s [--version] [--help] [-C <path>] [-o <path>] [-a "
	       "<addr>]\n"
	       "          [--print-schemes] <kernel>\n\n"
	       "Add a 64 bits mode loader stub to a multiboot2 compatible "
	       "kernel. The resulting\n"
	       "kernel can be loaded by a multiboot2 compliant bootloader "
	       "with the initial\n"
	       "entry point called by a processor running in 64 bits "
	       "mode.\n\n", progname);
	printf("More precisely, the entry point, as specified by the "
	       "multiboot2 header of the\n"
	       "input kernel, is executed by a processor in 64 bits mode with "
	       "an identity\n"
	       "mapping for all available physical memory and the registers "
	       "in the following\n"
	       "state:\n\n");
	printf("    RAX   contains the magic value '0x36d76289'\n\n"
	       "    RBX   contains the address of the multiboot2 information "
	       "structure\n\n"
	       "    RCX   contains the first address following the memory "
	       "reclaimable block\n\n"
	       "    RDX   contains the first address of the memory "
	       "reclaimable block\n\n"
	       "    CS    indicates a 64 bits read/execute code segment\n\n"
	       "    DS\n"
	       "    SS    indicates a data segment\n\n"
	       "    CR0   bits 0 (PE) and 31 (PG) are set, other bits are "
	       "all undefined\n\n"
	       "    CR3   indicates an address in the memory reclaimable "
	       "block\n\n"
	       "    CR4   bit 5 (PAE) is set, other bits are all undefined\n\n"
	       "    EFER  bits 4 (LME) and 6 (LMA) are set, other bits are "
	       "all udefined\n\n"
	       "    GDTR  may be invalid, the OS should set up its own "
	       "GDT\n\n");
	printf("The major difference with the original multiboot2 "
	       "specification is the memory\n"
	       "reclaimable block delimited by the content of RCX and RDX. "
	       "This memory block\n"
	       "contains data which should not be modified until the OS sets "
	       "its own page\n"
	       "table.\n");
	printf("The available virtual memory is an identity mapping of the "
	       "physical memory.\n"
	       "For each entry of the initial page table, the bits 0 (P), 1 "
	       "(R/W) and 3 (PWT)\n"
	       "are set, the other bits are undefined. The usage of large "
	       "pages is undefined.\n\n");
	printf("Additionally, the option '-a' allows to indicate another "
	       "virtual address where\n"
	       "to map the kernel and its bss. This mapping is added to the "
	       "identity mapping.\n"
	       "If this mapping overlap with the identity mapping, then it "
	       "replaces it.\n");
}

void version(void)
{
	printf("mistral 1.0.0\nGauthier Voron\n<gauthier.voron@mnesic.fr>\n");
}

static void error(const char *format, ...)
{
	va_list ap;

	fprintf(stderr, "%s: ", progname);

	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);

	fprintf(stderr, "\nPlease type '%s --help' formmore information\n",
		progname);
	exit(EXIT_FAILURE);
}

static void errorp(const char *format, ...)
{
	va_list ap;
	int err = errno;

	fprintf(stderr, "%s: %s: ", progname, strerror(err));

	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);

	fprintf(stderr, "\nPlease type '%s --help' formmore information\n",
		progname);
	exit(EXIT_FAILURE);
}


static void parse_options(struct args *dest, int *argc, const char ***argv)
{
	char *const *__argv = (char *const *) *argv;
	int __argc = *argc;
	static struct option options[] = {
		{ "help",          no_argument,       0, 'h' },
		{ "version",       no_argument,       0, 'V' },
		{ "output",        required_argument, 0, 'o' },
		{ "print-schemes", no_argument,       0, 's' },
		{ "load-vaddr",    required_argument, 0, 'a' },
		{ "loader-path",   required_argument, 0, 'C' },
		{  NULL,           0,                 0,  0  }
	};
	int index;
	char c, *err;

	while (1) {
		c = getopt_long(__argc, __argv, "hVo:sa:C:", options, &index);
		if (c == -1)
			break;

		switch (c) {
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
		case 'V':
			version();
			exit(EXIT_SUCCESS);
		case 'o':
			if (optarg == NULL)
				error("missing output argument");
			if (dest->output != NULL)
				error("'output' option used more than once");
			dest->output = optarg;
			break;
		case 's':
			if (dest->print_schemes)
				error("'print-schemes' option used more than "
				      "once");
			dest->print_schemes = 1;
			break;
		case 'a':
			if (dest->payload_vaddr_set)
				error("'load-vaddr' option used more than "
				      "once");
			dest->payload_vaddr_set = 1;
			if (optarg[0] == '0' && optarg[1] == 'x')
				dest->payload_vaddr =
					strtoul(optarg + 2, &err, 16);
			else if (optarg[0] == 'x')
				dest->payload_vaddr =
					strtoul(optarg + 1, &err, 16);
			else
				dest->payload_vaddr =
					strtol(optarg, &err, 10);
			if (*err != '\0')
				error("invalid option 'load-vaddr': '%s'",
				      optarg);
			break;
		case 'C':
			if (dest->loader_path)
				error("'loader-path' option used more than "
				      "once");
			dest->loader_path = optarg;
			break;
		}
	}

	*argc = __argc - optind;;
	*argv = (const char **) &__argv[optind];
}

static void parse_arguments(struct args *dest, int *argc, const char ***argv)
{
	int __argc = *argc;
	const char **__argv = *argv;

	if (__argc < 1)
		error("missing input operand");
	dest->input = __argv[0];
	__argc--;
	__argv++;

	*argc = __argc;
	*argv = __argv;
}


int main(int argc, const char **argv)
{
	struct args arguments = { 0 };
	struct payload_scheme pscheme;
	struct loader_scheme kscheme;
	struct basic_scheme oscheme;
	const char *kpath;
	int fd, ret;

	parse_options(&arguments, &argc, &argv);
	parse_arguments(&arguments, &argc, &argv);

	if (arguments.loader_path != NULL)
		kpath = find_loader(arguments.loader_path);
	else
		kpath = find_loader(DEFAULT_LOADER_PATH);
	if (kpath == NULL)
		error("cannot find mistral loader");
	
	ret = parse_loader(&kscheme, kpath);
	if (ret != 0)
		error("invalid mistral loader '%s'", kpath);

	if (arguments.print_schemes)
		print_loader_scheme(&kscheme);
	
	ret = parse_payload(&pscheme, arguments.input);
	if (ret != 0)
		error("invalid input operand '%s'", arguments.input);

	if (arguments.print_schemes)
		print_payload_scheme(&pscheme);

	ret = initialize_basic(&oscheme, &pscheme, &kscheme);
	if (ret != 0)
		error("failed to build output");

	if (arguments.payload_vaddr_set)
		oscheme.mistral.payload_vaddr = arguments.payload_vaddr;

	if (arguments.print_schemes)
		print_basic_scheme(&oscheme);

	if (arguments.output != NULL) {
		fd = open(arguments.output, O_WRONLY|O_TRUNC | O_CREAT, 0644);
		if (fd == -1)
			errorp("cannot open output '%s'", arguments.output);
	} else {
		fd = open("a.bin", O_WRONLY | O_TRUNC | O_CREAT, 0644);
		if (fd == -1)
			errorp("cannot open output 'a.bin'");
	}

	write_basic(&oscheme, fd);

	close(fd);

	return EXIT_SUCCESS;
}

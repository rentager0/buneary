#ifndef _INCLUDE_MISTRAL_MISTRAL_H_
#define _INCLUDE_MISTRAL_MISTRAL_H_


#include <stdint.h>
#include <unistd.h>


#  include "loader/mistral.h"


/*
 * Search a mistral header in the specified file descriptor (from offset 0)
 * and return the offset of the first matching byte of -1 if not found.
 * If not -1, reading from the returned offset can be interpreted as a struct
 * mistral_header.
 */
off_t mistral_lseek(int fd);


#endif

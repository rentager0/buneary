#ifndef _INCLUDE_LOADER_STRING_H
#define _INCLUDE_LOADER_STRING_H_


#include <loader/types.h>



#define ARRAY_SIZE(arr)  (sizeof (arr) / sizeof (*arr))


static inline void *memcpy(void *dest, const void *src, size_t n)
{
	const uint8_t *ptrs = src;
	uint8_t *ptrd = dest;
	uint8_t *end = ptrd + n;

	while (ptrd < end)
		*ptrd++ = *ptrs++;

	return dest;
}

static inline void *memmove(void *dest, const void *src, size_t n)
{
	const uint8_t *ends = ((uint8_t *) src) + n;
	uint8_t *ptrd = dest;
	uint8_t *endd = ptrd + n;

	if (dest == src)
		return dest;
	if (dest < src)
		return memcpy(dest, src, n);

	while (ptrd < endd)
		*--endd = *--ends;

	return dest;
}

static inline void *memset(void *dest, uint8_t c, size_t len)
{
	uint8_t *ptr = dest;
	uint8_t *end = ptr + len;

	while (ptr < end)
		*ptr++ = c;

	return dest;
}

static inline size_t strlen(const char *s)
{
	const char *ptr = s;

	while (*ptr != '\0') {
		ptr++;
	}

	return (ptr - s);
}

static inline int32_t strncmp(const char *a, const char *b, size_t len)
{
	while (len > 0) {
		if (*a != *b)
			return *a - *b;
		if (*a == '\0')
			return 0;
		a++;
		b++;
	}

	return 0;
}


#endif

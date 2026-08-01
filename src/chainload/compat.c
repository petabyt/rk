/* SPDX-License-Identifier: Apache-2.0 */
#include <stddef.h>

size_t strlen(const char *s) {
	const char *p = s;
	while (*p)
		p++;
	return (size_t)(p - s);
}

size_t strnlen(const char *s, size_t maxlen) {
	const char *p = s;

	while (maxlen-- && *p)
		p++;
	return (size_t)(p - s);
}

void *memchr(const void *s_, int c, size_t len) {
	const unsigned char *s = s_;
	while (len--) {
		if (*s == (unsigned char)c)
			return (void *)s;
		s++;
	}
	return NULL;
}

void *memmove(void *dst_, const void *src_, size_t len) {
	unsigned char *dst = dst_;
	const unsigned char *src = src_;
	if (dst < src) {
		while (len--)
			*dst++ = *src++;
	} else if (dst > src) {
		dst += len;
		src += len;
		while (len--)
			*--dst = *--src;
	}
	return dst_;
}

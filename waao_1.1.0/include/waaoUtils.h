#ifndef WAAO_UTILS_H
#define WAAO_UTILS_H

#include <stddef.h>
#include <stdint.h>
#include "waaoDefs.h"

int  waaoStringCompareIgnoreCase(const char *a, const char *b);
void waaoTrimInPlace(char *s);
void waaoToLowerInPlace(char *s);
void waaoToUpperInPlace(char *s);
char *waaoReplaceAlloc(const char *s, const char *find, const char *repl);

void waaoJoinPath(const char *a, const char *b, char out[WAAO_PATH_MAX]);
const char *waaoBasename(const char *path);
const char *waaoExtname(const char *path);

uint32_t waaoCrc32(const void *data, size_t len);
char *waaoBase64Encode(const uint8_t *data, size_t len, size_t *outLen);
uint8_t *waaoBase64Decode(const char *b64, size_t b64len, size_t *outLen);


#endif

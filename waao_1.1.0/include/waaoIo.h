#ifndef WAAO_IO_H
#define WAAO_IO_H

#include <stddef.h>
#include <stdint.h>
#include "waaoDefs.h"

int    waaoFileExists(const char *path);
int    waaoFileSize(const char *path, int64_t *outSize);
int    waaoCreateFile(const char *path);
int    waaoDeleteFile(const char *path);
int    waaoCopyFile(const char *src, const char *dst);
int    waaoMoveFile(const char *src, const char *dst);

int    waaoReadAll(const char *path, WaaoBuffer *out);
int    waaoWriteAll(const char *path, const void *data, size_t len);
int    waaoAppendAll(const char *path, const void *data, size_t len);

int    waaoMakeDir(const char *path);
int    waaoRemoveDir(const char *path);
int    waaoListDir(const char *path, char ***names, size_t *count);
void   waaoListDirFree(char **names, size_t count);

int    waaoDisplayLines(const char *path, int fromStart, size_t n);
size_t waaoFindPattern(const char *path, const char *pattern);
int    waaoCountLines(const char *path, size_t *out);
int    waaoCountWords(const char *path, size_t *out);
int    waaoSortFileLines(const char *inPath, const char *outPath, int ignoreCase);

#endif

#ifndef WAAO_ARCHIVE_H
#define WAAO_ARCHIVE_H

#include <stddef.h>
#include <stdint.h>

/* Minimal custom archive format "WAAO1"
   Layout:
   6 bytes magic "WAAO1\n"
   u32 fileCount (LE)
   Repeated:
     u16 nameLen, name bytes (UTF-8)
     u64 origSize
     u64 compSize
     u32 crc32
     data[compSize]  (simple RLE)
*/

int waaoCompressArchive(const char *archivePath, const char **files, size_t fileCount);
int waaoDecompressArchive(const char *archivePath, const char *outputDir);

#endif

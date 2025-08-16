#ifndef WAAO_CONVERT_H
#define WAAO_CONVERT_H

#include <stddef.h>
#include "waaoDefs.h"


int waaoTextToPdf(const char *inputPath, const char *outputPath);
int waaoCsvToTsv(const char *inputPath, const char *outputPath);
int waaoTsvToCsv(const char *inputPath, const char *outputPath);
int waaoJsonMinify(const char *inputPath, const char *outputPath);
int waaoTextToUpperFile(const char *inputPath, const char *outputPath);
int waaoHexDumpFile(const char *inputPath, const char *outputPath);

#endif

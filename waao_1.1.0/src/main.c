#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "waaoIo.h"
#include "waaoArchive.h"
#include "waaoConvert.h"
#include "waaoUtils.h"
#include "waaoDefs.h"

static void printUsage() {
    printf("WAAO - Command Line File Utility\n");
    printf("Usage:\n");
    printf("  waao display <file> <fromStart:0|1> <n>\n");
    printf("  waao find <pattern> <file>\n");
    printf("  waao compress <archive> <file1> [file2 ...]\n");
    printf("  waao decompress <archive> <outputDir>\n");
    printf("  waao txt2pdf <input.txt> <output.pdf>\n");
    printf("\nExamples:\n");
    printf("  waao display examples/sample.txt 1 5\n");
    printf("  waao find hello examples/sample.txt\n");
    printf("  waao compress archive.txt file1.txt file2.txt\n");
    printf("  waao decompress archive.txt output_dir\n");
    printf("  waao txt2pdf notes.txt notes.pdf\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    if (strcmp(argv[1], "display") == 0 && argc == 5) {
        int fromStart = atoi(argv[3]);
        size_t n = (size_t)atoi(argv[4]);
        return waaoDisplayLines(argv[2], fromStart, n);

    } else if (strcmp(argv[1], "find") == 0 && argc == 4) {
        size_t line = waaoFindPattern(argv[3], argv[2]);
        if (line) {
            printf("Pattern found at line: %zu\n", line);
        } else {
            printf("Pattern not found.\n");
        }
        return 0;

    } else if (strcmp(argv[1], "compress") == 0 && argc >= 4) {
        const char *archivePath = argv[2];
        const char **files = (const char **)&argv[3];
        size_t fileCount = argc - 3;
        return waaoCompressArchive(archivePath, files, fileCount);

    } else if (strcmp(argv[1], "decompress") == 0 && argc == 4) {
        return waaoDecompressArchive(argv[2], argv[3]);

    } else if (strcmp(argv[1], "txt2pdf") == 0 && argc == 4) {
        return waaoTextToPdf(argv[2], argv[3]);

    } else {
        printf("Invalid command or arguments.\n");
        printUsage();
        return 1;
    }

    return 0;
}

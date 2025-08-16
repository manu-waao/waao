---
````markdown
# WAAO Standard — Developer Guide & API Reference

WAAO is a modular **C-based file utility library** with a command-line interface (CLI) and **Python bindings** for seamless integration into Python projects.  
It supports file operations, pattern searching, archive compression/decompression, and text-to-PDF conversion.

---

## 📂 Project Structure
```plaintext
waao/
├── Makefile                # Build rules for C CLI and Python extension
├── setup.py                # Python build script
├── include/                # Header files (public API)
│   ├── waaoArchive.h
│   ├── waaoConvert.h
│   ├── waaoDefs.h
│   ├── waaoIo.h
│   └── waaoUtils.h
├── src/                    # C source files
│   ├── main.c               # CLI frontend
│   ├── waaoArchive.c
│   ├── waaoConvert.c
│   ├── waaoIo.c
│   └── waaoUtils.c
└── pythonBindings/
    └── pyWaao.c             # Python extension wrapper
````

---

## 🚀 Building & Installation

### **Build CLI (C executable)**

```bash
make
./bin/waao
```

### **Build Python module**

```bash
python3 setup.py build
python3 setup.py install --user
```

---

## 🖥️ CLI Usage

```plaintext
waao display <file> <fromStart:0|1> <n>        # Show first/last N lines
waao find <pattern> <file>                     # Search for pattern in file
waao compress <archive> <file1> [file2 ...]    # Compress files into archive
waao decompress <archive> <outputDir>          # Extract files from archive
waao txt2pdf <input.txt> <output.pdf>          # Convert text file to PDF
```

**Examples:**

```bash
waao display examples/sample.txt 1 5
waao find Hello examples/sample.txt
waao compress archive.txt file1.txt file2.txt
waao decompress archive.txt output_dir
waao txt2pdf notes.txt notes.pdf
```

---

## 🐍 Python Usage

```python
import waao

waao.displayLines("examples/sample.txt", 1, 3)
line = waao.findPattern("Hello", "examples/sample.txt")
print("Found at line:", line)
```

---

## 📚 API Reference

### **waaoDefs.h**

| Macro           | Description                       |
| --------------- | --------------------------------- |
| `WAAO_SUCCESS`  | Function success return code (0)  |
| `WAAO_ERROR`    | Function failure return code (-1) |
| `WAAO_PATH_MAX` | Max file path length (4096)       |

---

### **waaoUtils.h**

```c
int waaoStringCompareIgnoreCase(const char *a, const char *b);
```

Compares two strings ignoring case. Returns 0 if equal.

---

### **waaoIo.h**

```c
int waaoDisplayLines(const char *path, int fromStart, size_t n);
size_t waaoFindPattern(const char *path, const char *pattern);
```

* `waaoDisplayLines` — Show first or last N lines of a file.
* `waaoFindPattern` — Find a pattern in a file. Returns line number or 0.

---

### **waaoArchive.h**

```c
int waaoCompressArchive(const char *archivePath, const char **files, size_t fileCount);
int waaoDecompressArchive(const char *archivePath, const char *outputDir);
```

* `waaoCompressArchive` — Create an archive from multiple files.
* `waaoDecompressArchive` — Extract files from an archive.

---

### **waaoConvert.h**

```c
int waaoTextToPdf(const char *inputPath, const char *outputPath);
```

Converts a text file into a minimal PDF.

---

### **Python Bindings**

* `waao.displayLines(path, fromStart, n)`
* `waao.findPattern(pattern, path)`
* `waao.compressArchive(archive, [files])`
* `waao.decompressArchive(archive, outputDir)`
* `waao.textToPdf(inputPath, outputPath)`

---

## ⚙️ Implementation Notes

* **No external C libraries** used — portable code.
* Optimized with `-O3` and `-march=native`.
* Works as both a CLI tool and Python module.

```
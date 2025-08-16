#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdint.h>
#include "waaoIo.h"
#include "waaoArchive.h"
#include "waaoConvert.h"
#include "waaoUtils.h"


static int seqToCStringArray(PyObject *seq, char ***out, size_t *count){
    PyObject *fast = PySequence_Fast(seq, "expected a sequence");
    if(!fast) return 0;
    Py_ssize_t n = PySequence_Fast_GET_SIZE(fast);
    char **arr = (char**)malloc((size_t)n*sizeof(char*));
    if(!arr){ 
        Py_DECREF(fast); 
        return 0; 
    }
    for(Py_ssize_t i=0;i<n;i++){
        PyObject *item = PySequence_Fast_GET_ITEM(fast,i);
        const char *s = PyUnicode_Check(item)? PyUnicode_AsUTF8(item) : NULL;
        if(!s){ 
            free(arr); 
            Py_DECREF(fast); 
            PyErr_SetString(PyExc_TypeError,"list must contain str"); 
            return 0; 
        }
        arr[i]=strdup(s);
    }
    *out=arr; 
    *count=(size_t)n; 
    Py_DECREF(fast); 
    return 1;
}
static void freeCStringArray(char **arr, size_t n){
    if(!arr) return; 
    for(size_t i=0;i<n;i++) free(arr[i]); 
    free(arr);
}


static PyObject *pyFileExists(PyObject *self, PyObject *args){
    const char *path; 
    if(!PyArg_ParseTuple(args,"s",&path)) return NULL;
    return PyBool_FromLong(waaoFileExists(path));
}
static PyObject *pyFileSize(PyObject *self, PyObject *args){
    const char *path; if(!PyArg_ParseTuple(args,"s",&path)) return NULL;
    int64_t sz; 
    if(waaoFileSize(path,&sz)!=WAAO_SUCCESS){ 
        Py_RETURN_NONE; 
    }
    return PyLong_FromLongLong(sz);
}
static PyObject *pyCreateFile(PyObject *s, PyObject *a){ 
    const char *p; 
    if(!PyArg_ParseTuple(a,"s",&p)) return NULL; 
    return PyLong_FromLong(waaoCreateFile(p)); 
}

static PyObject *pyDeleteFile(PyObject *s, PyObject *a){ 
    const char *p; 
    if(!PyArg_ParseTuple(a,"s",&p)) return NULL; 
    return PyLong_FromLong(waaoDeleteFile(p)); 
}

static PyObject *pyCopyFile(PyObject *s, PyObject *a){ 
    const char *src,*dst; 
    if(!PyArg_ParseTuple(a,"ss",&src,&dst)) return NULL; 
    return PyLong_FromLong(waaoCopyFile(src,dst)); 
}
static PyObject *pyMoveFile(PyObject *s, PyObject *a){ 
    const char *src,*dst; 
    if(!PyArg_ParseTuple(a,"ss",&src,&dst)) return NULL; 
    return PyLong_FromLong(waaoMoveFile(src,dst)); 
}

static PyObject *pyReadAll(PyObject *s, PyObject *a){
    const char *p; 
    if(!PyArg_ParseTuple(a,"s",&p)) return NULL;
    WaaoBuffer b; 
    if(waaoReadAll(p,&b)!=WAAO_SUCCESS){ 
        Py_RETURN_NONE; 
    }
    PyObject *bytes = PyBytes_FromStringAndSize(b.data, (Py_ssize_t)b.len);
    waaoBufFree(&b); 
    return bytes;
}
static PyObject *pyWriteAll(PyObject *s, PyObject *a){
    const char *p; 
    Py_buffer buf;
    if(!PyArg_ParseTuple(a,"sy*",&p,&buf)) return NULL;
    int rc=waaoWriteAll(p, buf.buf, (size_t)buf.len);
    PyBuffer_Release(&buf);
    return PyLong_FromLong(rc);
}
static PyObject *pyAppendAll(PyObject *s, PyObject *a){
    const char *p; Py_buffer buf;
    if(!PyArg_ParseTuple(a,"sy*",&p,&buf)) return NULL;
    int rc=waaoAppendAll(p, buf.buf, (size_t)buf.len);
    PyBuffer_Release(&buf);
    return PyLong_FromLong(rc);
}
static PyObject *pyMakeDir(PyObject *s, PyObject *a){ 
    const char *p; 
    if(!PyArg_ParseTuple(a,"s",&p)) return NULL; 
    return PyLong_FromLong(waaoMakeDir(p)); 
}

static PyObject *pyRemoveDir(PyObject *s, PyObject *a){ 
    const char *p; 
    if(!PyArg_ParseTuple(a,"s",&p)) return NULL; 
    return PyLong_FromLong(waaoRemoveDir(p)); 
}

static PyObject *pyListDir(PyObject *self, PyObject *args){
    const char *p; 
    if(!PyArg_ParseTuple(args,"s",&p)) return NULL;
    char **names; size_t n;
    if(waaoListDir(p,&names,&n)!=WAAO_SUCCESS) Py_RETURN_NONE;
    PyObject *list=PyList_New((Py_ssize_t)n);
    for(size_t i=0;i<n;i++){ 
        PyList_SET_ITEM(list,(Py_ssize_t)i, PyUnicode_FromString(names[i])); 
    }
    waaoListDirFree(names,n);
    return list;
}

static PyObject *pyDisplayLines(PyObject *self, PyObject *args){
    const char *path; 
    int fromStart; 
    Py_ssize_t n;
    if(!PyArg_ParseTuple(args,"spi",&path,&fromStart,&n)) return NULL;
    return PyLong_FromLong(waaoDisplayLines(path,fromStart,(size_t)n));
}
static PyObject *pyFindPattern(PyObject *self, PyObject *args){
    const char *path,*pat; 
    if(!PyArg_ParseTuple(args,"ss",&path,&pat)) return NULL;
    return PyLong_FromSize_t(waaoFindPattern(path,pat));
}
static PyObject *pyCountLines(PyObject *s, PyObject *a){
    const char *p; 
    if(!PyArg_ParseTuple(a,"s",&p)) return NULL;
    size_t n; 
    if(waaoCountLines(p,&n)!=WAAO_SUCCESS) Py_RETURN_NONE;
    return PyLong_FromSize_t(n);
}
static PyObject *pyCountWords(PyObject *s, PyObject *a){
    const char *p; if(!PyArg_ParseTuple(a,"s",&p)) return NULL;
    size_t n; if(waaoCountWords(p,&n)!=WAAO_SUCCESS) Py_RETURN_NONE;
    return PyLong_FromSize_t(n);
}
static PyObject *pySortFileLines(PyObject *s, PyObject *a){
    const char *in,*out; 
    int ic; 
    if(!PyArg_ParseTuple(a,"ssi",&in,&out,&ic)) return NULL;
    return PyLong_FromLong(waaoSortFileLines(in,out,ic));
}






static PyObject *pyCompressArchive(PyObject *self, PyObject *args){
    const char *archive; 
    PyObject *seq;
    if(!PyArg_ParseTuple(args,"sO",&archive,&seq)) return NULL;
    char **arr; size_t n;
    if(!seqToCStringArray(seq,&arr,&n)) return NULL;
    int rc=waaoCompressArchive(archive, (const char**)arr, n);
    freeCStringArray(arr,n);
    return PyLong_FromLong(rc);
}
static PyObject *pyDecompressArchive(PyObject *self, PyObject *args){
    const char *archive,*out; 
    if(!PyArg_ParseTuple(args,"ss",&archive,&out)) return NULL;
    return PyLong_FromLong(waaoDecompressArchive(archive,out));
}


static PyObject *pyTextToPdf(PyObject *s, PyObject *a){ 
    const char *i,*o; 
    if(!PyArg_ParseTuple(a,"ss",&i,&o)) return NULL; return PyLong_FromLong(waaoTextToPdf(i,o)); 
}
static PyObject *pyCsvToTsv(PyObject *s, PyObject *a){ 
    const char *i,*o; 
    if(!PyArg_ParseTuple(a,"ss",&i,&o)) return NULL; 
    return PyLong_FromLong(waaoCsvToTsv(i,o)); 
}
static PyObject *pyTsvToCsv(PyObject *s, PyObject *a){ 
    const char *i,*o; 
    if(!PyArg_ParseTuple(a,"ss",&i,&o)) return NULL; 
    return PyLong_FromLong(waaoTsvToCsv(i,o)); 
}
static PyObject *pyJsonMinify(PyObject *s, PyObject *a){ 
    const char *i,*o; 
    if(!PyArg_ParseTuple(a,"ss",&i,&o)) return NULL; 
    return PyLong_FromLong(waaoJsonMinify(i,o)); 
}
static PyObject *pyTextToUpperFile(PyObject *s, PyObject *a){ 
    const char *i,*o; 
    if(!PyArg_ParseTuple(a,"ss",&i,&o)) return NULL; 
    return PyLong_FromLong(waaoTextToUpperFile(i,o)); 
}
static PyObject *pyHexDumpFile(PyObject *s, PyObject *a){ 
    const char *i,*o; 
    if(!PyArg_ParseTuple(a,"ss",&i,&o)) return NULL; 
    return PyLong_FromLong(waaoHexDumpFile(i,o)); 
}


static PyObject *pyBase64Encode(PyObject *s, PyObject *a){
    Py_buffer buf; 
    if(!PyArg_ParseTuple(a,"y*",&buf)) return NULL;
    size_t outLen; 
    char *enc=waaoBase64Encode(buf.buf,(size_t)buf.len,&outLen);
    PyBuffer_Release(&buf);
    if(!enc) { 
        Py_RETURN_NONE; 
    }
    PyObject *res = PyUnicode_FromStringAndSize(enc,(Py_ssize_t)outLen);
    free(enc); 
    return res;
}
static PyObject *pyBase64Decode(PyObject *s, PyObject *a){
    const char *b64; 
    Py_ssize_t L; 
    if(!PyArg_ParseTuple(a,"s#",&b64,&L)) return NULL;
    size_t outLen; 
    uint8_t *dec=waaoBase64Decode(b64,(size_t)L,&outLen);
    if(!dec) { 
        Py_RETURN_NONE; 
    }
    PyObject *res = PyBytes_FromStringAndSize((const char*)dec,(Py_ssize_t)outLen);
    free(dec); 
    return res;
}
static PyObject *pyCrc32(PyObject *s, PyObject *a){
    Py_buffer buf; 
    if(!PyArg_ParseTuple(a,"y*",&buf)) return NULL;
    uint32_t crc=waaoCrc32(buf.buf,(size_t)buf.len);
    PyBuffer_Release(&buf);
    return PyLong_FromUnsignedLong(crc);
}

static PyMethodDef WaaoMethods[] = {
    {"fileExists", pyFileExists, METH_VARARGS, "Return True/False if file exists"},
    {"fileSize", pyFileSize, METH_VARARGS, "Return file size or None"},
    {"createFile", pyCreateFile, METH_VARARGS, "Create empty file"},
    {"deleteFile", pyDeleteFile, METH_VARARGS, "Delete file"},
    {"copyFile", pyCopyFile, METH_VARARGS, "Copy file"},
    {"moveFile", pyMoveFile, METH_VARARGS, "Move/Rename file"},
    {"readAll", pyReadAll, METH_VARARGS, "Read entire file -> bytes"},
    {"writeAll", pyWriteAll, METH_VARARGS, "Write all bytes"},
    {"appendAll", pyAppendAll, METH_VARARGS, "Append all bytes"},
    {"makeDir", pyMakeDir, METH_VARARGS, "Create directory"},
    {"removeDir", pyRemoveDir, METH_VARARGS, "Remove empty directory"},
    {"listDir", pyListDir, METH_VARARGS, "List directory names"},

    {"displayLines", pyDisplayLines, METH_VARARGS, "Print first/last N lines"},
    {"findPattern", pyFindPattern, METH_VARARGS, "Find pattern; return 1-based line or 0"},
    {"countLines", pyCountLines, METH_VARARGS, "Count lines"},
    {"countWords", pyCountWords, METH_VARARGS, "Count words"},
    {"sortFileLines", pySortFileLines, METH_VARARGS, "Sort lines into outPath (ignoreCase 0/1)"},

    {"compressArchive", pyCompressArchive, METH_VARARGS, "Create WAAO archive from files list"},
    {"decompressArchive", pyDecompressArchive, METH_VARARGS, "Extract WAAO archive to directory"},

    {"textToPdf", pyTextToPdf, METH_VARARGS, "Text to minimal PDF"},
    {"csvToTsv", pyCsvToTsv, METH_VARARGS, "CSV -> TSV"},
    {"tsvToCsv", pyTsvToCsv, METH_VARARGS, "TSV -> CSV"},
    {"jsonMinify", pyJsonMinify, METH_VARARGS, "Minify JSON"},
    {"textToUpperFile", pyTextToUpperFile, METH_VARARGS, "Uppercase text file"},
    {"hexDumpFile", pyHexDumpFile, METH_VARARGS, "Hex dump a file"},

    {"base64Encode", pyBase64Encode, METH_VARARGS, "Base64 encode bytes -> str"},
    {"base64Decode", pyBase64Decode, METH_VARARGS, "Base64 decode str -> bytes"},
    {"crc32", pyCrc32, METH_VARARGS, "CRC32 of bytes"},

    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef waaoModule = {
    PyModuleDef_HEAD_INIT, "waao", "WAAO utilities C extension", -1, WaaoMethods
};

PyMODINIT_FUNC PyInit_waao(void){ 
    return PyModule_Create(&waaoModule); 
}

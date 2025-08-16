#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "waaoArchive.h"
#include "waaoIo.h"
#include "waaoUtils.h"
#include "waaoDefs.h"


static void *rleCompress(const uint8_t *in, size_t len, size_t *outLen){
    size_t cap = len*2 + 2;
    uint8_t *out=(uint8_t*)malloc(cap); if(!out) return NULL;
    size_t i=0,o=0;
    while(i<len){
        uint8_t b=in[i]; size_t run=1;
        while(i+run<len && in[i+run]==b && run<255) run++;
        out[o++]=(uint8_t)run; out[o++]=b;
        i+=run;
    }
    *outLen=o; return out;
}
static void *rleDecompress(const uint8_t *in, size_t len, size_t *outLen){
    size_t cap=len*4+16; 
    uint8_t *out=(uint8_t*)malloc(cap); 
    if(!out) return NULL;
    size_t i=0,o=0;
    while(i+1<len){
        uint8_t run=in[i++], b=in[i++];
        if(o+run>cap){ cap=(o+run)*2; 
            out=(uint8_t*)realloc(out,cap); }
        memset(out+o, b, run); 
        o+=run;
    }
    *outLen=o; return out;
}

int waaoCompressArchive(const char *archivePath, const char **files, size_t fileCount){
    FILE *arc=fopen(archivePath,"wb"); 
    if(!arc) return WAAO_ERROR;
    fwrite("WAAO1\n",1,6,arc);
    uint32_t cnt=(uint32_t)fileCount; 
    fwrite(&cnt,4,1,arc);
    for(size_t i=0;i<fileCount;i++){
        WaaoBuffer buf; 
        if(waaoReadAll(files[i], &buf)!=WAAO_SUCCESS){ 
            fclose(arc); return WAAO_ERROR; 
        }
        size_t compLen; 
        void *comp=rleCompress((uint8_t*)buf.data, buf.len, &compLen);
        if(!comp){ 
            waaoBufFree(&buf); 
            fclose(arc); 
            return WAAO_ERROR; 
        }
        uint16_t nameLen=(uint16_t)strlen(files[i]);
        fwrite(&nameLen,2,1,arc); 
        fwrite(files[i],1,nameLen,arc);
        uint64_t orig=(uint64_t)buf.len, compSz=(uint64_t)compLen;
        uint32_t crc=waaoCrc32(buf.data, buf.len);
        fwrite(&orig,8,1,arc); 
        fwrite(&compSz,8,1,arc); 
        fwrite(&crc,4,1,arc);
        fwrite(comp,1,compLen,arc);
        free(comp); 
        waaoBufFree(&buf);
    }
    fclose(arc); 
    return WAAO_SUCCESS;
}

int waaoDecompressArchive(const char *archivePath, const char *outputDir){
    FILE *arc=fopen(archivePath,"rb"); 
    if(!arc) return WAAO_ERROR;
    char magic[6]; 
    if(fread(magic,1,6,arc)!=6 || memcmp(magic,"WAAO1\n",6)!=0){ 
        fclose(arc); 
        return WAAO_ERROR; 
    }
    uint32_t cnt; 
    if(fread(&cnt,4,1,arc)!=1){ 
        fclose(arc); return WAAO_ERROR; 
    }
    for(uint32_t i=0;i<cnt;i++){
        uint16_t nameLen; 
        if(fread(&nameLen,2,1,arc)!=1){ 
            fclose(arc); 
            return WAAO_ERROR; 
        }
        char *name=(char*)malloc(nameLen+1); 
        fread(name,1,nameLen,arc); 
        name[nameLen]='\0';
        uint64_t orig, compSz; uint32_t crc;
        fread(&orig,8,1,arc); 
        fread(&compSz,8,1,arc); 
        fread(&crc,4,1,arc);
        uint8_t *comp=(uint8_t*)malloc((size_t)compSz); 
        if(!comp){ 
            free(name); 
            fclose(arc); 
            return WAAO_ERROR; 
        }
        if(fread(comp,1,(size_t)compSz,arc)!=(size_t)compSz){ 
            free(comp); 
            free(name); 
            fclose(arc); 
            return WAAO_ERROR; 
        }
        size_t decLen; void *dec=rleDecompress(comp,(size_t)compSz,&decLen); 
        free(comp);
        if(!dec || decLen!=(size_t)orig || waaoCrc32(dec,decLen)!=crc){ 
            free(dec); 
            free(name); 
            fclose(arc); 
            return WAAO_ERROR; 
        }
        char outPath[WAAO_PATH_MAX]; 
        waaoJoinPath(outputDir, name, outPath);
        if(waaoWriteAll(outPath, dec, decLen)!=WAAO_SUCCESS){ 
            free(dec); 
            free(name); 
            fclose(arc); 
            return WAAO_ERROR; 
        }
        free(dec); 
        free(name);
    }
    fclose(arc); 
    return WAAO_SUCCESS;
}

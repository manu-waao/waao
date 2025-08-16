#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#ifdef _WIN32
  #include <io.h>
  #include <direct.h>
  #define WAAO_MKDIR(p) _mkdir(p)
  #define WAAO_UNLINK _unlink
  #define WAAO_RENAME _rename
  #include <windows.h>
#else
  #include <dirent.h>
  #include <unistd.h>
  #define WAAO_MKDIR(p) mkdir(p, 0755)
  #define WAAO_UNLINK unlink
  #define WAAO_RENAME rename
#endif
#include "waaoIo.h"
#include "waaoUtils.h"

int waaoFileExists(const char *path){
    FILE *f=fopen(path,"rb");
    if(!f) return 0;
    fclose(f); return 1;
}

int waaoFileSize(const char *path, int64_t *outSize){
    struct stat st;
    if(stat(path, &st)!=0) return WAAO_ERROR;
    if(outSize) *outSize = (int64_t)st.st_size;
    return WAAO_SUCCESS;
}

int waaoCreateFile(const char *path){
    FILE *f=fopen(path,"wb"); 
    if(!f) return WAAO_ERROR; 
    fclose(f); 
    return WAAO_SUCCESS;
}

int waaoDeleteFile(const char *path){ 
    return (WAAO_UNLINK(path)==0)? WAAO_SUCCESS:WAAO_ERROR; 
}

int waaoCopyFile(const char *src, const char *dst){
    FILE *in=fopen(src,"rb"); 
    if(!in) return WAAO_ERROR;
    FILE *out=fopen(dst,"wb"); 
    if(!out){ 
        fclose(in); 
        return WAAO_ERROR; 
    }
    char *buf=(char*)malloc(WAAO_IO_BUFSZ); 
    if(!buf){ 
        fclose(in); 
        fclose(out); 
        return WAAO_ERROR; 
    }
    size_t n;
    while((n=fread(buf,1,WAAO_IO_BUFSZ,in))>0){
        if(fwrite(buf,1,n,out)!=n){ 
            free(buf); 
            fclose(in); 
            fclose(out); 
            return WAAO_ERROR; 
        }
    }

    free(buf); 
    fclose(in); 
    fclose(out); 
    return WAAO_SUCCESS;
}

int waaoMoveFile(const char *src, const char *dst){ 
    return (WAAO_RENAME(src,dst)==0)? WAAO_SUCCESS:WAAO_ERROR; 
}

int waaoReadAll(const char *path, WaaoBuffer *out){
    waaoBufInit(out);
    FILE *f=fopen(path,"rb"); 
    if(!f) return WAAO_ERROR;
    if(fseek(f,0,SEEK_END)!=0){ 
        fclose(f); return WAAO_ERROR; 
    }
    long sz=ftell(f); 
    if(sz<0){ 
        fclose(f); 
        return WAAO_ERROR; 
    }
    if(fseek(f,0,SEEK_SET)!=0){ 
        fclose(f); 
        return WAAO_ERROR; 
    }
    out->data=(char*)malloc((size_t)sz+1); 
    if(!out->data){ 
        fclose(f); 
        return WAAO_ERROR; 
    }
    size_t rd=fread(out->data,1,(size_t)sz,f);
    fclose(f);
    if(rd!=(size_t)sz){ 
        waaoBufFree(out); 
        return WAAO_ERROR; 
    }
    out->data[sz]='\0'; out->len=(size_t)sz; 
    out->cap=(size_t)sz+1;
    return WAAO_SUCCESS;
}

int waaoWriteAll(const char *path, const void *data, size_t len){
    FILE *f=fopen(path,"wb"); 
    if(!f) return WAAO_ERROR;
    size_t wr=fwrite(data,1,len,f); 
    fclose(f);
    return (wr==len)? WAAO_SUCCESS:WAAO_ERROR;
}

int waaoAppendAll(const char *path, const void *data, size_t len){
    FILE *f=fopen(path,"ab"); 
    if(!f) return WAAO_ERROR;
    size_t wr=fwrite(data,1,len,f); 
    fclose(f);
    return (wr==len)? WAAO_SUCCESS:WAAO_ERROR;
}

int waaoMakeDir(const char *path){ 
    return (WAAO_MKDIR(path)==0)? WAAO_SUCCESS:WAAO_ERROR; 
}

int waaoRemoveDir(const char *path){
#ifdef _WIN32
    return (_rmdir(path)==0)? WAAO_SUCCESS:WAAO_ERROR;
#else
    return (rmdir(path)==0)? WAAO_SUCCESS:WAAO_ERROR;
#endif
}

int waaoListDir(const char *path, char ***names, size_t *count){
    *names=NULL; *count=0;
#ifdef _WIN32
    char pattern[WAAO_PATH_MAX];
    snprintf(pattern,sizeof(pattern),"%s\\*.*", path);
    WIN32_FIND_DATAA ffd;
    HANDLE hFind = FindFirstFileA(pattern, &ffd);
    if(hFind==INVALID_HANDLE_VALUE) return WAAO_ERROR;
    size_t cap=16; char **arr=(char**)malloc(cap*sizeof(char*));
    do{
        const char *n=ffd.cFileName;
        if(strcmp(n,".")==0||strcmp(n,"..")==0) continue;
        if(*count==cap){ cap*=2; 
            arr=(char**)realloc(arr,cap*sizeof(char*)); 
        }
        arr[*count]=_strdup(n); 
        (*count)++;
    } while(FindNextFileA(hFind,&ffd)!=0);
    FindClose(hFind);
    *names=arr; 
    return WAAO_SUCCESS;
#else
    DIR *d=opendir(path); 
    if(!d) return WAAO_ERROR;
    size_t cap=16; 
    char **arr=(char**)malloc(cap*sizeof(char*));
    struct dirent *ent;
    while((ent=readdir(d))){
        const char *n=ent->d_name;
        if(strcmp(n,".")==0||strcmp(n,"..")==0) continue;
        if(*count==cap){ 
            cap*=2; 
            arr=(char**)realloc(arr,cap*sizeof(char*)); 
        }
        arr[*count]=strdup(n); 
        (*count)++;
    }
    closedir(d);
    *names=arr; return WAAO_SUCCESS;
#endif
}

void waaoListDirFree(char **names, size_t count){
    if(!names) return;
    for(size_t i=0;i<count;i++) free(names[i]);
    free(names);
}

int waaoDisplayLines(const char *path, int fromStart, size_t n){
    FILE *f=fopen(path,"r"); 
    if(!f) return WAAO_ERROR;
    char *line=NULL; 
    size_t cap=0; ssize_t m;
    if(fromStart){
        size_t cnt=0;
        while(cnt<n && (m=getline(&line,&cap,f))!=-1){
            fwrite(line,1,(size_t)m,stdout);
            cnt++;
        }
    }else{
        size_t *offsets=(size_t*)calloc(n+1,sizeof(size_t));
        size_t idx=0, lines=0; 
        long pos=0;
        while((m=getline(&line,&cap,f))!=-1){
            offsets[idx]=pos;
            idx=(idx+1)%(n+1);
            lines++; pos=ftell(f);
        }
        size_t start= (lines>n)? idx : 0;
        size_t toPrint= (lines>n)? n : lines;
        fseek(f, (long)offsets[start], SEEK_SET);
        size_t printed=0;
        while(printed<toPrint && (m=getline(&line,&cap,f))!=-1){
            fwrite(line,1,(size_t)m,stdout); 
            printed++;
        }
        free(offsets);
    }
    free(line); 
    fclose(f); 
    return WAAO_SUCCESS;
}

size_t waaoFindPattern(const char *path, const char *pattern){
    FILE *f=fopen(path,"r"); 
    if(!f) return 0;
    char *line=NULL; 
    size_t cap=0; 
    ssize_t m; 
    size_t ln=1;
    while((m=getline(&line,&cap,f))!=-1){
        if(strstr(line,pattern)){ 
            free(line); 
            fclose(f); 
            return ln; 
        }
        ln++;
    }
    free(line); 
    fclose(f); 
    return 0;
}

int waaoCountLines(const char *path, size_t *out){
    *out=0; FILE *f=fopen(path,"r"); 
    if(!f) return WAAO_ERROR;
    int c; 
    while((c=fgetc(f))!=EOF){ 
        if(c=='\n') (*out)++; 
    }
    fclose(f); 
    return WAAO_SUCCESS;
}

int waaoCountWords(const char *path, size_t *out){
    *out=0; FILE *f=fopen(path,"r"); 
    if(!f) return WAAO_ERROR;
    int c, in=0;
    while((c=fgetc(f))!=EOF){
        if(isspace(c)){ 
            if(in){ 
                (*out)++; 
                in=0; 
            } 
        }
        else in=1;
    }
    if(in) (*out)++;
    fclose(f); 
    return WAAO_SUCCESS;
}

static int cmpLines(const void *a, const void *b){
    const char * const *sa = (const char* const*)a;
    const char * const *sb = (const char* const*)b;
    return strcmp(*sa,*sb);
}
static int icmpLines(const void *a, const void *b){
    const unsigned char *sa = *(const unsigned char**)a;
    const unsigned char *sb = *(const unsigned char**)b;
    for(;;){
        int ca=tolower(*sa), cb=tolower(*sb);
        if(ca!=cb) return ca-cb;
        if(!*sa||!*sb) return *sa-*sb;
        sa++; sb++;
    }
}

int waaoSortFileLines(const char *inPath, const char *outPath, int ignoreCase){
    WaaoBuffer buf; 
    if(waaoReadAll(inPath,&buf)!=WAAO_SUCCESS) return WAAO_ERROR;
    size_t linesCap=1024, linesCount=0;
    char **lines=(char**)malloc(linesCap*sizeof(char*));
    char *p=buf.data, *start=p;
    for(; *p; p++){
        if(*p=='\n'){ *p='\0';
            if(linesCount==linesCap){ 
                linesCap*=2; 
                lines=(char**)realloc(lines,linesCap*sizeof(char*)); 
            }
            lines[linesCount++]=start; 
            start=p+1;
        }
    }
    if(start && *start){
        if(linesCount==linesCap){ 
            linesCap*=2; 
            lines=(char**)realloc(lines,linesCap*sizeof(char*)); 
        }
        lines[linesCount++]=start;
    }
    qsort(lines, linesCount, sizeof(char*), ignoreCase? icmpLines:cmpLines);
    FILE *out=fopen(outPath,"w"); 
    if(!out){ 
        free(lines); 
        waaoBufFree(&buf); 
        return WAAO_ERROR; 
    }
    for(size_t i=0;i<linesCount;i++){ 
        fputs(lines[i],out); 
        fputc('\n',out); 
    }
    fclose(out);
    free(lines); 
    waaoBufFree(&buf); 
    return WAAO_SUCCESS;
}

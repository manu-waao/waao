#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "waaoUtils.h"

int waaoStringCompareIgnoreCase(const char *a, const char *b){
    while(*a && *b){
        int ca = tolower((unsigned char)*a);
        int cb = tolower((unsigned char)*b);
        if(ca!=cb) return ca-cb;
        a++; b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

void waaoTrimInPlace(char *s){
    if(!s) return;
    char *start = s; while(*start && isspace((unsigned char)*start)) start++;
    char *end = s + strlen(s); while(end>start && isspace((unsigned char)*(end-1))) end--;
    size_t len = (size_t)(end-start);
    memmove(s, start, len);
    s[len] = '\0';
}

void waaoToLowerInPlace(char *s){ for(;*s;s++) *s = (char)tolower((unsigned char)*s); }
void waaoToUpperInPlace(char *s){ for(;*s;s++) *s = (char)toupper((unsigned char)*s); }

char *waaoReplaceAlloc(const char *s, const char *find, const char *repl){
    size_t sl=strlen(s), fl=strlen(find), rl=strlen(repl);
    if(fl==0) { 
        char *dup = (char*)malloc(sl+1); 
        if(dup) memcpy(dup,s,sl+1); 
        return dup; 
    }
    size_t count=0; 
    for(const char *p=s; (p=strstr(p,find)); p+=fl) count++;
    size_t outLen = sl + count*(rl-fl);
    char *out = (char*)malloc(outLen+1); 
    if(!out) return NULL;
    const char *src=s; 
    char *dst=out;
    while((s=strstr(src,find))){
        size_t n=(size_t)(s-src); 
        memcpy(dst,src,n); 
        dst+=n;
        memcpy(dst,repl,rl); 
        dst+=rl; 
        src=s+fl;
    }
    size_t tail=strlen(src); 
    memcpy(dst,src,tail); 
    dst+=tail;
    *dst='\0';
    return out;
}

void waaoJoinPath(const char *a, const char *b, char out[WAAO_PATH_MAX]){
#ifdef _WIN32
    const char sep = '\\';
#else
    const char sep = '/';
#endif
    size_t al = strlen(a);
    int need = (al>0 && a[al-1]!=sep);
    snprintf(out, WAAO_PATH_MAX, "%s%s%s", a, need? (char[2]){sep,0} : "", b);
}

const char *waaoBasename(const char *path){
    const char *p1 = strrchr(path,'/');
    const char *p2 = strrchr(path,'\\');
    const char *p = (p1>p2? p1 : p2);
    return p? p+1 : path;
}

const char *waaoExtname(const char *path){
    const char *base = waaoBasename(path);
    const char *dot = strrchr(base,'.');
    return dot? dot : "";
}

/* CRC32 (Polynomial 0xEDB88320) */
uint32_t waaoCrc32(const void *data, size_t len){
    const uint8_t *p=(const uint8_t*)data;
    uint32_t crc=0xFFFFFFFFu;
    for(size_t i=0;i<len;i++){
        crc ^= p[i];
        for(int k=0;k<8;k++){
            uint32_t mask = -(crc & 1u);
            crc = (crc >> 1) ^ (0xEDB88320u & mask);
        }
    }
    return ~crc;
}


static const char *B64="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char *waaoBase64Encode(const uint8_t *data, size_t len, size_t *outLen){
    size_t olen = 4*((len+2)/3);
    char *out = (char*)malloc(olen+1); 
    if(!out) return NULL;
    size_t i=0,o=0;
    while(i<len){
        uint32_t v = data[i++]<<16;
        if(i<len) v |= data[i++]<<8;
        if(i<len) v |= data[i++];
        out[o++] = B64[(v>>18)&63];
        out[o++] = B64[(v>>12)&63];
        out[o++] = (i-1>len)? '=' : B64[(v>>6)&63];
        out[o++] = (i>len)? '=' : B64[v&63];
    }
    out[o]='\0'; if(outLen) *outLen=o; return out;
}

static int b64Index(char c){
    if('A'<=c&&c<='Z') return c-'A';
    if('a'<=c&&c<='z') return c-'a'+26;
    if('0'<=c&&c<='9') return c-'0'+52;
    if(c=='+') return 62; 
    if(c=='/') return 63; 
    if(c=='=') return -2; 
    return -1;
}

uint8_t *waaoBase64Decode(const char *b64, size_t b64len, size_t *outLen){
    size_t cap = (b64len/4)*3;
    uint8_t *out=(uint8_t*)malloc(cap?cap:1); 
    if(!out) return NULL;
    size_t o=0; 
    int vals[4]; 
    size_t i=0;
    while(i<b64len){
        int vcnt=0;
        for(int k=0;k<4 && i<b64len;){
            int idx=b64Index(b64[i++]); 
            if(idx==-1) continue;
            vals[k++]=idx; 
            vcnt++;
            if(vcnt==4) break;
        }
        if(vcnt<4) break;
        int v0=vals[0],v1=vals[1],v2=vals[2],v3=vals[3];
        if(v2==-2){ 
            out[o++] = (uint8_t)((v0<<2)|(v1>>4));
            break;
        } else if(v3==-2){ 
            uint8_t b0=(uint8_t)((v0<<2)|(v1>>4));
            uint8_t b1=(uint8_t)((v1<<4)|(v2>>2));
            out[o++]=b0; 
            out[o++]=b1;
            break;
        } else {
            uint8_t b0=(uint8_t)((v0<<2)|(v1>>4));
            uint8_t b1=(uint8_t)((v1<<4)|(v2>>2));
            uint8_t b2=(uint8_t)((v2<<6)|v3);
            out[o++]=b0; 
            out[o++]=b1; 
            out[o++]=b2;
        }
    }
    if(outLen) *outLen=o;
    return out;
}
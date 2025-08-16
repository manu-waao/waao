#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "waaoConvert.h"
#include "waaoIo.h"
#include "waaoUtils.h"

int waaoTextToPdf(const char *inputPath, const char *outputPath){
    WaaoBuffer buf; 
    if(waaoReadAll(inputPath,&buf)!=WAAO_SUCCESS) return WAAO_ERROR;
    FILE *out=fopen(outputPath,"wb"); 
    if(!out){ 
        waaoBufFree(&buf); 
        return WAAO_ERROR; 
    }

    const char *header="%PDF-1.4\n";
    fwrite(header,1,strlen(header),out);

    const char *obj1="1 0 obj\n<< /Type /Catalog /Pages 2 0 R >>\nendobj\n";
    fwrite(obj1,1,strlen(obj1),out);

    const char *obj2="2 0 obj\n<< /Type /Pages /Kids [3 0 R] /Count 1 >>\nendobj\n";
    fwrite(obj2,1,strlen(obj2),out);

    const char *obj3="3 0 obj\n<< /Type /Page /Parent 2 0 R /MediaBox [0 0 595 842] /Contents 4 0 R /Resources << /Font << /F1 5 0 R >> >> >>\nendobj\n";
    fwrite(obj3,1,strlen(obj3),out);

    const char *obj5="5 0 obj\n<< /Type /Font /Subtype /Type1 /BaseFont /Courier >>\nendobj\n";
    fwrite(obj5,1,strlen(obj5),out);

    WaaoBuffer stream; waaoBufInit(&stream);

    const char *p=buf.data; const char *end=p+buf.len;
    const char *prefix="BT /F1 12 Tf 50 800 Td ";
    size_t preLen=strlen(prefix);
    stream.data=(char*)malloc(buf.len*2 + 1024); 
    stream.cap=buf.len*2+1024; 
    stream.len=0;
    memcpy(stream.data+stream.len, prefix, preLen); 
    stream.len+=preLen;

    size_t y=800;
    while(p<end){
        const char *nl=memchr(p,'\n',(size_t)(end-p));
        size_t l = nl? (size_t)(nl-p) : (size_t)(end-p);
        char *escaped=(char*)malloc(l*2+4);
        size_t e=0; escaped[e++]='('; 
        for(size_t i=0;i<l;i++){
            char c=p[i]; if(c=='('||c==')'||c=='\\') escaped[e++]='\\';
            escaped[e++]= (c=='\r')? ' ' : c;
        }
        escaped[e++]=')'; escaped[e++]=' '; escaped[e++]='T'; escaped[e++]='j';
        escaped[e++]='\n';
        if(stream.len+e+32 > stream.cap){ 
            stream.cap=(stream.len+e+32)*2; 
            stream.data=(char*)realloc(stream.data,stream.cap); 
        }
        memcpy(stream.data+stream.len, escaped, e); stream.len+=e;
        free(escaped);
        y = (y>20)? y-14 : 800;
        char move[32]; int mlen=snprintf(move,sizeof(move), "0 -14 Td ");
        memcpy(stream.data+stream.len, move, (size_t)mlen); stream.len+= (size_t)mlen;
        if(!nl) break; p=nl+1;
    }
    const char *suffix=" ET\n";
    memcpy(stream.data+stream.len, suffix, strlen(suffix)); stream.len+=strlen(suffix);


    char hdr[64]; int hlen=snprintf(hdr,sizeof(hdr), "4 0 obj\n<< /Length %zu >>\nstream\n", stream.len);
    fwrite(hdr,1,(size_t)hlen,out);
    fwrite(stream.data,1,stream.len,out);
    fwrite("endstream\nendobj\n",1,18,out);


    const char *eof="%%EOF\n";
    fwrite(eof,1,strlen(eof),out);

    fclose(out); waaoBufFree(&buf); waaoBufFree(&stream);
    return WAAO_SUCCESS;
}

int waaoCsvToTsv(const char *inputPath, const char *outputPath){
    FILE *in=fopen(inputPath,"r"); if(!in) return WAAO_ERROR;
    FILE *out=fopen(outputPath,"w"); if(!out){ fclose(in); return WAAO_ERROR; }
    int c; int inQuotes=0;
    while((c=fgetc(in))!=EOF){
        if(c=='"'){ inQuotes=!inQuotes; fputc(c,out); }
        else if(c==',' && !inQuotes) fputc('\t',out);
        else fputc(c,out);
    }
    fclose(in); fclose(out); return WAAO_SUCCESS;
}

int waaoTsvToCsv(const char *inputPath, const char *outputPath){
    FILE *in=fopen(inputPath,"r"); if(!in) return WAAO_ERROR;
    FILE *out=fopen(outputPath,"w"); if(!out){ fclose(in); return WAAO_ERROR; }
    int c; int needQuote=0; char line[8192];
    while(fgets(line,sizeof(line),in)){
        size_t L=strlen(line);

        char *p=line; int first=1; char field[4096]; size_t f=0;
        for(size_t i=0;i<=L;i++){
            if(line[i]=='\t' || line[i]=='\n' || line[i]=='\0'){
                field[f]='\0';
                needQuote = (strchr(field,',')||strchr(field,'"')||strchr(field,'\n'))!=NULL;
                if(!first) fputc(',',out); first=0;
                if(needQuote){
                    fputc('"',out);
                    for(size_t j=0;j<f;j++){ if(field[j]=='"') fputc('"',out); fputc(field[j],out); }
                    fputc('"',out);
                }else{
                    fputs(field,out);
                }
                f=0;
                if(line[i]=='\n'){ fputc('\n',out); break; }
            }else{
                if(f<sizeof(field)-1) field[f++]=line[i];
            }
        }
    }
    fclose(in); fclose(out); return WAAO_SUCCESS;
}

int waaoJsonMinify(const char *inputPath, const char *outputPath){
    WaaoBuffer buf; if(waaoReadAll(inputPath,&buf)!=WAAO_SUCCESS) return WAAO_ERROR;
    char *out=(char*)malloc(buf.len+1); if(!out){ waaoBufFree(&buf); return WAAO_ERROR; }
    size_t o=0; int inStr=0, esc=0;
    for(size_t i=0;i<buf.len;i++){
        char c=buf.data[i];
        if(inStr){
            out[o++]=c;
            if(esc) esc=0;
            else if(c=='\\') esc=1;
            else if(c=='"') inStr=0;
        }else{
            if(isspace((unsigned char)c)) continue;
            if(c=='"'){ inStr=1; out[o++]=c; }
            else out[o++]=c;
        }
    }
    int rc=waaoWriteAll(outputPath, out, o);
    free(out); waaoBufFree(&buf); return rc;
}

int waaoTextToUpperFile(const char *inputPath, const char *outputPath){
    WaaoBuffer buf; if(waaoReadAll(inputPath,&buf)!=WAAO_SUCCESS) return WAAO_ERROR;
    for(size_t i=0;i<buf.len;i++) buf.data[i]=(char)toupper((unsigned char)buf.data[i]);
    int rc=waaoWriteAll(outputPath, buf.data, buf.len);
    waaoBufFree(&buf); return rc;
}

int waaoHexDumpFile(const char *inputPath, const char *outputPath){
    FILE *in=fopen(inputPath,"rb"); if(!in) return WAAO_ERROR;
    FILE *out=fopen(outputPath,"w"); if(!out){ fclose(in); return WAAO_ERROR; }
    unsigned char buf[16]; size_t n; size_t off=0;
    while((n=fread(buf,1,16,in))>0){
        fprintf(out, "%08zx  ", off);
        for(size_t i=0;i<16;i++){
            if(i<n) fprintf(out,"%02x ", buf[i]); else fputs("   ",out);
            if(i==7) fputc(' ',out);
        }
        fputc(' ',out);
        for(size_t i=0;i<n;i++){
            unsigned char c=buf[i]; fputc(isprint(c)? c : '.', out);
        }
        fputc('\n',out);
        off+=n;
    }
    fclose(in); fclose(out); return WAAO_SUCCESS;
}

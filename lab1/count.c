/**
 * author :yao wei (wei.849)
 *  01/14/2018
 **/
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
int  main(int argc,char *argv[]){
    if(argc!=4){
        printf("wrong number of args\n");
        printf("input should has the format: count <input-filename> <search-string> <output-filename>\n");
        return -1;
    }
    const int STR_LEN=20;
    const int CHUNK_SIZE=100;
    char* infile=argv[1];
    char* str=argv[2];
    
    char* outfile=argv[3];
    if(strlen(str)>STR_LEN){
        printf("the length of the input string is too large\n");
        return -1;
    }
    FILE* fin=fopen(infile,"r");
    if(fin==NULL){
        printf("can not open file %s \n",infile);
        exit(0);
    }
    size_t fSize=0;
    char * buff=malloc(CHUNK_SIZE);
    if(buff==NULL){
        printf("malloc %d bytes memory fails\n",CHUNK_SIZE);
        return -1;
    }
    size_t readSize;
    size_t matchCount=0;
    //read chunk pointer
    char* p;
    
    char* pend;
    char* pstr;
    char* pstre=str+strlen(str);
    char* next;
    char* lastMatch[STR_LEN-1];
    int i=0;
    while((readSize=fread(buff,1,CHUNK_SIZE,fin))){
        fSize+=readSize;
        pend=buff+readSize;
        p=buff;
        if(i>0){
            //handle the partial match from last chunk;
            
            for(int j=0;j<i;j++){
                size_t leftSize=pstre-lastMatch[j];
                //the bytes to be compare is larger than the left memory
                
                if(leftSize>readSize)
                    break;
                if(memcmp(buff,lastMatch[j],leftSize)==0)
                    matchCount++;
            }
        }
        i=0;
        
        while(p!=pend){
            pstr=str;
            
            next=0;
            
            while(p!=pend&&pstr!=pstre&&*pstr==*p){
                //if next match of first char is not found and this is not the first char
                if(!next&&pstr!=str&&*p==*str)
                    next=p;
                pstr++;
                p++;
            }
            
            if(pstr==pstre)
                matchCount++;
            else if(p==pend){
                //the byte before pstr has been matched, store pstr for comparison in next chunk.
                lastMatch[i++]=pstr;
                //end of the chunk
                
            }
            //never entered while loop,start match from next
            if(pstr==str)
                p++;
            //if find next match of first char, scan from that position
            //if not,just scan from next p;
            else if(next)
                p=next;
            
            //now handle string span in different chuncks
        }
    }
    //   printf("%zu \n",matchCount);
    fclose(fin);
    FILE* fout=fopen(outfile,"w");
    if(!fout){
        printf("can not open output file %s \n",outfile);
        exit(0);
    }
    fprintf(fout,"Size of file is %zu\n",fSize);
    printf("Size of file is %zu\n",fSize);
    fprintf(fout,"Number of matches = %zu\n",matchCount);
    printf("Number of matches = %zu\n",matchCount);
    fclose(fout);
    if(buff)
        free(buff);
    return 0;
    
}

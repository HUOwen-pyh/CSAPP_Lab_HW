/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "HUOWen",
    /* First member's full name */
    "HUO",
    /* First member's email address */
    "mymail@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define ull64 unsigned long long 
#define GET(p)          (*(ull64 *)(p))
#define PUT(p,val)      (*(ull64 *)(p) = (val))

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

static void* str_p;
static void* end_p;
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    mem_sbrk(27*sizeof(void*));
    str_p = mem_heap_lo();
    end_p = (void*)((ull64)str_p + (ull64)(26*sizeof(void*)));
    //printf("init heap with str = %lx and end = %lx\n\n",str_p,end_p);
    for(int i = 0;i<27;i++){
        ull64* cur = ((ull64*)str_p) + i;
        if(i<25)(*cur) = 0;
        else (*cur) = 1;
    }
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size){
    
    ull64 realsize = ALIGN(size + 2*SIZE_T_SIZE);
    ull64 newsize = ALIGN(size + 5*SIZE_T_SIZE);
    ull64* cur = NULL;
    ull64* ret = NULL;
    for(ull64 i = 0;i<25;i++){
        cur = ((ull64*)str_p) + i;
        if((*cur)!=0 && (1ULL<<i)>=newsize)break;
    }
    //printf("malloc for real %lx new %lx mem\n",realsize, newsize);
    if((*cur)!=0){
        ret = (ull64*)(*cur);
        ull64 len = (*ret);
        (*cur) = (*(ret+1)); 
        //printf("split the block %lx with len %lx for %lx mem\n",ret,(*ret),realsize);
        ull64* ep = ((ull64*)((char*)(ret) + realsize) - 1);
        ull64* eep = ((ull64*)((char*)(ret) + len) - 1);
        ull64* newnode = ep + 1;
        //printf("new blocks are %lx %lx and %lx %lx, ",ret,ep,newnode,eep);
        (*ret) = (realsize ^ 1);
        (*ep) = (*ret);
        ull64 newlen = len - realsize;
        //printf("their lens are %lx %lx\n",realsize, newlen);
        (*eep) = newlen;
        (*newnode) = (*eep);
        for(int i = 0;i<25;i++){
            cur = ((ull64*)str_p) + i;
            if((1ULL<<(i+1))>newlen){
                //printf("newnode is put into %d th list with range %lx %lx\n",i,1ULL<<i,1ULL<<(i+1));
                //printf("newnode len %lx %lx stat %x %x\n",(*newnode),(*eep),(*newnode)&1ULL,(*eep)&1ULL);
                //printf("expected to be %ld\n",newlen);
                (*(newnode+1)) = (*cur);
                (*cur) = (ull64)(newnode);
                break;
            }
        }
        //printf("allocate mem start from %lx, stat %d, len %lx, the expected len is %lx\n",ret,(*ret)&1ULL,(*ret)^1ULL,realsize);
        //printf("end at %lx, stat %d, len %lx\n",ep,(*ep)&1ULL,(*ep)^1ULL);
        //printf("current str %lx, end %lx\n\n", str_p,end_p);
        ret ++;
        return (void*)ret;
    }
    ull64* next = (ull64*)mem_sbrk(realsize);
    if (next == (ull64*)-1){
        return NULL;
    }
    ret = end_p; 
    next = ((ull64*)((char*)next + realsize)-1);
    (*next) = 1ULL;
    end_p = (void*)next;
    next --;
    (*ret) = (realsize ^ 1);
    (*next) = (*ret);
    //printf("allocate mem start from %lx, stat %d, len %lx, the expected len is %lx\n",ret,(*ret)&1ULL,(*ret)^1ULL,realsize);
    //printf("end at %lx, stat %d, len %lx\n",next,(*next)&1ULL,(*next)^1ULL);
    //printf("current str %lx, end %lx\n\n", str_p,end_p);
    ret ++;
    return (void *)ret;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr){
    ull64* bptr = ((ull64*)ptr - 1);
    ull64* cur = NULL;
    //printf("free block from %lx, stat %d\n",bptr,(*bptr)&1ULL);
    (*bptr) ^= 1ULL;
    ull64 len = (*bptr);
    ull64* ep = ((ull64*)((char*)bptr + len) - 1);
    (*ep) ^= 1ULL;
    //printf("from %lx to %lx, len %lx, val of end is %lx\n",bptr,ep,len,(*ep));
    ull64* next = ep + 1;
    ull64* pre = bptr - 1; 
    if(((*next) & 1ULL) == 0){
        len += (*next);
        ep = ((ull64*)((char*)bptr + len) - 1);
        //printf("merge the next block from %lx to %lx, len is %lx\n",next,ep,len);
        //printf("incre is %lx, incre at end is %lx\n",(*next),(*ep));
        ull64 tmplen = (*next);
        for(int i = 0;i<25;i++){
            cur = (ull64*)str_p + i;
            if((1ULL << (i + 1)) > tmplen){
                ull64* npt = (ull64*)(*cur);
                if((ull64)npt == (ull64)next){
                    (*cur) = (ull64)(*(npt+1));
                    break;
                }
                while((*(npt+1)) != (ull64)next)npt = (ull64*)(*(npt+1));

                (*(npt+1)) = (*(next+1));
                break;
            }
        }
    }

    if(((*pre) & 1ULL) == 0){
        len += (*pre);
        bptr = (ull64*)((char*)(ep+1)-len);
        //printf("merge the previous block from %lx to %lx, len is %lx\n",bptr,pre,len);
        //printf("incre is %lx, incre at head is %lx\n",(*pre),(*bptr));
        ull64 tmplen = (*pre);
        for(int i = 0;i<25;i++){
            cur = (ull64*)str_p + i;
            if((1ULL << (i + 1)) > tmplen){
                ull64* npt = (ull64*)(*cur);
                //printf("merged block found in %d th list with addr %lx with range %lx %lx\n",i,cur,1ULL<<i,1ULL<<(i+1));
                if((ull64)npt == (ull64)bptr){
                    (*cur) = (ull64)(*(npt+1));
                    break;
                }
                while((*(npt+1)) != (ull64)bptr){
                    //printf("curnode is %lx and nextnode is %lx and expected is %lx\n",npt,*(npt+1),bptr);
                    npt = (ull64*)(*(npt+1));
                }

                (*(npt+1)) = (*(bptr+1));
                break;
            }
        }
    }
    (*bptr) = len;
    (*ep)   = len;
    //printf("free block from %lx to %lx, len is %lx\n",bptr,ep,len);
    for(int i = 0;i<25;i++){
        cur = (ull64*)str_p + i;
        if((1ULL << (i + 1)) > len){
            (*(bptr+1)) = (*cur);
            (*cur) = (ull64)(bptr);
            //printf("free block is put into %d th list with addr %lx with range %lx %lx\n",i,cur,1ULL<<i,1ULL<<(i+1));
            //printf("the inserted addr is %lx\n",(*cur));
            break;
        }
    }
    //printf("current str %lx, end %lx\n\n", str_p,end_p);
    return;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size){
    ull64* oldptr = (ull64*)ptr;
    ull64* newptr;
    ull64 copySize, realsize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)return NULL;

    realsize = (*(oldptr-1))^(1ULL);
    copySize = realsize - 2*sizeof(ull64);
    if (size < copySize)copySize = size;

    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}















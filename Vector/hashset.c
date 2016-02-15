#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn)
{
    assert(elemSize > 0);
    assert(numBuckets > 0);
    assert(hashfn != NULL);
    assert(comparefn != NULL);

    h->elemSize = elemSize;
    h->numBuckets = numBuckets;
    h->comparefn = comparefn;
    h->freefn = freefn;
    h->hashfn = hashfn;
    h->count = 0;
    h->buckets = malloc(numBuckets * sizeof(vector));

    assert(h->buckets != NULL);

    for(int i = 0; i < h->numBuckets; i++){
        VectorNew(h->buckets + i,elemSize,freefn,0);
    }
}

void HashSetDispose(hashset *h)
{
    if(h->freefn != NULL){
        for(int i = 0; i < h->numBuckets; i++){
            VectorDispose(h->buckets + i);                              //free the vector elements
        }
    }
    free(h->buckets);
}

int HashSetCount(const hashset *h)
{
    return h->count;
}

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData)
{
    for(int i = 0; i < h->numBuckets; i++){
        VectorMap(h->buckets + i,mapfn,auxData);
    }
}

void HashSetEnter(hashset *h, const void *elemAddr)
{
    assert(elemAddr != NULL);

    int bucket = h->hashfn(elemAddr,h->numBuckets);            //calculate which bucket to add to

    assert(bucket > -1 && bucket < h->numBuckets);

    int position = VectorSearch(h->buckets + bucket,elemAddr,h->comparefn,0,false);
    if(position != -1){                                         //if element in vector is same as new element, replace the old element
        VectorReplace(h->buckets + bucket,elemAddr,position);
    }else{
        VectorAppend(h->buckets + bucket,elemAddr);
        h->count++;
    }
}

void *HashSetLookup(const hashset *h, const void *elemAddr)
{
    assert(elemAddr != NULL);

    int hashcode = h->hashfn(elemAddr,h->numBuckets);            //get the hashcode for the element
    assert(hashcode > -1 && hashcode < h->numBuckets);

    assert(h->buckets + hashcode != NULL);
    int isFound = VectorSearch(h->buckets + hashcode,elemAddr,h->comparefn,0,false);  //search the vector for the element

    if(isFound != -1){
        return VectorNth(h->buckets + hashcode,isFound);                            //return the element if it is found else return null
    }else{
        return NULL;
    }
}

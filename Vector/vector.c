#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation)
{
    assert(initialAllocation > -1);

    if(initialAllocation == 0)
        v->initialAllocSize = 4;
    else
        v->initialAllocSize = initialAllocation;

    v->elementSize = elemSize;
    v->freefn = freeFn;
    v->elements = malloc(v->initialAllocSize * elemSize);
    v->logicalLen = 0;
    v->allocatedLen = v->initialAllocSize;
    assert(v->elements != NULL);
}

void VectorDispose(vector *v)
{
    if(v->freefn != NULL){
        for(int i = 0; i < v->logicalLen; i++){
            void *toFree = (char *)v->elements + i * v->elementSize;
            v->freefn(toFree);   //frees elements one at a time with free function
        }
    }
    free(v->elements);   //free the elements array
}

int VectorLength(const vector *v)
{
    return v->logicalLen;
}

void *VectorNth(const vector *v, int position)
{
    assert(position > -1);
    assert(position < v->logicalLen);

    void *target = (char *)v->elements + position * v->elementSize;
    return target;
}

void VectorReplace(vector *v, const void *elemAddr, int position)
{
    assert(position > -1);
    assert(position < v->logicalLen);

    if(v->freefn != NULL){
        v->freefn(VectorNth(v,position));
    }
    void *target = VectorNth(v,position);
    memcpy(target,elemAddr,v->elementSize);
}

void VectorInsert(vector *v, const void *elemAddr, int position)
{
    assert(position > -1);
    assert(position <= v->logicalLen);

    if(v->logicalLen == v->allocatedLen){
        v->allocatedLen *= v->initialAllocSize;
        v->elements = realloc(v->elements,v->allocatedLen * v->elementSize);
    }
    void *elemPos = (char*)v->elements + position * v->elementSize;
    int backSize = ((char *)v->elements + v->logicalLen * v->elementSize) - (char *)elemPos;
    memmove((char *)elemPos + 1 * v->elementSize,elemPos,backSize); //move memory over one position
    memcpy(elemPos,elemAddr,v->elementSize);                        //copy elemaddr into the elem position
    v->logicalLen++;
}

void VectorAppend(vector *v, const void *elemAddr)
{
    if(v->logicalLen == v->allocatedLen){
        v->allocatedLen *= v->initialAllocSize;
        v->elements = realloc(v->elements,v->allocatedLen * v->elementSize);
    }
    void *target = (char *)v->elements + v->logicalLen * v->elementSize; //get end of vector pointer
    memcpy(target,elemAddr,v->elementSize);                              //memcpy address of elemaddr to target
    v->logicalLen++;
}

void VectorDelete(vector *v, int position)
{
    if(v->freefn != NULL){
        v->freefn(VectorNth(v,position));
    }
    //shift everything behind position onto the position, overwriting what was at position
    void *target = VectorNth(v,position);
    int backSize = ((char *)v->elements + v->logicalLen * v->elementSize) - (char *)target;
    memcpy(target,(char*)target+1*v->elementSize,backSize);
    v->logicalLen--;
}

void VectorSort(vector *v, VectorCompareFunction compare)
{
    assert(compare != NULL);

    qsort(v->elements,v->logicalLen,v->elementSize,compare);
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData)
{
    assert(mapFn != NULL);

    for(int i = 0; i < v->logicalLen; i++){
        void *target = (char*)v->elements + i * v->elementSize;
        mapFn(target,auxData);
    }
}

static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted)
{
    assert(v != NULL);
    assert(key != NULL);
    assert(searchFn != NULL);

    void *start = (char *)v->elements + startIndex * v->elementSize;
    int numElements = v->logicalLen - startIndex;

    if(isSorted == true){
        void *item = bsearch(key,start,numElements,v->elementSize,searchFn);
        int position = ((char*)item - (char*)start) / v->elementSize;
        return position;
    }else{
        for(int i = startIndex; i < v->logicalLen; i++){
            void *current = (char*)v->elements + i * v->elementSize;
            if(searchFn(current,key) == 0){
                return i;
            }
        }
    }
    return -1;
}

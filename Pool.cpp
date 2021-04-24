/*
    Alex Redmond, Group R

    Definitions for Pool class
*/
#include <cstdlib>
#include "Pool.h"

Pool::Pool(int blockSize, int noOfBlocks){
    this->blockSize = blockSize;
    this->poolSize = noOfBlocks;
    // Allocate pool (using calloc as the pool needs to be zeroed for free list to work)
    this->memoryPool = calloc(blockSize, noOfBlocks);
    this->nextFree = 0;
    this->blocksUsed = 0;  
}

/* void* Pool::allocate(){
    // Allocate a block and return a pointer to it
    if(this->blocksUsed < this->poolSize){
        if (this->nextFree == 0){
            // There are no previously deallocated blocks, so allocate the next free block for the first time
            return this->memoryPool + (this->blocksUsed * this->blockSize);
        }
    }
    else{
        // There are no unallocated blocks left
    }
} */
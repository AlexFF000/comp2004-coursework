/*
    Alex Redmond, Group R

    Definitions for Pool class
*/
#include <cstdlib>
#include "Pool.h"

Pool::Pool(int blockSize, int noOfBlocks){
    this->blockSize = blockSize;
    this->poolSize = noOfBlocks;
    // Allocate pool
    this->memoryPool = malloc(blockSize * noOfBlocks);
}
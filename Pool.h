/*
    Alex Redmond, Group R

    Declaration of class for implementing memory pools using free list allocation
    This class just implements a basic allocation algorithm, it doesn't handle reading from / writing to blocks once allocated
    It also does not protect against too much data being written to a block
*/
class Pool{
    public:
        // The pool is made up of blocks of the given size, and contains space for noOfBlocks
        Pool(int blockSize, int noOfBlocks);
        ~Pool();
        // Allocates a block and returns a pointer to it
        void* allocate();
        // Deallocates a block
        void deallocate(void* address);
    private:
        int blockSize, poolSize, blocksUsed;
        void * memoryPool, * nextFree;
};
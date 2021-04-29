/*
    Alex Redmond, Group R

    Declaration and definitions of FIFO buffer class
    The class is based on the producer consumer pattern and signal/wait:
        - The producer thread pushes to the buffer with addItem()
        - Consumer threads read from it with readItems
        - readItems uses requestItems to register to be woken up by addItems when the requested number of items are ready
    It uses a memory pool using the free list allocation algorithm
*/
# include "mbed.h"

template <class T>
class Buffer{
    public:
        // The buffer only accepts fixed size items
        Buffer(int maxSize){
            if (sizeof(T) < sizeof(T*)){
                // For free pointer allocation to work, the type to be stored must be at least large enough to store a pointer
                MBED_ERROR(MBED_MAKE_ERROR(MBED_MODULE_APPLICATION, MBED_ERROR_CODE_EINVAL), "T must be a large enough type to contain a pointer");  // EINVAL = 22 = Invalid Argument
            }
            else{
                this->maxSize = maxSize;
                // Create queue
                this->queue = new T*[maxSize];
                // Allocate memory pool (using calloc as pool needs to be zeroed for free list to work)
                this->pool = (T*) calloc(sizeof(T), maxSize);
                this->nextFreeBlock = nullptr;  // Point to null to indicate no deallocated blocks
                this->blocksUsed = 0;
            }
        };
        ~Buffer();
        void addItem(T item){
            if(this->itemPointersMutex.trylock_for(10000)){
                if (this->difference(this->nextEmpty, this->oldestItem) != 0){
                    // Allocate a block in pool, write item to it, and place pointer in buffer
                    T *block = this->allocate();
                    block = *item;
                    this->queue[this->nextEmpty] = block;
                    this->increment(this->nextEmpty, 1);
                    // Signal waiting threads

                    this->itemPointersMutex.unlock();
                }
                else{
                    // CRITICAL ERROR (queue is full)
                    printf("Queue full");
                }
            }
            else{
                // CRITICAL ERROR (mutex timeout)
                printf("Mutex timeout");
            }
        }
        // When the items are ready, they are written to an area of memory starting at addressToWrite
        void readItems(int quantity, int &addressToWrite, bool LIFO=false, bool removeAfterRead=false);

    private:
        void requestItems(int threadHandle, int quantity);
        void removeItems();
        // The buffer is circular, so special increment / decrement functions are useful
        int increment(int pointer, int amount);
        int decrement(int pointer, int amount);
        int difference(int firstPosition, int secondPosition){
            /* 
            Returns the circular difference between two positions in the queue (the number of places after and including firstPosition up to (but not including) secondPosition)
            This is used to calculate the amount of free or used spaces in the buffer (blocksUsed is not used for this purpose to avoid needing to worry about thread synchronisation in the alloc / dealloc methods)
            */
            int totalDifference = secondPosition - firstPosition;
            if (0 <= totalDifference) return totalDifference;
            // If totalDifference is negative secondPosition is before firstPosition so need to take into account the queue wrapping around
            else return this->maxSize + totalDifference;  // maxSize is never modified so no need for thread synchronisation 
        }

        T* allocate(){
            // Allocate a block and return a pointer to it
            if(this->blocksUsed < this->maxSize){
                if (this->nextFreeBlock == nullptr){
                    // There are no previously deallocated blocks, so allocate the next free block for the first time
                    T* newBlock = this->pool + this->blocksUsed;
                    this->blocksUsed++;
                    return newBlock;
                }
                else{
                    // Use the next deallocated block
                    T* newBlock = this->nextFreeBlock;
                    // Copy the pointer in the deallocated block, that points to the next deallocated block, into nextFreeBlock
                    this->nextFreeBlock = (T*) (int) *newBlock;
                    this->blocksUsed++;
                    return newBlock;
                }
            }
            else{
                // There are no unallocated blocks left
                return nullptr;
            }
        }

        void deallocate(T* address){
            // Write the value of nextFreeBlock to the address to be deallocated
            *address = (int)this->nextFreeBlock;
            // Convert this address to an index, and write it to nextFreeBlock
            this->nextFreeBlock = address;
            this->blocksUsed--;
        }
        int maxSize, blocksUsed, oldestItem, nextEmpty;  // maxSize is the number of places in the buffer, blocksUsed is the number of blocks in the pool that are currently allocated.  oldestItem is the index in the queue of the oldest item in the queue, nextEmpty is index of oldest free postion
        T *pool, *nextFreeBlock, **queue;  // pool points to the memory pool, nextFreeBlockBlock to the next deallocated block, and queue is an array of pointers to blocks in pool
        Mutex itemPointersMutex;  // oldestItem and nextEmpty will always be used together so can share the same mutex
};

/* template <class T>
Buffer<T>::Buffer(int maxSize){
    int itemSize = sizeof(T);
}

template <class T>
Buffer<T>::~Buffer(){

}

template <class T>
void Buffer<T>::addItem(T item){
    printf("Added");
} 

template <class T>
void Buffer<T>::readItems(int quantity, int &addressToWrite, bool LIFO, bool removeAfterRead){

}

template <class T>
void Buffer<T>::requestItems(int threadHandle, int quantity){

}
template <class T>
void Buffer<T>::removeItems(){

}
template <class T>
int Buffer<T>::increment(int pointer, int amount){
    return 0;
}

template <class T>
int Buffer<T>::decrement(int pointer, int amount){
    return 0;
} */
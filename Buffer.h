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
            this->maxSize = maxSize;
            // Allocate memory pool (using calloc as pool needs to be zeroed for free list to work)
            this->pool = (T*) calloc(sizeof(T), maxSize);
            this->nextFree = 0;
            this->blocksUsed = 0;
        };
        ~Buffer();
        void addItem(T item){
            printf("%i", (int)this->allocate());
        }
        // When the items are ready, they are written to an area of memory starting at addressToWrite
        void readItems(int quantity, int &addressToWrite, bool LIFO=false, bool removeAfterRead=false);

    private:
        void requestItems(int threadHandle, int quantity);
        void removeItems();
        // The buffer is circular, so special increment / decrement functions are useful
        int increment(int pointer, int amount);
        int decrement(int pointer, int amount);

        T* allocate(){
            // Allocate a block and return a pointer to it
            if(this->blocksUsed < this->maxSize){
                if (this->nextFree == 0){
                    // There are no previously deallocated blocks, so allocate the next free block for the first time
                    T* newBlock = this->pool + this->blocksUsed;
                    this->blocksUsed++;
                    return newBlock;
                }
                else{
                    // Use the next deallocated block
                    T* newBlock = this->pool + this->nextFree;
                    // Copy the pointer in the deallocated block, that points to the next deallocated block, into nextFree
                    this->nextFree = (int) *newBlock;
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
            // Write the value of nextFree to the address to be deallocated
            address = this->nextFree;
            // Convert this address to an index, and write it to nextFree
            this->nextFree = (address - this->pool) / sizeof(T);
        }
        int maxSize, blocksUsed, nextFree;  // The number of blocks in the pool that are currently allocated
        T* pool;
        T* queue[];
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
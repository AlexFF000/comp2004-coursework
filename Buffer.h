/*
    Alex Redmond, Group R

    Declaration and definitions of FIFO buffer class
    The class is based on the producer consumer pattern and signal/wait:
        - The producer thread pushes to the buffer with addItem()
        - Consumer threads read from it with readItems
        - readItems uses requestItems to register to be woken up by addItems when the requested number of items are ready
    Signal/Wait is used as opposed to semaphores:
        a) To allow multiple items to be read from the buffer by the same readItems call (without needing to try to aquire the semaphore multiple times)
        b) To more easily allow multiple consumer threads
    It uses a memory pool using the free list allocation algorithm  *THIS MAY NOT STILL BE TRUE*
*/
# include "mbed.h"

bool exited = false;


// Struct used by Buffer class to represent waiting consumers
struct consumer{
    osThreadId_t threadId = 0;
    int requestedItems = 0;
};

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
                // this->queue = new T*[maxSize];
                // Allocate memory pool (using calloc as pool needs to be zeroed for free list to work)
                this->pool = (T*) calloc(sizeof(T), maxSize);
                this->nextFreeBlock = nullptr;  // Point to null to indicate no deallocated blocks
                this->blocksUsed = 0;
                this->nextEmpty = 0;
                this->oldestItem = -1;  // Set to -1 initially as there are no items yet
                this->amountToDelete = 0;
                this->consumersUsingData = 0;
            }
        };
        ~Buffer();
        void addItem(T item){
            if(this->itemPointersMutex.trylock_for(10s)){
                if (this->oldestItem == -1 || this->difference(this->nextEmpty, this->oldestItem) != 0){
                    // Write to position nextEmpty in pool and increment nextEmpty
                    printf("Got difference");
                    this->pool[this->nextEmpty] = item;
                    printf("Wrote to pool");
                    if (this->oldestItem == -1){
                        // This is the first item to be added so point to it
                        this->oldestItem = this->nextEmpty;
                    }
                    this->increment(this->nextEmpty, 1);
                    // Signal waiting threads
                    if (this->waitingConsumersMutex.trylock_for(10s) && this->amountToDeleteMutex.trylock_for(10s)){
                        int spacesAvailable = difference(this->oldestItem, this->nextEmpty);
                        int spacesUsed = this->maxSize - spacesAvailable;
                        osThreadId_t consumersToWake[5];
                        int consumersToWakeLength = 0;
                        printf("Waiting: %i,  %i, %i, %i, %i, Avail: %i", waitingConsumers[0].requestedItems, waitingConsumers[1].requestedItems, waitingConsumers[2].requestedItems, waitingConsumers[3].requestedItems, waitingConsumers[4].requestedItems, spacesUsed);
                        for (int i = 0; i < 5; i++){
                            if (this->waitingConsumers[i].requestedItems != 0 && this->waitingConsumers[i].requestedItems <= spacesUsed){
                                // Increment consumersUsingData for each consumer that will be woken.  This will prevent anything being deleted until all these threads have finished with the buffer
                                this->consumersUsingData++;
                                // Add threads to array to be woken after this loop, not during it.  Otherwise a race condition could be introduced as consumersUsingData could be incomplete by the time consumers wake
                                consumersToWake[consumersToWakeLength] = this->waitingConsumers[i].threadId;
                                consumersToWakeLength++;
                                // Free slot for another consumer
                                this->waitingConsumers[i].requestedItems = 0;
                                printf("Found consumer to wake");
                            }
                        }
                        this->waitingConsumersMutex.unlock();
                        this->amountToDeleteMutex.unlock();
                        this->itemPointersMutex.unlock();
                        for (int i = 0; i < consumersToWakeLength; i++)
                            osSignalSet(consumersToWake[i], 1);

                        printf("\nAdded ");
                    }
                    else{
                        // CRITICAL ERROR (mutex timeout)
                    }
                }
                else{
                    this->itemPointersMutex.unlock();
                    // CRITICAL ERROR (queue is full)
                    if (exited == false){
                    printf("Queue full oldestItem: %i nextEmpty: %i", this->oldestItem, this->nextEmpty);
                    exited = true;
                    }
                }
            }
            else{
                // CRITICAL ERROR (mutex timeout)
                printf("Mutex timeout");  // Seems to crash somewhere Error Status: 0x80010133 Code: 307 Module: 1
            }
        }
        // When the items are ready, they are written to an area of memory starting at addressToWrite
        void readItems(int quantity, T* addressToWrite, bool LIFO=false, bool removeAfterRead=false){
            // Wait until enough items become available
            printf("About to request");
            this->requestItems(quantity);
            printf("Request done");
            if (this->itemPointersMutex.trylock_for(10000)){
                if (LIFO){
                    int spacesUsed = this->maxSize - difference(this->oldestItem, this->nextEmpty);
                    // Read from most recent rather than oldest
                    int itemsRead = 0;
                    int index = this->nextEmpty;
                    // Most recent item will the one immediately before the nextEmpty pointer
                    decrement(index, 1);

                    while (itemsRead < quantity && itemsRead < spacesUsed){
                        // Read quantity items (or as many as available if not enough) starting from the most recent
                        printf("Index LiFo is %i.  spacesUsed : %i", index, spacesUsed);
                        addressToWrite[itemsRead] = this->pool[index];
                        decrement(index, 1);
                        itemsRead++;
                    }
                }
                else{
                    // Default (FIFO) mode.  Read from oldest
                    int itemsRead = 0;
                    int index = this->oldestItem;
                    int spacesUsed = this->maxSize - this->difference(this->oldestItem, this->nextEmpty);
                    while (itemsRead < quantity && itemsRead < spacesUsed){
                        printf("Index is %i", this->oldestItem);
                        addressToWrite[itemsRead] = this->pool[index];
                        increment(index, 1);
                        itemsRead++;
                    }
                }
                if (this->amountToDeleteMutex.trylock_for(10s)){
                    if (!LIFO && removeAfterRead){
                        /* 
                            Delete the items (only available for FIFO mode. Being able to delete in both directions would be unnecessarily complicated, as no LIFO functionality needs to delete)
                            To help avoid race conditions, the items are not necessarily deleted here. Instead the amountToDelete property is modified to specify that the items should be deleted when ready
                        */
                        if (this->amountToDelete < quantity){
                            // Only increase the amountToDelete if another thread hasn't already requested a larger amount
                            this->amountToDelete = quantity;
                        }
                    }
                    // Decrement to show that this thread is no longer affected if the data is deleted
                    this->consumersUsingData--;
                    if (this->consumersUsingData == 0 && 0 < this->amountToDelete){
                        // If there is data to be deleted and no consumers need it, then delete it now
                        // Items do not need to be deleted, instead the oldestItem pointer is adjusted so they can be overwritten
                        this->increment(this->oldestItem, this->amountToDelete);
                        if(this->oldestItem == this->nextEmpty){
                            // The buffer is now empty
                            this->oldestItem = -1;
                        }
                        printf("\nRemoved ");
                    }
                    this->amountToDeleteMutex.unlock();
                    this->itemPointersMutex.unlock();
                }
                else{
                    // CRITICAL ERROR (mutex timeout)
                }
                    
                
            }
            else{
                // CRITICAL ERROR (mutex timeout)
            }
        }

    private:
        void requestItems(int quantity){
            // Add this thread, and the number of items it wants, to waitingConsumers and then wait to be woken by addItems when enough become ready
            if (this->waitingConsumersMutex.trylock_for(10s)){
                int i = 0;
                for (; i < 5; i++){
                    // Find a free slot in waitingConsumers (requestedItems being 0 indicates an unused slot)
                    if (this->waitingConsumers[i].requestedItems == 0){
                        this->waitingConsumers[i] = consumer{ThisThread::get_id(), quantity};
                        this->waitingConsumersMutex.unlock();
                        printf("About to start waiting");
                        uint32_t flags = ThisThread::flags_wait_all_for(1, 100s, true);
                        if (flags != 1){
                            printf("An error occured, the flags were %i", flags);
                        }
                        printf("%i Finished waiting", i);
                        break;
                    }
                }
                if (i == 5){
                    // There were no free slots
                    // CRITICAL ERROR (too many consumers)
                }
                /*if (this->consumersCount <= 5){
                   this->waitingConsumers[this->consumersCount - 1] = consumer{ThisThread::get_id(), quantity};
                   this->waitingConsumersMutex.unlock();
                   ThisThread::flags_wait_all_for(1, 100s, true);
                   // Get mutex again to remove this thread from waitingConsumers, as it is no longer waiting
                   if (this->waitingConsumersMutex.trylock_for(10s)){
                       this->consumersCount--;
                       this->waitingConsumersMutex.unlock();
                   }
                   else{
                       // CRITICAL ERROR (mutex timeout)
                   }
                }
                else{
                    // CRITICAL ERROR (too many consumers)
                } */
            }
            else{
                // CRITICAL ERROR (mutex timeout)
                printf("RequestItems mutex timeout");
            }
        }

        void removeItems();
        // The buffer is circular, so special increment / decrement functions are useful
        void increment(int &pointer, int amount){
            if (this->maxSize <= pointer + amount) pointer = amount - (this->maxSize - pointer);
            else pointer += amount;
        }
        void decrement(int &pointer, int amount){
            if (pointer - amount <= -1) pointer = this->maxSize - (amount - pointer);
            else pointer -= amount;
        }

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
        T *pool, *nextFreeBlock, *queue;  // pool points to the memory pool, nextFreeBlockBlock to the next deallocated block, and queue is an array of pointers to blocks in pool
        int amountToDelete, consumersUsingData;  // amountToDelete tells the delete thread how many items to delete, consumersUsingData is used for consumer threads to announce that they no longer need the data in the buffer (so the delete thread can safely delete)
        consumer waitingConsumers[5];
        Mutex itemPointersMutex, amountToDeleteMutex, waitingConsumersMutex;  // oldestItem and nextEmpty will always be used together so can share the same mutex
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
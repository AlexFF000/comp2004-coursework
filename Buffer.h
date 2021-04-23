/*
    Alex Redmond, Group R

    Declaration and definitions of FIFO buffer class
    The class is based on the producer consumer pattern and signal/wait:
        - The producer thread pushes to the buffer with addItem()
        - Consumer threads read from it with readItems
        - readItems uses requestItems to register to be woken up by addItems when the requested number of items are ready
*/
# include "mbed.h"

template <class T>
class Buffer{
    public:
        // The buffer only accepts fixed size items
        Buffer(int maxSize){
            
        };
        ~Buffer();
        void addItem(T item);
        // When the items are ready, they are written to an area of memory starting at addressToWrite
        void readItems(int quantity, int &addressToWrite, bool LIFO=false, bool removeAfterRead=false);

    private:
        void requestItems(int threadHandle, int quantity);
        void removeItems();
        // The buffer is circular, so special increment / decrement functions are useful
        int increment(int pointer, int amount);
        int decrement(int pointer, int amount);
        * pool;
        T *queue[];
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
/*
    Alex Redmond, Group R

    Declaration of FIFO buffer class
    The class is baded on the producer consumer pattern and signal/wait:
        - The producer thread pushes to the buffer with addItem()
        - Consumer threads read from it with readItems
        - readItems uses requestItems to register to be woken up by addItems when the requested number of items are ready
*/
template <class T>
class Buffer{
    public:
        // The buffer only accepts fixed size items
        Buffer(int itemSize, int maxLength);
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


};
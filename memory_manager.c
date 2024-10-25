#include "memory_manager.h"

typedef struct memoryBlock
{
    struct memoryBlock* nextBlock;
    void* memPtr;
    int size;
    bool isFree;
}memoryBlock;

void* memoryPool;
memoryBlock* firstBlock;

// memoryBlock* memoryPool;
int allocatedMemory;
int totalSize;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Initalizes the memory pool
void mem_init(size_t size)
{
    
    if(size < 0) return;
    // Allocates memory for the memory pool
    memoryPool = malloc(size);

    // Initializes the first memory block
    firstBlock = malloc(sizeof(memoryBlock));
    firstBlock->memPtr = memoryPool;
    firstBlock->size = size;
    firstBlock->isFree = true;
    firstBlock->nextBlock = NULL;
    
    totalSize = size;
    allocatedMemory = 0;
    printf("memory %ld initialized successfully!\n\n", size);
}

void* mem_alloc(size_t size)
{
    if(size + allocatedMemory > totalSize || size < 0) return NULL;

    pthread_mutex_lock(&lock);
    memoryBlock* current = firstBlock;
    //printf("trying to allocate %ld memory\n", size);
    while(current)
    {
        if (current->isFree && current->size >= size)
        { 
            if(current->size > size)
            {
                memoryBlock* newBlock = malloc(sizeof(memoryBlock));

                newBlock->memPtr = (char*)memoryPool + allocatedMemory + size;
                newBlock->size = current->size-size;
                newBlock->isFree = true;
                newBlock->nextBlock = current->nextBlock;

                current->nextBlock = newBlock;
            }
            current->isFree = false;
            current->size = size;
            allocatedMemory += size;
          //  printf("%ld memory allocated successfully!\n", size);
            pthread_mutex_unlock(&lock);
            return current->memPtr;
        }
        current = current->nextBlock;
    }
    pthread_mutex_unlock(&lock);
    printf("allocation %ld failed unexpectedly!\n", size);
    return NULL;
}

// Frees the memory of a block
void mem_free(void* block)
{
    if(!block) return;
    pthread_mutex_lock(&lock);
    memoryBlock* current = firstBlock;
    memoryBlock* prev = NULL;

    while(current)
    {
        if(current->memPtr == block)
        {
            current->isFree = true;
            allocatedMemory -= current->size;
            if(prev && prev->isFree)
            {
                prev->size += current->size;
                prev->nextBlock = current->nextBlock;
                free(current);
                current = prev;
            }
            if(current->nextBlock && current->nextBlock->isFree)
            {
                current->size += current->nextBlock->size;
                memoryBlock* temp = current->nextBlock;
                current->nextBlock = temp->nextBlock;
                free(temp);
            }
            pthread_mutex_unlock(&lock);
            //printf("memory freed!\n");
            return;
        }
        prev = current;
        current = current->nextBlock;
    }
    pthread_mutex_unlock(&lock);
    printf("Couldn't find block!\n");
}

void* mem_resize(void* block, size_t size)
{
    //printf("Trying to resize block...\n");
    if(!block || size < 0) return NULL;

    pthread_mutex_lock(&lock);

    // Finds the block
    memoryBlock* ptr = firstBlock;
    memoryBlock* prev = NULL;


    while (ptr && ptr->memPtr != block)
    {
        prev = ptr;
        ptr = ptr->nextBlock;
    }

    // Case: Block does not exist
    if(!ptr)
    {
        pthread_mutex_unlock(&lock);
        return NULL;
    }

    // Case: Does the memory pool have enough allocated memory?
    if(allocatedMemory - ptr->size + size > totalSize)
    {
        pthread_mutex_unlock(&lock);
        return NULL;
    }
    // Case: No change in size
    if(ptr->size == size)
    {
        pthread_mutex_unlock(&lock);
        return ptr->memPtr;
    }

    // Case: ptr is bigger than new size, splitting the block
    if(ptr->size > size)
    {
        allocatedMemory = allocatedMemory - ptr->size + size;

        // Splitting block
        memoryBlock* newBlock = malloc(sizeof(memoryBlock));
        newBlock->memPtr = (char*)ptr->memPtr+size;
        newBlock->size = ptr->size-size;
        newBlock->isFree = true;
        newBlock->nextBlock = ptr->nextBlock;

        ptr->nextBlock = newBlock;
        ptr->size = size;

        pthread_mutex_unlock(&lock);
        return ptr->memPtr;
    }

    // Case: Ptr can merge with previous block
    if(prev && prev->isFree && (prev->size + ptr->size >= size))
    {
        allocatedMemory = allocatedMemory - ptr->size + size;

        prev->nextBlock = ptr->nextBlock;
        prev->size = size;
        
        // Splitting block
        if(prev->size > size)
        {
            memoryBlock* newBlock = malloc(sizeof(memoryBlock));
            newBlock->memPtr = (char*)prev->memPtr+size;
            newBlock->size = prev->size -size;
            newBlock->isFree = true;
            newBlock->nextBlock = ptr->nextBlock;

            prev->nextBlock = newBlock;
        }
        pthread_mutex_unlock(&lock);
        return prev->memPtr;
    }


    // Case: Ptr can merge with next block
    if (ptr->nextBlock && ptr->nextBlock->isFree && (ptr->size + ptr->nextBlock->size >= size))
    {
        allocatedMemory =allocatedMemory- ptr->size + size;

        ptr->nextBlock = ptr->nextBlock->nextBlock;
        ptr->size = size;

        // Splitting the block if possible
        if(ptr->size> size)
        {
            memoryBlock* newBlock = malloc(sizeof(memoryBlock));
            newBlock->memPtr = (char*)ptr->memPtr+size;
            newBlock->size = ptr->size + ptr->nextBlock->size - size;
            newBlock->isFree = true;
            newBlock->nextBlock = ptr->nextBlock;

            ptr->nextBlock = newBlock;
        }
        pthread_mutex_unlock(&lock);
        return ptr->memPtr;
    }

    // Case: Find suitable block
    memoryBlock* current = firstBlock;
    while(current)
    {
        if(current->isFree && current->size >= size)
        {
            allocatedMemory = allocatedMemory - ptr->size + size;
            memcpy(current->memPtr, ptr->memPtr, ptr->size);
            current->isFree = false;
            current->size = size;

            mem_free(ptr);

            pthread_mutex_unlock(&lock);
            return current->memPtr;
        }
        current = current->nextBlock;
    }

    // Nothing changed...
    pthread_mutex_unlock(&lock);
    return NULL;
}


void mem_deinit()
{
    memoryBlock* current = firstBlock;
    while(current)
    {
        memoryBlock* next = current->nextBlock;
        free(current);
        current = next;
    }
    free(memoryPool);
    firstBlock = NULL;
    memoryPool = NULL;
    totalSize = 0;
    printf("Memory pool deinitialized!\n\n");
}


// int main()
// {
//     mem_init(1024);
//     void* block1 = mem_alloc(256);
//     mem_free(block1);
//     void* block2 = mem_alloc(512);
    
//     void* block3 = mem_alloc(256);

//     printf("Resizing block!");
//     block3 = mem_resize(block3, 100);


        


//     printf("freeing memory:\n");

//    // mem_free(block1);
//     mem_free(block2);
//     mem_free(block3);

//     printf("Deinit memory!");
//     mem_deinit();


//     return 0;
// }

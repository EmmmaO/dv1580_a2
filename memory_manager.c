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
    printf("memory initialized successfully!\n");
}

void* mem_alloc(size_t size)
{
    if(size + allocatedMemory > totalSize || size < 0) return NULL;

    memoryBlock* current = firstBlock;
    printf("trying to allocate memory\n");
    while(current)
    {
        if (current->isFree && current->size >= size)
        {
            pthread_mutex_lock(&lock);   
            if(current->size > size)
            {
                memoryBlock* newBlock = malloc(sizeof(memoryBlock));

                newBlock->memPtr = (char*)memoryPool + allocatedMemory + size;
                newBlock->size = current->size-size;
                newBlock->isFree = true;
                newBlock->nextBlock = NULL;

                current->nextBlock = newBlock;
            }
            current->isFree = false;
            current->size = size;
            allocatedMemory += size;
            printf("memory allocated successfully!\n");
            pthread_mutex_unlock(&lock);
            return current->memPtr;
        }
        current = current->nextBlock;
    }
    printf("allocation failed unexpectantly!\n");
    pthread_mutex_unlock(&lock);
    return NULL;
}

// Frees the memory of a block
void mem_free(void* block)
{
    if(!block) return;
    memoryBlock* current = firstBlock;

    while(current)
    {
        if(current->memPtr == block)
        {
            allocatedMemory -= current->size;
            current->isFree = true;
            if(current->nextBlock && current->nextBlock->isFree)
            {
                current->size += current->nextBlock->size;
                memoryBlock* temp = current->nextBlock;
                current->nextBlock = current->nextBlock->nextBlock;
                free(temp);
            }
            printf("memory freed!\n");
            return;
        }
        current = current->nextBlock;
    }
    printf("Couldn't find block!\n");
}

void* mem_resize(void* block, size_t size)
{
    if(!block || size < 0) return NULL;

    // Finds the block
    memoryBlock* ptr = firstBlock;

    while (ptr && ptr->memPtr != block)
    {
        ptr = ptr->nextBlock;
    }

    // Case: Block does not exist
    if(!ptr) return NULL;

    // Case: Does the memory pool have enough allocated memory?
    if(allocatedMemory - ptr->size + size > totalSize) return NULL;

    // Case: No change in size
    if(ptr->size == size) return NULL;

    // Case: ptr is bigger than new size, splitting the block
    if(ptr->size > size)
    {
        memoryBlock* newBlock = malloc(sizeof(memoryBlock));
        newBlock->memPtr = (char*)ptr->memPtr+size;
        newBlock->size = ptr->size-size;
        newBlock->isFree = true;
        newBlock->nextBlock = ptr->nextBlock;

        ptr->nextBlock = newBlock;
        ptr->size = size;
        return ptr->memPtr;
    }

    // Case: Ptr can merge with next block
    if (ptr->nextBlock && ptr->nextBlock->isFree)
    {
        if(ptr->size + ptr->nextBlock->size >= size)
        {
            size_t blockSize = ptr->size + ptr->nextBlock->size; 
            ptr->nextBlock = ptr->nextBlock->nextBlock;
            ptr->size = size;
            // Splitting the block if possible
            if(blockSize > size)
            {
                memoryBlock* newBlock = malloc(sizeof(memoryBlock));
                newBlock->memPtr = (char*)ptr->memPtr+size;
                newBlock->size = ptr->size-size;
                newBlock->isFree = true;
                newBlock->nextBlock = ptr->nextBlock;

                ptr->nextBlock = newBlock;
            }
            return ptr->memPtr;
        }
    }
    return NULL;

/*    memoryBlock* walker = firstBlock;
    while (walker)
    {
        if(walker->nextBlock && walker == ptr && walker->nextBlock->isFree || walker->nextBlock == ptr && walker->isFree || walker->isFree)
        {
            if (walker->size >= size)
            {
                if(walker->size > size)
                {
                    memoryBlock* newBlock = malloc(sizeof(memoryBlock));
                    newBlock->memPtr = (char*)walker->memPtr+size;
                    newBlock->size = walker->size-size;
                    newBlock->isFree = true;
                    newBlock->nextBlock = walker->nextBlock;

                    walker->nextBlock = newBlock;
                    walker->size = size;
                }
                memcpy(walker, ptr, ptr->size);
                if(walker != ptr)
                    mem_free(ptr);
                return(walker->memPtr);
            }
            
        }
        walker = walker->nextBlock;
    }
    printf("Failed to resize block!\n");

    



    // if(!newBlock) return NULL;

    // memcpy(newBlock,ptr,size);
    // mem_free(ptr);

    return newBlock->memPtr;
    */
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
    printf("Memory pool deinitialized!\n");
}


int main()
{
    mem_init(1024);
    void* block1 = mem_alloc(512);
    mem_free(block1);
    void* block2 = mem_alloc(512);
    
    void* block3 = mem_alloc(1);

    printf("Resizing block!");
    block3 = mem_resize(block3, 100);

    printf("freeing memory:\n");

    mem_free(block1);
    mem_free(block2);
    mem_free(block3);

    printf("Deinit memory!");
    mem_deinit();

    return 0;
}



// // memory Block
// typedef struct memoryBlock
// {
//     size_t size;
//     bool is_free;
//     struct memoryBlock* nextBlock;
// }memoryBlock;

// //Global variables
// memoryBlock* memoryPool = NULL;
// size_t poolSize = 0;
// size_t allocatedSize = 0;
// void* memory_address;

// //Initializes the memory pool
// void mem_init(size_t size)
// {
//     memory_address = malloc(size*sizeof(memoryBlock));
//     printf("\nInitialized: %ld\n", size*sizeof(memoryBlock));
//     if(!memory_address)
//     {
//         printf("Memory initializing failed!\n");
//         exit(EXIT_FAILURE);
//     }
//     memoryPool = (memoryBlock*)((char*)memory_address);
//     memoryPool->size = size*sizeof(memoryBlock);
//    // printf("memoryPool->size: %ld, size: %ld, sizeof(memoryBlock): %ld\n", memoryPool->size, size, sizeof(memoryBlock));
//     memoryPool->is_free = true;
//     memoryPool->nextBlock = NULL;
//     poolSize = size*sizeof(memoryBlock);
//     printf("Memory initialized!\n\n");
// }

// //Allocated a block of size in memory pool
// void* mem_alloc(size_t size)
// {
//     printf("Trying to allocate: %ld (total: %ld) has: %ld left\n", size, allocatedSize+size, poolSize/sizeof(memoryBlock)-allocatedSize);
    
//     if (poolSize >= sizeof(memoryBlock)*(allocatedSize + size))
//     {
//         memoryBlock* current = memoryPool;
//         while(current)
//         {
//             //printf("current block size: %ld\n", current->size/sizeof(memoryBlock));
//             if (current->is_free && size*sizeof(memoryBlock) <= current->size || current->size == 0)
//             {
//                 //Splites the block if there is memory left
//                 if (current->size > size + sizeof(memoryBlock))
//                 {
//                     memoryBlock* newMemBlock = (memoryBlock*)((char*)current+sizeof(memoryBlock)+size);
//                     newMemBlock->size = current->size - size*sizeof(memoryBlock);            
//                     //newMemBlock->size = poolSize - sizeof(memoryBlock)*(allocatedSize - size);
//                     newMemBlock->is_free = true;
//                     newMemBlock->nextBlock = current->nextBlock;
//                     //newMemBlock->prevBlock = current;

//                   //  printf("nmb.s: %ld\n", (newMemBlock->size)/sizeof(memoryBlock));

//                     current->size = size*sizeof(memoryBlock);
//                     current->nextBlock = newMemBlock;
//                     printf("Memory block split!\n");
//                     //printf("Current: %ld  nextBlock: %ld\n", current->size/sizeof(memoryBlock), newMemBlock->size/sizeof(memoryBlock));
//                 }
//                 current->is_free = false;
//                 printf("Memory allocated!\n\n");
//                 allocatedSize += size;// + sizeof(memoryBlock);
//               //  printf("Allocated size: %ld\n", allocatedSize);
//                 return (void*)((char*)current + sizeof(memoryBlock));
//             }
//             current = current->nextBlock;
//         }
//     }
//     else
//     {
//         printf("Trying to allocate %ld (total %ld), but only have %ld left.\n", size, allocatedSize+size, (poolSize)/sizeof(memoryBlock) - allocatedSize);
//         printf("Not enought space! Allocation failed!\n");
//         printf("Failed to allocate memory!\n\n");
//     }
//     return NULL;
// }

// // Frees memory in a specific block
// void mem_free(void* block)
// {
//     if (!block)
//     {
//         printf("Cannot free null block\n");
//         return;
//     }
    
//     memoryBlock *thisblock = (memoryBlock*)((char*)block - sizeof(memoryBlock));
//     thisblock->is_free = true;
//     allocatedSize -= thisblock->size/sizeof(memoryBlock);

//     memoryBlock *current = memoryPool;
//     printf("Memory block freed (freed: %ld)  have: %ld left\n\n", thisblock->size/sizeof(memoryBlock), poolSize/sizeof(memoryBlock) - allocatedSize);    

//     while(current)
//     {
//         if(current->nextBlock)
//         {
//             if(current->is_free && current->nextBlock->is_free)
//             {
//                 current->size += current->nextBlock->size;
//                 current->nextBlock = current->nextBlock->nextBlock;
//             }
//         }
//         current = current->nextBlock;
//     }

// }

// //Resizes a specific block
// void* mem_resize(void* block, size_t size)
// {
//     if(!block)
//     {
//         printf("Can't resize a non-existing block!");
//         return NULL;
//     }

//     memoryBlock* walker = (memoryBlock*)((char*)block - sizeof(memoryBlock));
    

//     memoryBlock* newBlock = mem_alloc(size);
//     if(newBlock)
//     {
//         size_t copy_size = (walker->size < size) ? walker->size : size;
//         memcpy(newBlock,block,copy_size);
//         mem_free(block);
//     }
//     return newBlock;

// }

// // Frees the memory in memory pool
// void mem_deinit()
// {
//     if(memory_address)
//     {
//         free(memory_address);
//         memoryPool = NULL;
//         memory_address = NULL;   
//         poolSize = 0;
//     }
// }

// // int main()
// // {
// //     mem_init(1024);
// //     mem_alloc(500);
// //     mem_alloc(300);
// //     mem_alloc(200);

// //     printf("Memory pool size: %zu\n", memoryPool->size);
// //     printf("Pool size: %ld\n", poolSize);
// //     printf("Alloxated size: %ld\n", allocatedSize);

// //     return 0;
// // }

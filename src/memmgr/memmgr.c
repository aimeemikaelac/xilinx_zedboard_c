//----------------------------------------------------------------
// Statically-allocated memory manager
//
// by Eli Bendersky (eliben@gmail.com)
//  
// This code is in the public domain.
//----------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "memmgr.h"
#include "user_mmap_driver.h"
#include "pthread.h"

typedef ulong Align;

union mem_header_union
{
    struct 
    {
        // Pointer to the next block in the free list
        //
        union mem_header_union* next;

        // Size of the block (in quantas of sizeof(mem_header_t))
        //
        ulong size; 

	// Nonce to use to check if struct is valid in the assert
	//
	unsigned nonce;

	// Second nonce to use to check if struct is valid in assert
	//
	unsigned nonce2;

	// Offset from pool start
	//
	unsigned offset;
	
    } s;

    // Used to align headers in memory to a boundary
    //
    Align align_dummy;
};

typedef union mem_header_union mem_header_t;

static unsigned nonce = 99999999;

static unsigned nonce2 = 298709375;

static unsigned largest_allocation = 0;
// Initial empty list
//
static mem_header_t base;
//base.s.nonce = nonce;
//base.s.nonce2 = nonce2;

// Start of free list
//
static mem_header_t* freep = 0;

// Static pool for new allocations
//
//static byte pool[POOL_SIZE] = {0};

//UPDATED: use length provided in init as the POOL_SIZE, rather than a macro
static unsigned POOL_SIZE = 0;

//UPDATED: changed byte pool to point at the buffer in the fpga shared memory area
//use a buffer passed into the init
static byte *pool = 0;

//UPDATE: store the input base address to perform lookups
static unsigned base_address = 0;

static ulong pool_free_pos = 0;

static session = 0;

static shared_memory shared_mem = NULL;

//add suport for multi threads with lazy synchronization
static pthread_mutex_t lock;
static int initialized = 0;
static pthread_once_t create_lock_once = PTHREAD_ONCE_INIT;
static pthread_mutexattr_t attr;


void createLock(){
//	printf("\nInitializing memmgr reentrant lock");
	if(initialized == 0){
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&lock, &attr);
	}
	initialized = 1;
}

//length should be entire length of the shared memory region for the 
//memory allocation to work right
//TODO: if memmgr is used by another process after a fork, then the 
//pointer that we get from mmap is invalid. However, the data in memory
//will still be valid. Either each process needs to re-map, or the 
//list must be destroyed. If we get a bunch of segfaults in alloc,
//this is probably the problem
//Another possibility is to always return a struct as the current
//memmgr object and alloc using that struct
void memmgr_init(void* buffer, unsigned length, unsigned baseAddress)
{
    pthread_once(&create_lock_once, createLock);
//    printf("\nIn memmgr main init()");
/*    if(lock == NULL){
	lock = (pthread_mutex_t)malloc(sizeof(pthread_murex_t));
	if(lock == NULL){
		printf("\nCould not allocate lock");
		return;
	}
    	pthread_mutex_init(lock, NULL);
	session = 1;
    } else{
	    return;
    }*/
    pthread_mutex_lock(&lock);
  //  printf("\nIn critical section of main init function");
    if(session == 0){
	    base.s.next = 0;
	    base.s.size = 0;
	    base.s.nonce = nonce;
	    base.s.nonce2 = nonce2;
	    freep = 0;
	    pool_free_pos = 0;
	    pool = (byte*)(buffer);
	    POOL_SIZE = length;
	    base_address = baseAddress;
	    session = 1;
    }
//    printf("\nInput buffer: %p", buffer);
//    printf("\nExiting critical section. Is pool NULL? %i. Pool is : %p", pool == NULL? 1:0, pool);
    pthread_mutex_unlock(&lock);

//    printf("\nPool pointer: %p", pool);

}

void memmgr_init_check(void* buffer, unsigned length, unsigned baseAddress){
	pthread_once(&create_lock_once, createLock);
//	printf("\nIn memmgr check()");
	pthread_mutex_lock(&lock);
	if(session == 0	){
		memmgr_init(buffer, length, baseAddress);
	}
	pthread_mutex_unlock(&lock);
}

int memmgr_init_shared_short(){
	return memmgr_init_check_shared_mem(SHARED_SIZE, UIO_DEVICE, BASE_ADDRESS);
}

int memmgr_init_check_shared_mem(unsigned length, char* uioDevice, unsigned baseAddress){
	pthread_once(&create_lock_once, createLock);
	pthread_mutex_lock(&lock);
//	printf("\nAm in memmmgr check shared()");
	if(session == 1){
//		printf("\nA session already exists. Not starting a new one");
		pthread_mutex_unlock(&lock);
		return -2;
	}

	if(shared_mem == NULL){
		printf("\nAccessing new uio memory region");
//		shared_mem = getUioMemoryArea(uioDevice, length);
		shared_mem = getSharedMemoryArea(BASE_ADDRESS, SHARED_SIZE);		
		if(shared_mem == NULL){
			printf("\nError getting UIO shared memory area");
			pthread_mutex_unlock(&lock);
			return -1;
		}
	}
	//for now, create a new session if one does not exist. 
	//assume that a session does not exist
	//TODO: catch if a session already exists and take ? some action?
	memmgr_init_check((void*)(shared_mem->ptr), length, baseAddress);
	pthread_mutex_unlock(&lock);
	return 0;
}

void memmgr_print_stats()
{
    pthread_mutex_lock(&lock);
    if(pool == NULL){
	    printf("\nMemmgr pool was invalidated by another thread. in print stats");
	    pthread_mutex_unlock(&lock);
	    return;
    }
    #ifdef DEBUG_MEMMGR_SUPPORT_STATS
    mem_header_t* p;

    printf("------ Memory manager stats ------\n\n");
    printf(    "Pool: free_pos = %lu (%lu bytes left)\n\n", 
            pool_free_pos, POOL_SIZE - pool_free_pos);

    p = (mem_header_t*) pool;

    while (p < (mem_header_t*) (pool + pool_free_pos))
    {
        printf(    "  * Addr: 0x%8p; Size: %8lu\n",
                p, p->s.size);

        p += p->s.size;
    }

    printf("\nFree list:\n\n");

    if (freep)
    {
        p = freep;

        while (1)
        {
            printf(    "  * Addr: 0x%8p; Size: %8lu; Next: 0x%8p\n", 
                    p, p->s.size, p->s.next);

            p = p->s.next;

            if (p == freep)
                break;
        }
    }
    else
    {
        printf("Empty\n");
    }
    
    printf("\n");
    #endif // DEBUG_MEMMGR_SUPPORT_STATS
    pthread_mutex_unlock(&lock);
}


static mem_header_t* get_mem_from_pool(ulong nquantas)
{
    pthread_mutex_lock(&lock);
    if(pool == NULL){
	    printf("\nMemmgr pool was invalidated by another thread. in get mem from pool");
	    pthread_mutex_unlock(&lock);
	    return NULL;
    }
    ulong total_req_size;

    mem_header_t* h;

    if (nquantas < MIN_POOL_ALLOC_QUANTAS)
        nquantas = MIN_POOL_ALLOC_QUANTAS;

    total_req_size = nquantas * sizeof(mem_header_t);

    if (pool_free_pos + total_req_size <= POOL_SIZE)
    {
        h = (mem_header_t*) (pool + pool_free_pos);
        h->s.size = nquantas;
	h->s.nonce = nonce;
	h->s.nonce2 = nonce2;
	h->s.offset = pool_free_pos + sizeof(mem_header_t);
        memmgr_free((void*) (h + 1));
        pool_free_pos += total_req_size;
    }
    else
    {
	pthread_mutex_unlock(&lock);
        return 0;
    }

    pthread_mutex_unlock(&lock);
    return freep;
}


// Allocations are done in 'quantas' of header size.
// The search for a free block of adequate size begins at the point 'freep' 
// where the last block was found.
// If a too-big block is found, it is split and the tail is returned (this 
// way the header of the original needs only to have its size adjusted).
// The pointer returned to the user points to the free space within the block,
// which begins one quanta after the header.
//
void* memmgr_alloc(ulong nbytes)
{
    pthread_mutex_lock(&lock);
    mem_header_t* p;
    mem_header_t* prevp;

//    printf("\nmemmgr attempting to allocate: %lu bytes", nbytes);

    if(pool == NULL){
	    printf("\nPool is null. cannot allocate in memmgr");
	    pthread_mutex_unlock(&lock);
	    return NULL;
    } else if(session == 0){
	    printf("\nNot in a session. Will not allocate in memmgr");
	    pthread_mutex_unlock(&lock);
	    return NULL;
    }

	//printf("\nmemmgr-------------------------------------------\n");
    // Calculate how many quantas are required: we need enough to house all
    // the requested bytes, plus the header. The -1 and +1 are there to make sure
    // that if nbytes is a multiple of nquantas, we don't allocate too much
    //
    ulong nquantas = (nbytes + sizeof(mem_header_t) - 1) / sizeof(mem_header_t) + 1;

    // First alloc call, and no free list yet ? Use 'base' for an initial
    // denegerate block of size 0, which points to itself
    // 
    if ((prevp = freep) == 0)
    {
        base.s.next = freep = prevp = &base;
        base.s.size = 0;
    }
//	printf("\nPool: %p", pool);
//	printf("\nmemmgr-------------------------------------------\n");
    for (p = prevp->s.next; ; prevp = p, p = p->s.next)
    {
        // big enough ?
        if (p->s.size >= nquantas) 
        {
//	    printf("\nmemmgr-------------------------------------------1\n");
            // exactly ?
            if (p->s.size == nquantas)
            {
//	    printf("\nmemmgr------------------------------------------2\n");
                // just eliminate this block from the free list by pointing
                // its prev's next to its next
                //
                prevp->s.next = p->s.next;
            }
            else // too big
            {
//	    printf("\nmemmgr-------------------------------------------2\n");
                p->s.size -= nquantas;
                p += p->s.size;
                p->s.size = nquantas;
            }

//	    printf("\nmemmgr-------------------------------------------3\n");
            freep = prevp;
	    if(nbytes > largest_allocation){
		    largest_allocation = nbytes;
	    }
//	    printf("\nmemmgr  allocated: %lu bytes", nbytes);
            pthread_mutex_unlock(&lock);
            return (void*) (p + 1);
        }
        // Reached end of free list ?
        // Try to allocate the block from the pool. If that succeeds,
        // get_mem_from_pool adds the new block to the free list and
        // it will be found in the following iterations. If the call
        // to get_mem_from_pool doesn't succeed, we've run out of
        // memory
        //
        else if (p == freep)
        {
            if ((p = get_mem_from_pool(nquantas)) == 0)
            {
                #ifdef DEBUG_MEMMGR_FATAL
                printf("!! Memory allocation failed !!\n");
                #endif
		pthread_mutex_unlock(&lock);
                return 0;
            }
        }
    }
    pthread_mutex_unlock(&lock);
}


// Scans the free list, starting at freep, looking the the place to insert the 
// free block. This is either between two existing blocks or at the end of the
// list. In any case, if the block being freed is adjacent to either neighbor,
// the adjacent blocks are combined.
//
void memmgr_free(void* ap)
{
    pthread_mutex_lock(&lock);
    if(pool == NULL){
	    printf("\nMemmgr pool was invalidated by another thread. In free");
	    pthread_mutex_unlock(&lock);
	    return;
    }
    mem_header_t* block;
    mem_header_t* p;

    if(ap == NULL){
	    pthread_mutex_unlock(&lock);
	    return;
    }
    // acquire pointer to block header
    block = ((mem_header_t*) ap) - 1;

    // Find the correct place to place the block in (the free list is sorted by
    // address, increasing order)
    //
    for (p = freep; !(block > p && block < p->s.next); p = p->s.next)
    {
        // Since the free list is circular, there is one link where a 
        // higher-addressed block points to a lower-addressed block. 
        // This condition checks if the block should be actually 
        // inserted between them
        //
        if (p >= p->s.next && (block > p || block < p->s.next))
            break;
    }

    // Try to combine with the higher neighbor
    //
    if (block + block->s.size == p->s.next)
    {
        block->s.size += p->s.next->s.size;
        block->s.next = p->s.next->s.next;
    }
    else
    {
        block->s.next = p->s.next;
    }

    // Try to combine with the lower neighbor
    //
    if (p + p->s.size == block)
    {
        p->s.size += block->s.size;
        p->s.next = block->s.next;
    }
    else
    {
        p->s.next = block;
    }

    freep = p;
    pthread_mutex_unlock(&lock);
}

//looks up the physical address of a buffer that was allocated by the
//memmgr_alloc function in the shared memory region indicated by uioDev
//needs to be the base pointer, else will fail
unsigned lookupBufferPhysicalAddress(void* ap){
	pthread_mutex_lock(&lock);
    	if(pool == NULL){
		printf("\nMemmgr pool was invalidated by another thread. In lookup");
		pthread_mutex_unlock(&lock);
		return -1;
	}
	mem_header_t* block;
	unsigned base_ptr, buffer_ptr, offset;
	//get the base address from the uio device
	unsigned baseAddress = base_address;
	//lookup the header for the block
    //	block = ((mem_header_t*) ap) - 1;
//	printf("\nPointer address being looked up: %p", ap);
	base_ptr = (unsigned)pool;
	buffer_ptr = (unsigned)ap;
	//calculate the address using the header's offset
	offset = buffer_ptr - base_ptr;
//	offset = block->s.offset;
	pthread_mutex_unlock(&lock);
	return baseAddress + offset;
}


void memmgr_assert(void* ap){
    pthread_mutex_lock(&lock);
    if(pool == NULL){
	    printf("\nMemmgr pool was invalidated by another thread. In assert");
	    pthread_mutex_unlock(&lock);
	    abort();
    }
    mem_header_t* block;
    int i;
    //Need to loop to the base pointer, in case a pointer is tested
    //that is offset from the base of its allocation
    // acquire pointer to block header
    //
    //should actually only need to check that the input pointer is
    //betweent he base pointer and the base pointer + the length
    unsigned basePointer = (unsigned)pool;
    unsigned poolEnd = basePointer + POOL_SIZE;
    unsigned currentPointer = (unsigned)ap;
    if(currentPointer > poolEnd || currentPointer < basePointer){
	    printf("\nMEMMGR------------------------------");
	    printf("\nPointer is not in the correct memory area. Aborting");
	    pthread_mutex_unlock(&lock);
	    abort();
    }

    for(i=0; i<largest_allocation; i++){
	    block = ((mem_header_t*) ap) - i;
	    //check if the nonces in the struct are correct
	    if(block->s.nonce != nonce && block->s.nonce2 != nonce2){
		    pthread_mutex_unlock(&lock);
		    return;
	    }
    }
    //if this loop finishes without returning, then a header was not found in memory
    //in front of the pointer. a seg fault might also happen, which would have the same
    //effect, as we abort now
    printf("\nMEMMGR-------------------------------");
    printf("\nCould not find header with valid nonce. Aborting.");
    pthread_mutex_unlock(&lock);
    abort();
}

void memmgr_destroy(){
	/*
	 * Need to clear everything set by the init() function
    base.s.next = 0;
    base.s.size = 0;
    base.s.nonce = nonce;
    base.s.nonce2 = nonce2;
    freep = 0;
    pool_free_pos = 0;
    pool = (byte*)(buffer);
    POOL_SIZE = length;
    base_address = baseAddress;
    session = 1;
    */
	pthread_mutex_lock(&lock);
	printf("\nAm in memmgr destroy()! Are you sure? ---------------------------------------");
	session = 0;
	if(shared_mem != NULL){
		cleanupSharedMemoryPointer(shared_mem);
		shared_mem = NULL;
	}
	POOL_SIZE = 0;
	pool = NULL;
	base.s.next = 0;
	base.s.size = 0;
	freep = 0;
	pool_free_pos = 0;
	base_address = 0;
	pthread_mutex_unlock(&lock);
}

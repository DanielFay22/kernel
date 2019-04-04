

/*
 * Simple, 32-bit and 64-bit clean allocator based on an implicit free list,
 * first fit placement, and boundary tag coalescing, as described in the
 * CS:APP3e text.  Blocks are aligned to double-word boundaries.  This
 * yields 8-byte aligned blocks on a 32-bit processor, and 16-byte aligned
 * blocks on a 64-bit processor.  However, 16-byte alignment is stricter
 * than necessary; the assignment only requires 8-byte alignment.  The
 * minimum block size is four words.
 *
 * This allocator uses the size of a pointer, e.g., sizeof(void *), to
 * define the size of a word.  This allocator also uses the standard
 * type uintptr_t to define unsigned integers that are the same size
 * as a pointer, i.e., sizeof(uintptr_t) == sizeof(void *).
 */

#include <system.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>


/* Basic constants and macros: */
#define WSIZE      sizeof(void *) /* Word and header/footer size (bytes) */
#define ALIGN	   8              /* Align to 8 bytes */
#define DSIZE      (2 * WSIZE)    /* Doubleword size (bytes) */
#define CHUNKSIZE  (1 << 12)      /* Extend heap by this amount (bytes) */

#define MAX(x, y)  ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word. */
#define PACK(size, alloc)  		((size) | (alloc))

/* Read and write a word at address p. */
#define GET(p)       (*(uintptr_t *)(p))
#define PUT(p, val)  (*(uintptr_t *)(p) = (val))

/* Read the size, allocated, and color fields from address p. */
#define GET_SIZE(p)   (GET(p) & ~(ALIGN - 1))
#define GET_ALLOC(p)  (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer. */
#define HDRP(bp)  ((char *)(bp) - WSIZE)
#define FTRP(bp)  ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks. */
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

// Smallest size class.
#define MIN_CLASS	5
#define MIN_SIZE	(1 << MIN_CLASS)

// Largest size class.
#define MAX_CLASS	20
#define MAX_SIZE	(1 << MAX_CLASS)


// Macro to determine size class from size.
// This is the fastest available command (5 instructions)
// As the Xeon processors in CLEAR do no support clz or ctz instructions.
#define GET_BIN(c)	(32u - __builtin_clz(c))


/* Global variables: */
static char *heap_listp; /* Pointer to first block */

/* Function prototypes for internal helper routines: */
static void *coalesce(void *bp);
//static void *extend_heap(size_t words);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);


/* 
 * Requires:
 *   None.
 *
 * Effects:
 *   Initialize the memory manager.  Returns 0 if the memory manager was
 *   successfully initialized and -1 otherwise.
 */
int
heap_init(void)
{

        /* Create the initial empty heap. */
        heap_listp = heap;

	PUT(heap_listp, 0);                            /* Alignment padding */
        PUT(heap_listp + (1 * WSIZE), PACK(ALIGN, 1)); /* Prologue header */
        PUT(heap_listp + (2 * WSIZE), PACK(ALIGN, 1)); /* Prologue footer */

        char *fblock = heap_listp + 2 * WSIZE;
        size_t bsize = ALIGN * ((HEAP_SIZE - 3 * WSIZE) / ALIGN);
        PUT(HDRP(fblock), bsize);
        PUT(FTRP(fblock), bsize);

        PUT(HDRP(NEXT_BLKP(fblock)), PACK(0, 1));     /* Epilogue header */
        heap_listp += (2 * WSIZE);

        return (0);
}

/* 
 * Requires:
 *   None.
 *
 * Effects:
 *   Allocate a block with at least "size" bytes of payload, unless "size" is
 *   zero.  Returns the address of this block if the allocation was successful
 *   and NULL otherwise.
 */
void *
malloc(size_t size)
{
        size_t asize;      /* Adjusted block size */
        void *bp;

        /* Ignore spurious requests. */
        if (size == 0)
                return (NULL);

        /* Adjust block size to include overhead and alignment reqs. */
        if (size <= ALIGN)
                asize = DSIZE + ALIGN;
        else
                asize = ALIGN * ((size + DSIZE + (ALIGN - 1)) / ALIGN);

        /* Search the free list for a fit. */
        if ((bp = find_fit(asize)) != NULL) {
                place(bp, asize);
                return (bp);
        }
        return NULL;

}

/* 
 * Requires:
 *   "bp" is either the address of an allocated block or NULL.
 *
 * Effects:
 *   Free a block.
 */
void
free(void *bp)
{
        size_t size;

        /* Ignore spurious requests. */
        if (bp == NULL)
                return;

        /* Free and coalesce the block. */
        size = GET_SIZE(HDRP(bp));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        coalesce(bp);
}

/*
 * Requires:
 *   "ptr" is either the address of an allocated block or NULL.
 *
 * Effects:
 *   Reallocates the block "ptr" to a block with at least "size" bytes of
 *   payload, unless "size" is zero.  If "size" is zero, frees the block
 *   "ptr" and returns NULL.  If the block "ptr" is already a block with at
 *   least "size" bytes of payload, then "ptr" may optionally be returned.
 *   Otherwise, a new block is allocated and the contents of the old block
 *   "ptr" are copied to that new block.  Returns the address of this new
 *   block if the allocation was successful and NULL otherwise.
 */
void *
realloc(void *ptr, size_t size)
{
        size_t oldsize;
        void *newptr;

        /* If size == 0 then this is just free, and we return NULL. */
        if (size == 0) {
                free(ptr);
                return (NULL);
        }

        /* If oldptr is NULL, then this is just malloc. */
        if (ptr == NULL)
                return (malloc(size));

        newptr = malloc(size);

        /* If realloc() fails the original block is left untouched  */
        if (newptr == NULL)
                return (NULL);

        /* Copy the old data. */
        oldsize = GET_SIZE(HDRP(ptr));
        if (size < oldsize)
                oldsize = size;
        memcpy(newptr, ptr, oldsize);

        /* Free the old block. */
        free(ptr);

        return (newptr);
}

/*
 * The following routines are internal helper routines.
 */

/*
 * Requires:
 *   "bp" is the address of a newly freed block.
 *
 * Effects:
 *   Perform boundary tag coalescing.  Returns the address of the coalesced
 *   block.
 */
static void *
coalesce(void *bp)
{
        size_t size = GET_SIZE(HDRP(bp));
        bool prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
        bool next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));

        if (prev_alloc && next_alloc) {                 /* Case 1 */
                return (bp);
        } else if (prev_alloc && !next_alloc) {         /* Case 2 */
                size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
                PUT(HDRP(bp), PACK(size, 0));
                PUT(FTRP(bp), PACK(size, 0));
        } else if (!prev_alloc && next_alloc) {         /* Case 3 */
                size += GET_SIZE(HDRP(PREV_BLKP(bp)));
                PUT(FTRP(bp), PACK(size, 0));
                PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
                bp = PREV_BLKP(bp);
        } else {                                        /* Case 4 */
                size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
                        GET_SIZE(FTRP(NEXT_BLKP(bp)));
                PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
                PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
                bp = PREV_BLKP(bp);
        }
        return (bp);
}

/*
 * Requires:
 *   None.
 *
 * Effects:
 *   Find a fit for a block with "asize" bytes.  Returns that block's address
 *   or NULL if no suitable block was found. 
 */
static void *
find_fit(size_t asize)
{
        void *bp;

        /* Search for the first fit. */
        for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
                if (!GET_ALLOC(HDRP(bp)) && asize <= GET_SIZE(HDRP(bp)))
                        return (bp);
        }
        /* No fit was found. */
        return (NULL);
}

/* 
 * Requires:
 *   "bp" is the address of a free block that is at least "asize" bytes.
 *
 * Effects:
 *   Place a block of "asize" bytes at the start of the free block "bp" and
 *   split that block if the remainder would be at least the minimum block
 *   size. 
 */
static void
place(void *bp, size_t asize)
{
        size_t csize = GET_SIZE(HDRP(bp));

        if ((csize - asize) >= (DSIZE)) {
                PUT(HDRP(bp), PACK(asize, 1));
                PUT(FTRP(bp), PACK(asize, 1));
                bp = NEXT_BLKP(bp);
                PUT(HDRP(bp), PACK(csize - asize, 0));
                PUT(FTRP(bp), PACK(csize - asize, 0));
        } else {
                PUT(HDRP(bp), PACK(csize, 1));
                PUT(FTRP(bp), PACK(csize, 1));
        }
}






// TODO: Figure out why this doesn't work.

//typedef struct Node {
//    struct Node *prev;
//    struct Node *next;
//}__attribute__((packed)) node_t;
//typedef struct Header {
//    node_t heads[MAX_CLASS - MIN_CLASS + 1];
//}__attribute__((packed)) header_t;
//
//
///* Global variables: */
//static char *heap_listp; /* Pointer to first block */
//static header_t *head;
//
//// Pointer to epilogue block, useful for fast operations at end of heap.
//void *end;
//
///* Function prototypes for internal helper routines: */
////static void *extend_heap(size_t words);
//static void *find_fit(size_t asize);
//static void place(void *bp, size_t asize);
//
///* Function prototypes for heap consistency checker routines: */
////static void checkblock(void *bp);
////static void checkheap(bool verbose);
////static void printblock(void *bp);
//
//
//static void insert_node(node_t *new, node_t *list);
//static void remove_node(node_t *node);
//
///*
// * Requires:
// *   None.
// *
// * Effects:
// *   Initialize the memory manager.  Returns 0 if the memory manager was
// *   successfully initialized and -1 otherwise.
// */
//int
//heap_init(void)
//{
//
//        /* Create the initial empty heap. */
//        heap_listp = (char *)heap;
////        if ((heap_listp = mem_sbrk(3*WSIZE + sizeof(header_t))) == (void *)-1)
////                return (-1);
//
//        /* Prologue header and footer */
//        PUT(heap_listp + sizeof(header_t), PACK(DSIZE, 1));
//        PUT(heap_listp + sizeof(header_t) + WSIZE, PACK(DSIZE, 1));
//
//        head = (header_t *)(heap_listp);
//
//        // Initialize the free lists.
//        int i = 0;
//        for (; i <= MAX_CLASS - MIN_CLASS; i ++) {
//                head->heads[i].next = &head->heads[i];
//                head->heads[i].prev = &head->heads[i];
//        }
//
//        /* Epilogue header */
//        node_t *fblock = (node_t *)(heap_listp + 2 * WSIZE + sizeof(header_t));
//        unsigned int bsize = (HEAP_SIZE) - 3 * WSIZE + sizeof(header_t);
//        bsize = ALIGN * (bsize / ALIGN);
//        PUT(HDRP(fblock), bsize);
//        PUT(FTRP(fblock), bsize);
//        insert_node(fblock, &head->heads[MAX_CLASS - MIN_CLASS]);
//        
//        end = FTRP(fblock) + DSIZE;
//        PUT(HDRP(end), PACK(0, 1));
//
//        heap_listp += (WSIZE + sizeof(header_t));
//
//        return (0);
//}
//
///*
// * Requires:
// *   None.
// *
// * Effects:
// *   Allocate a block with at least "size" bytes of payload, unless
// *   "size" is zero. Returns the address of this block if the allocation
// *   was successful and NULL otherwise.
// */
//void *
//malloc(size_t size)
//{
//        size_t asize;      /* Adjusted block size */
//        void *bp;
//
//        /* Ignore spurious requests. */
//        if (size == 0)
//                return (NULL);
//
//        /* Adjust block size to include overhead and alignment reqs. */
//        if (size + DSIZE <= MIN_SIZE)
//                asize = MIN_SIZE;
//        else
//                asize = ALIGN * ((size + DSIZE + (ALIGN - 1)) / ALIGN);
//
//
//        /* Search the free list for a fit. */
//        if ((bp = find_fit(asize)) != NULL)
//                return (bp);
//
//        return NULL;
//}
//
///*
// * Requires:
// *   "bp" is either the address of an allocated block or NULL.
// *
// * Effects:
// *   Free a block.
// */
//void
//free(void *bp)
//{
//        unsigned int prev_alloc, next_alloc;
//        /* Ignore spurious requests. */
//        if (bp == NULL)
//                return;
//
//        size_t size = GET_SIZE(HDRP(bp));
//
////        coalesce:
////        prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
////        next_alloc = GET_ALLOC(NEXT_BLKP(bp) - WSIZE);
////
////        /*
////         * The following cases handle coalescing.
////         * Like find_fit below, putting the code here instead of a single
////         * function allowed for improved granularity and better performance,
////         * at the cost of longer and less modular code.
////         */
////
////        // Case 1:
////        // no coalescing necessary.
////        if (prev_alloc && next_alloc)
////                goto insert;
////
////        // Case 2:
////        // Coalesce with blocks after current block until there are
////        // no more free blocks after.
////        if (prev_alloc && !next_alloc) {
//////                while (!next_alloc) {
////                        size += GET_SIZE(NEXT_BLKP(bp) - WSIZE);
////
////                        remove_node((node_t *) (NEXT_BLKP(bp)));
////
////                        PUT(HDRP(bp), size);
////                        PUT(FTRP(bp), size);
////
////                        next_alloc = GET_ALLOC(NEXT_BLKP(bp) - WSIZE);
//////                }
////                goto insert;
////                // Case 3:
////                // Same as case 2, but this time for blocks preceding the
////                // current block.
////        } else if (!prev_alloc && next_alloc) {
//////                while (!prev_alloc) {
////                        size += GET_SIZE(PREV_BLKP(bp) - WSIZE);
////
////                        PUT(FTRP(bp), size);
////
////                        bp = PREV_BLKP(bp);
////                        remove_node(bp);
////                        PUT(HDRP(bp), size);
////
////                        prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
//////                }
////                goto insert;
////                // Case 4:
////                // If the blocks in both directions are free, coalesce in both
////                // directions until everything in joined. This case repeats
////                // everything since it often becomes one of the earlier cases on
////                // later iterations.
////        } else {
////                size += GET_SIZE(PREV_BLKP(bp) - WSIZE) +
////                        GET_SIZE(FTRP(NEXT_BLKP(bp)));
////
////                PUT(FTRP(NEXT_BLKP(bp)), size);
////                remove_node((node_t *)NEXT_BLKP(bp));
////
////                bp = PREV_BLKP(bp);
////                PUT(HDRP(bp), size);
////                remove_node(bp);
////
//////                goto coalesce;
////        }
////
////        // Put the block back into the appropriate free list.
////        insert:
//
//        if (size <= MAX_SIZE) {
//                PUT(HDRP(bp), size);
//                PUT(FTRP(bp), size);
//
//                insert_node(bp, &head->heads[
//                    GET_BIN(size + 1) - MIN_CLASS - 1]);
//
//        } else {
//                PUT(HDRP(bp), size);
//                PUT(FTRP(bp), size);
//
//                insert_node(bp, &head->heads[MAX_CLASS - MIN_CLASS]);
//        }
//
//        return;
//}
//
///*
// * Requires:
// *   "ptr" is either the address of an allocated block or NULL.
// *
// * Effects:
// *   Reallocates the block "ptr" to a block with at least "size" bytes of
// *   payload, unless "size" is zero.  If "size" is zero, frees the block
// *   "ptr" and returns NULL.  If the block "ptr" is already a block with at
// *   least "size" bytes of payload, then "ptr" may optionally be returned.
// *   Otherwise, a new block is allocated and the contents of the old block
// *   "ptr" are copied to that new block.  Returns the address of this new
// *   block if the allocation was successful and NULL otherwise.
// */
//void *
//realloc(void *ptr, size_t size)
//{
//        size_t oldsize, nsize;
//        void *newptr, *optr;
//
//        /* If size == 0 then this is just free, and we return NULL. */
//        if (size == 0) {
//                free(ptr);
//                return (NULL);
//        }
//
//        /* If oldptr is NULL, then this is just malloc. */
//        if (ptr == NULL)
//                return (malloc(size));
//
//        oldsize = GET_SIZE(HDRP(ptr));
//        size_t asize = ALIGN * ((size + DSIZE + ALIGN - 1) / ALIGN);
//
//        // If block was over-allocated, then no change is necessary.
//        if (oldsize >= asize)
//                return ptr;
//
//
//        // Attempt to expand into blocks following ptr.
//        nsize = oldsize;
//        while (!GET_ALLOC(NEXT_BLKP(ptr) - WSIZE)) {
//                nsize += GET_SIZE(NEXT_BLKP(ptr) - WSIZE);
//
//                remove_node((node_t *) NEXT_BLKP(ptr));
//
//                PUT(HDRP(ptr), PACK(nsize, 1));
//                PUT(FTRP(ptr), PACK(nsize, 1));
//
//                if (asize < nsize)
//                        return ptr;
//
//        }
//
//        // In order to use previous, there must be a continuous free block
//        // not including the current block, in order to preserve memory.
//        if (!GET_ALLOC(PREV_BLKP(ptr) - WSIZE)) {
//                optr = PREV_BLKP(ptr);
//                size_t msize = GET_SIZE((char *) optr - WSIZE);
//                while (!GET_ALLOC(PREV_BLKP(optr) - WSIZE)) {
//                        msize += GET_SIZE(PREV_BLKP(optr) - WSIZE);
//
//                        remove_node((node_t *) PREV_BLKP(optr));
//                        PUT(FTRP(optr), PACK(msize, 1));
//                        optr = PREV_BLKP(optr);
//                        PUT(HDRP(optr), PACK(msize, 1));
//
//                        if (msize >= asize) {
//                                if (size < oldsize)
//                                        oldsize = size;
//                                memcpy(optr, ptr, oldsize);
//                                free(ptr);
//                                return optr;
//                        }
//                }
//        }
//
//        // Attempt to place in an existing free block.
//        // This does not bother splitting any block it does fine, so it is
//        // potentially wasteful.
//        unsigned int i;
//        for (i = GET_BIN(asize) - MIN_CLASS; i < MAX_CLASS - MIN_CLASS; i++) {
//                if (head->heads[i].next != &head->heads[i]) {
//                        node_t *b = head->heads[i].next;
//
//                        remove_node(b);
//
//                        *HDRP(b) |= 1;
//                        *FTRP(b) |= 1;
//
//                        if (size < oldsize)
//                                oldsize = size;
//                        memcpy(b, ptr, oldsize);
//                        free(ptr);
//                        return b;
//                }
//        }
//
//        // If the current block is at the end of the heap, simply expand the
//        // heap to meet the new memory requirement.
////        if (NEXT_BLKP(ptr) == end) {
////                void *b;
////                if ((b = mem_sbrk(asize - nsize)) == (void *) -1)
////                        return (NULL);
////
////                b = (node_t *)PREV_BLKP(b);
////                PUT(HDRP(b), PACK(asize, 1));
////                PUT(FTRP(b), PACK(asize, 1));
////                PUT(NEXT_BLKP(b) - WSIZE, 1);
////
////                end = NEXT_BLKP(b);
////        }
//
//        // If all the above methods fail, then just allocate a new block.
//        newptr = malloc(size);
//
//        /* If realloc() fails the original block is left untouched  */
//        if (newptr == NULL)
//                return (NULL);
//
//        /* Copy the old data. */
//        if (size < oldsize)
//                oldsize = size;
//        memcpy(newptr, ptr, oldsize);
//
//        /* Free the old block. */
//        free(ptr);
//
//        return (newptr);
//}
//
///*
// * The following routines are internal helper routines.
// */
//
///*
// * Requires:
// *   None.
// *
// * Effects:
// *   Extend the heap with a free block and return that block's address.
// */
////static void *
////extend_heap(size_t words)
////{
////        size_t size;
////        void *bp;
////
////        size = words * ALIGN;
////        if ((bp = mem_sbrk(size)) == (void *)-1)
////                return (NULL);
////
////        /* Initialize free block header/footer and the epilogue header. */
////        PUT(HDRP(bp), size);         /* Free block header */
////        PUT(FTRP(bp), size);         /* Free block footer */
////        PUT(NEXT_BLKP(bp) - WSIZE, 1); /* New epilogue header */
////        end = NEXT_BLKP(bp);
////
////        return (bp);
////}
//
///*
// * Requires:
// *   None.
// *
// * Effects:
// *   Find a fit for a block with "asize" bytes. If no suitable fit is found,
// *   then the heap is extended to create a suitable block. After finding the
// *   block, the block is marked as allocated, and a pointer to the block is
// *   returned. If no suitable block was found, and no block could be created,
// *   returns NULL.
// *
// */
//static void *
//find_fit(size_t asize)
//{
//        node_t *bp;//, *b;
//        unsigned int i;//, prev_alloc;
////        size_t psize;
//
//        // Search the existing free blocks for any which are large enough.
//        // If one is found, place it and return the pointer.
//        for (i = GET_BIN(asize) - MIN_CLASS;
//             i <= MAX_CLASS - MIN_CLASS; i++) {
//                if (head->heads[i].next != &head->heads[i]) {
//                        bp = head->heads[i].next;
//                        remove_node(bp);
//                        place(bp, asize);
//                        return bp;
//                }
//        }
//
//        /*
//         * If no existing block is large enough, then we will need to get
//         * more memory.
//         * The following code is long and fairly repetitive, however the
//         * various cases tested here produce fairly significant performance
//         * improvements, so the complex code is the cost for improved
//         * performance.
//         * That said we have added comments to explain the cases.
//         */
//
//        
//        // No way to extend heap, this is just an error.
//        // If the last block in the heap is free, we don't need to
//        // extend the heap by as much.
////        prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(end)));
////        if (!prev_alloc) {
////                remove_node((node_t *)PREV_BLKP(end));
////                psize = GET_SIZE(HDRP(PREV_BLKP(end)));
////
////                /*
////                 * This is a case that shouldn't really happen, but can due to
////                 * the way blocks are replaced in the free lists.
////                 * i.e. A block with 56 bytes will be placed in the 32 byte
////                 * size class. Thus, it's possible that no heap extension is
////                 * necessary after all.
////                 */
////                if (psize >= asize) {
////                        b = (node_t *) PREV_BLKP(end);
////
////                        // Mark the block as allocated and recycle the excess
////                        // if large enough.
////                        if (psize - asize >= MIN_SIZE) {
////                                PUT(HDRP(b), PACK(asize, 1));
////                                PUT(FTRP(b), PACK(asize, 1));
////
////                                // Set headers for excess.
////                                PUT(NEXT_BLKP(b) - WSIZE, psize - asize);
////                                PUT(FTRP(NEXT_BLKP(b)), psize - asize);
////
////                                if (psize - asize < MAX_SIZE) {
////                                        i = GET_BIN(psize - asize);
////                                        insert_node(
////                                            (node_t *) NEXT_BLKP(b),
////                                            &head->heads[i - 1 - MIN_CLASS]);
////
////                                } else
////                                        insert_node(
////                                            (node_t *) NEXT_BLKP(b),
////                                            &head->heads[MAX_CLASS - MIN_CLASS]
////                                        );
////
////                        } else {        // Excess is too small to recycle.
////                                PUT(HDRP(b), PACK(psize, 1));
////                                PUT(FTRP(b), PACK(psize, 1));
////                        }
////                } else {
////                        /*
////                         * Extend the heap so the two blocks joined meet
////                         * the requested size. In this case, there will be no
////                         * excess, so no reason to check for excess.
////                         */
////                        if ((b = mem_sbrk(asize - psize)) == (void *) -1)
////                                return (NULL);
////
////                        b = (node_t *)PREV_BLKP(b);
////                        PUT(HDRP(b), PACK(asize, 1));
////                        PUT(FTRP(b), PACK(asize, 1));
////                        PUT(NEXT_BLKP(b) - WSIZE, 1);
////
////                        end = NEXT_BLKP(b);
////                }
////        } else {
////                /*
////                 * If the last block is allocated, then extend the heap to
////                 * make a new block. For blocks larger than CHUNKSIZE, simply
////                 * extend by the size of the block. For smaller blocks, it's
////                 * more efficient to allocate a page, then split off the
////                 * excess and store the resultant free blocks for later
////                 * requests.
////                 */
////                if (asize > CHUNKSIZE ) {
////                        if ((b = mem_sbrk(asize)) == (void *) -1)
////                                return (NULL);
////
////                        PUT(HDRP(b), PACK(asize, 1));
////                        PUT(FTRP(b), PACK(asize, 1));
////                        PUT(NEXT_BLKP(b) - WSIZE, 1);
////
////                        end = NEXT_BLKP(b);
////                } else {
////                        // Retrieve a full page.
////                        b = extend_heap(CHUNKSIZE / WSIZE);
////
////                        // Split the page and store the excess free block if
////                        // the block size is small enough.
////                        if (CHUNKSIZE - asize >= MIN_SIZE) {
////                                PUT(HDRP(b), PACK(asize, 1));
////                                PUT(FTRP(b), PACK(asize, 1));
////
////                                PUT(NEXT_BLKP(b) - WSIZE, CHUNKSIZE - asize);
////                                PUT(FTRP(NEXT_BLKP(b)), CHUNKSIZE - asize);
////
////                                if (CHUNKSIZE - asize < MAX_SIZE) {
////                                        i = GET_BIN(CHUNKSIZE - asize);
////                                        insert_node((node_t *)NEXT_BLKP(b),
////                                                    &head->heads[
////                                                        (i - 1 - MIN_CLASS)]);
////                                } else
////                                        insert_node(
////                                            (node_t *)NEXT_BLKP(b),
////                                            &head->heads[MAX_CLASS - MIN_CLASS]
////                                        );
////                        } else {
////                                PUT(HDRP(b), PACK(CHUNKSIZE, 1));
////                                PUT(FTRP(b), PACK(CHUNKSIZE, 1));
////                        }
////                }
////        }
////        return b;
//
//        return NULL;
//}
//
///*
// * Requires:
// *   "bp" is the address of a free block that is at least "asize" bytes.
// *
// * Effects:
// *   Place a block of "asize" bytes at the start of the free block "bp" and
// *   split that block if the remainder would be at least the minimum block
// *   size.
// */
//static void
//place(void *bp, size_t asize)
//{
//        // Subtracting here, then adding later if necessary, reduces the
//        // overall operation count slightly.
//        size_t csize = GET_SIZE(HDRP(bp)) - asize;
//
//        // Free block is large enough to split
//        if (csize >= MIN_SIZE) {
//
//                PUT(HDRP(bp), PACK(asize, 1));
//                PUT(FTRP(bp), PACK(asize, 1));
//
//                // We know the size, so no reason to get size.
//                bp = NEXT_BLKP(bp);
//
//                // Set the header and footer for the excess block.
//                PUT(HDRP(bp), csize);
//                PUT(FTRP(bp), csize);
//
//                // Put the excess back into the correct free list.
//                if (csize <= MAX_SIZE)
//                        insert_node(bp, &head->heads[
//                            GET_BIN(csize) - MIN_CLASS - 1]);
//                else
//                        insert_node(bp, &head->heads[MAX_CLASS - MIN_CLASS]);
//
//        } else {
//                // Set the headers, no further action required.
//                PUT(HDRP(bp), PACK(csize + asize, 1));
//                PUT((char *)bp + (csize + asize) - DSIZE,
//                    PACK(csize + asize, 1));
//        }
//}
//
///*
// * Requires:
// *   An node_t struct representing an existing doubly linked list, with
// *   appropriate (non-null, pointing to valid node_t structs) next and prev
// *   pointers, and a node_t struct to be inserted.
// *
// * Effects:
// *   Inserts the node_t "new" into the list containing "list".
// */
//static void
//insert_node(node_t *new, node_t *list)
//{
//        new->next = list->next;
//        new->prev = list;
//
//        list->next = new;
//        new->next->prev = new;
//}
//
///*
// * Requires:
// *   A node_t struct which is a member of a valid node list.
// *
// * Effects:
// *   Removes "node" from the list containing "node".
// */
//static void
//remove_node(node_t *node)
//{
//        node->next->prev = node->prev;
//        node->prev->next = node->next;
//}
//
//
///*
// * The remaining routines are heap consistency checker routines.
// */
//
///*
// * Requires:
// *   "bp" is the address of a block.
// *
// * Effects:
// *   Perform a minimal check on the block "bp".
// */
////static void
////checkblock(void *bp)
////{
////        if ((uintptr_t)bp % ALIGN)
////                printf("Error: %p is not doubleword aligned\n", bp);
////        if (GET(HDRP(bp)) != GET(FTRP(bp)))
////                printf("Error: header does not match footer\n");
////        if (GET_SIZE(HDRP(bp)) < MIN_SIZE && bp != heap_listp) {
////                printf("Error: block is too small.\n");
////        }
////}
////
/////*
//// * Requires:
//// *   None.
//// *
//// * Effects:
//// *   Perform a minimal check of the heap for consistency.
//// */
////void
////checkheap(bool verbose)
////{
////        void *bp;
////
////        if (verbose)
////                printf("Scanning heap...\n");
////
////        if (GET_SIZE(HDRP(heap_listp)) != DSIZE ||
////            !GET_ALLOC(HDRP(heap_listp)))
////                printf("Bad prologue header\n");
////        checkblock(heap_listp);
////
////        for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
////                if (verbose)
////                        printblock(bp);
////                checkblock(bp);
////        }
////
////        if (verbose)
////                printblock(bp);
////        if (GET_SIZE(HDRP(bp)) != 0 || !GET_ALLOC(HDRP(bp)))
////                printf("Bad epilogue header\n");
////
////        if (verbose) {
////                printf("Scanned heap.\n");
////                printf("Scanning free lists...\n");
////        }
////
////        // Check free lists:
////        // Every block in the free lists should be free
////        // and should be the appropriate size
////        unsigned int i;
////        for (i = 0; i <= MAX_CLASS - MIN_CLASS; i++) {
////                if (verbose)
////                        printf("Checking class %u.\n", i + MIN_CLASS);
////
////                bp = head->heads[i].next;
////                while (bp != &head->heads[i]) {
////                        if (verbose)
////                                printblock(bp);
////                        if (GET_ALLOC(HDRP(bp)))
////                                printf("Allocated block in free list.\n");
////                        if (GET_SIZE(HDRP(bp)) < (1u << (i + MIN_CLASS)))
////                                printf("Block in wrong size class.\n");
////                        if (((node_t *)bp)->next == NULL ||
////                            ((node_t *)bp)->prev == NULL)
////                                printf("Free block fields not set.\n");
////                        bp = ((node_t *)bp)->next;
////                }
////
////
////
////        }
////
////        if (verbose) {
////                printf("Scanned free lists.\n");
////                printf("Checking for isolated free blocks...\n");
////        }
////
////        // Check that all free blocks are stored in the correct free list.
////        bool found;
////        node_t *bp2;
////        for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
////                if (GET_ALLOC(HDRP(bp)))
////                        continue;
////
////                i = GET_BIN(GET_SIZE(HDRP(bp)));
////                if (i > MAX_CLASS)
////                        i = MAX_CLASS + 1;
////
////                found = false;
////                bp2 = head->heads[i - MIN_CLASS - 1].next;
////                while (bp2 != &head->heads[i - MIN_CLASS - 1]) {
////                        if (bp2 == bp) {
////                                found = true;
////                                break;
////                        }
////                        bp2 = bp2->next;
////                }
////
////                if (!found)
////                        printf("Error: Free block not in free list\n");
////
////        }
////
////        if (verbose)
////                printf("All checks complete\n");
////
////}
////
/////*
//// * Requires:
//// *   "bp" is the address of a block.
//// *
//// * Effects:
//// *   Print the block "bp".
//// */
////static void
////printblock(void *bp)
////{
////        size_t hsize, fsize;
////        bool halloc, falloc;
////
////        checkheap(false);
////        hsize = GET_SIZE(HDRP(bp));
////        halloc = GET_ALLOC(HDRP(bp));
////        fsize = GET_SIZE(FTRP(bp));
////        falloc = GET_ALLOC(FTRP(bp));
////
////        if (hsize == 0) {
////                printf("%p: end of heap\n", bp);
////                return;
////        }
////
////        printf("%p: header: [%zu:%c] footer: [%zu:%c]\n", bp,
////               hsize, (halloc ? 'a' : 'f'),
////               fsize, (falloc ? 'a' : 'f'));
////}

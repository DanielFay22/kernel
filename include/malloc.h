
#ifndef __MALLOC_H
#define __MALLOC_H

extern int	 heap_init(void);
extern void	*malloc(size_t size);
extern void	 free(void *ptr);
extern void	*realloc(void *ptr, size_t size);


extern void *heap;

#define HEAP_SIZE        (1 << 22)

#endif //KERNEL_MALLOC_H

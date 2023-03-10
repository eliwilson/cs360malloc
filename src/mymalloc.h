#ifndef _MYMALLOC_H
#define _MYMALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

void *my_malloc(size_t size);
void my_free(void *ptr);
void *free_list_begin();
void *free_list_next(void *node);
void coalesce_free_list();

#ifdef __cplusplus
}  /* extern C */
#endif

#endif

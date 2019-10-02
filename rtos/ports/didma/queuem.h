#include "didma.h"
#ifndef _QUEUEM_H_
#define _QUEUEM_H_

#define BLOCK_SIZE_CM (32 * sizeof(unsigned char *))

typedef struct cm_block {
  struct cm_block *next;
  unsigned char *block[BLOCK_SIZE_CM];
} cm_block_t;
 
static cm_block_t *cm_header, *cm_free;

static inline int _init_malloc_ (int n_blocks) {
  int n; 

  cm_header = (cm_block_t *) SYSTEM_MALLOC (n_blocks * sizeof(cm_block_t));

  if (cm_header == NULL) return -1;

  for (n = 0; n < n_blocks; n++)
    cm_header [n].next =((n + 1 == n_blocks)? NULL : &(cm_header [n + 1]));
  
  cm_free = &cm_header [0];
  return 0;
}

static inline void _destroy_malloc_ (void) {
  SYSTEM_FREE (cm_header);
}


static inline void *_malloc_ (void){
  cm_block_t *aux;
  if (cm_free == NULL) return NULL;

  aux = cm_free;
  cm_free = cm_free -> next;
  aux -> next = NULL;
  memset(aux, 0, BLOCK_SIZE_CM);
  return aux;
}

static inline void _free_ (void *ptr){
  cm_block_t *aux;
  aux = (cm_block_t *) ptr;
  aux -> next = cm_free;
  cm_free = aux;
}

#endif

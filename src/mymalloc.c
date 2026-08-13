// mymalloc - un asignador de memoria propio, estilo el de K&R pero con mmap.
//
// Entender malloc por dentro es entender el heap: bloques, cabeceras,
// fragmentacion, coalescing. Y es la base para el lado ofensivo (heap
// overflow, use-after-free, tcache poisoning): no puedes explotar lo que
// no entiendes.
//
// DISENO: una unica arena grande pedida con mmap. Cada bloque lleva una
// cabecera (tamano + libre?) y encadenamos bloques en una lista implicita
// (recorremos por tamanos). first-fit.
//
// HITO ACTUAL (ya funciona): my_malloc / my_free basicos con split de
// bloques. El test de abajo pide, libera y reusa memoria.
//
// LO QUE SIGUES TU (TODO):
//   1. coalescing: al liberar, fusionar con el bloque siguiente si esta libre
//   2. my_realloc
//   3. alineacion a 16 bytes (ABI x86-64)
//   4. lista explicita de libres (free list) para que malloc sea O(bloques
//      libres) en vez de O(todos los bloques)
//   5. (ofensivo) escribe un mini-writeup: si un usuario hace overflow de un
//      bloque, que le pasa a la cabecera del bloque de al lado?

#include "mymalloc.h"
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#define ARENA_SIZE (1 << 20)   // 1 MiB de arena

// Cabecera de bloque. size = bytes utiles (sin contar la cabecera).
typedef struct block {
    size_t size;
    int    free;
    struct block *next;   // siguiente bloque en memoria (lista implicita)
} block_t;

#define HDR sizeof(block_t)

static block_t *heap_start = NULL;
#define FOOT sizeof(size_t)

static void set_footer(block_t *b){
  *(size_t *)((char *)b +HDR +b->size) = b->size;
}
static void heap_init(void) {
    void *mem = mmap(NULL, ARENA_SIZE, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) {
        perror("mmap");
        return;
    }
    heap_start = (block_t *)mem;
    heap_start->size = ARENA_SIZE - HDR -FOOT;
    heap_start->free = 1;
    heap_start->next = NULL;
    set_footer(heap_start);
}

// Divide un bloque si sobra espacio para un bloque util adicional.
static void split(block_t *b, size_t want) {
    if (b->size >= want + HDR + FOOT + 8) {
        block_t *rest = (block_t *)((char *)b + HDR + want + FOOT);
        rest->size = b->size - want - HDR - FOOT;
        rest->free = 1;
        rest->next = b->next;
        b->size = want;
        b->next = rest;
        set_footer(b);
        set_footer(rest);
    }
}

void *my_malloc(size_t size) {
    if (size == 0) return NULL;
    if (!heap_start) heap_init();
    if (!heap_start) return NULL;

    // first-fit: primer bloque libre que quepa.
    for (block_t *b = heap_start; b; b = b->next) {
        if (b->free && b->size >= size) {
            split(b, size);
            b->free = 0;
            return (char *)b + HDR;      // devolvemos el puntero UTIL
        }
    }
    return NULL;   // arena agotada (TODO: crecer con otra mmap)
}

void my_free(void *ptr) {
    if (!ptr) return;
    block_t *b = (block_t *)((char *)ptr - HDR);
    b->free = 1;
    // TODO(1): coalescing. Si b->next existe y esta libre, fusionalos:
    //   b->size += HDR + b->next->size;  b->next = b->next->next;
    while (b->next != NULL && b->next->free){
    b->size+=FOOT + HDR + b->next->size;
    b->next = b->next->next;
    }
    while(b != heap_start ){
      size_t prev_size = *(size_t *)((char*)b-FOOT);
      block_t *prev = (block_t *)((char *)b-FOOT-prev_size-HDR);
      if (!prev->free) break;

      prev->size += FOOT+ HDR + b->size;
      prev->next = b->next;
      b = prev;
    }
    set_footer(b);
}
void *my_realloc(void *ptr, size_t size){
  if (ptr == NULL) return my_malloc(size);
  if (size == 0) {my_free(ptr);return NULL;}
  block_t *b = (block_t *)((char *)ptr - HDR);
  if (b->size >=size) return ptr;
  // TODO caso 4 , realocar y moverme , primero chequeo si hace falta igual

}

// Utilidad de depuracion: imprime el mapa del heap.
void my_heap_dump(void) {
    printf("--- heap ---\n");
    for (block_t *b = heap_start; b; b = b->next) {
        printf("  [%p] size=%-8zu %s\n",
               (void *)b, b->size, b->free ? "LIBRE" : "usado");
    }
}

// Prueba basica del asignador. Pide, escribe, libera y reusa.
#include "mymalloc.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    printf("== pedir 3 bloques ==\n");
    char *a = my_malloc(100);
    char *b = my_malloc(200);
    char *c = my_malloc(50);
    strcpy(a, "soy el bloque a");
    strcpy(b, "soy el bloque b");
    printf("a='%s'  b='%s'\n", a, b);
    my_heap_dump();

    printf("\n== liberar b ==\n");
    my_free(b);
    my_heap_dump();

    printf("\n== pedir 150 (deberia reusar el hueco de b) ==\n");
    char *d = my_malloc(150);
    printf("d=%p  (b era=%p)\n", (void *)d, (void *)b);
    my_heap_dump();

    printf("\n== liberar a y d (son vecinos) y pedir 260 ==\n");
    my_free(a);
    my_free(d);
    my_heap_dump();
    char *e = my_malloc(260);
    printf("e=%p  <- NULL o lejos = fragmentado\n", (void *)e);
    return 0;
}

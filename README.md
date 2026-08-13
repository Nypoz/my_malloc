# mymalloc — tu propio asignador de memoria

Entender `malloc` por dentro = entender el heap. Y es la base del lado ofensivo:
no se puede explotar (heap overflow, use-after-free, tcache poisoning) lo que no
se entiende.

## Diseño
Una arena de 1 MiB pedida con `mmap`. Cada bloque lleva cabecera
(`size`, `free`, `next`) y encadenamos con una **lista implícita**. Estrategia
first-fit + split de bloques.

## Correr el hito actual
```bash
make run
```
Verás cómo pide 3 bloques, libera el del medio y reusa el hueco.

## Tu ruta (en orden)
1. **Coalescing** — al liberar, fusionar con el vecino libre (ver TODO en `my_free`).
   Sin esto el heap se fragmenta y `d` no siempre cabe.
2. **`my_realloc`**.
3. **Alineación a 16 bytes** — la ABI de x86-64 lo exige para SSE.
4. **Free list explícita** — malloc pasa de O(todos) a O(libres).
5. **Writeup ofensivo** — dibuja qué le pasa a la cabecera del bloque vecino si
   escribes más allá del tamaño pedido. Ese es el mecanismo de un heap overflow.

## Portable (funciona en macOS también)
Este sí corre en Mac porque solo usa `mmap`, no `ptrace`. Bien para arrancar
antes de tener el Linux montado.

## Conceptos que sales sabiendo
Layout del heap · cabeceras de metadatos · fragmentación interna/externa ·
por qué un overflow de 1 byte puede corromper el allocator entero.

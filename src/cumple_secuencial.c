// Habilita el reloj monotono POSIX al compilar con -std=c11.
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <limits.h>
#include <time.h>

#include "cumple_functions.h"

int main(int argc, char *argv[]) {
    // 1. Leer K, N y una semilla opcional desde la terminal.
    unsigned long long k_leido, n, semilla = 2026;
    if ((argc != 3 && argc != 4) ||
        !leer_numero(argv[1], &k_leido) || k_leido < 1 || k_leido > INT_MAX ||
        !leer_numero(argv[2], &n) || n == 0 ||
        (argc == 4 && !leer_numero(argv[3], &semilla)) || semilla > UINT_MAX) {
        printf("Uso: %s K N [semilla]; valores invalidos\n", argv[0]);
        return 1;
    }
    int k = (int)k_leido;
    unsigned long long coincidencias = 0;

    // 2. Preparar la semilla y comenzar a medir el tiempo de pared.
    srand((unsigned int)semilla);
    struct timespec inicio, fin;
    if (clock_gettime(CLOCK_MONOTONIC, &inicio) != 0) return 1;

    // Para estos tamaños, el resultado se conoce sin simular.
    if (k > DIAS) {
        coincidencias = n;
    } else if (k > 1) {
        // 3. Ejecutar N simulaciones y contar coincidencias
        for (unsigned long long i = 0; i < n; i++) {
            if (hay_coincidencia(k)) {
                coincidencias++;
            }
        }
    }

    // 4. Terminar la medicion antes de imprimir.
    if (clock_gettime(CLOCK_MONOTONIC, &fin) != 0) return 1;
    double tiempo = (double)(fin.tv_sec - inicio.tv_sec)
                  + (double)(fin.tv_nsec - inicio.tv_nsec) / 1000000000.0;
    double probabilidad = (double)coincidencias / n;

    // 5. Mostrar los resultados.
    printf("Personas por grupo: %d\n", k);
    printf("Semilla base: %llu\n", semilla);
    printf("Simulaciones: %llu\n", n);
    printf("Coincidencias: %llu\n", coincidencias);
    printf("Probabilidad estimada: %.8f%%\n",
           probabilidad * 100);
    printf("Tiempo secuencial: %.9f segundos\n", tiempo);

    return 0;
}

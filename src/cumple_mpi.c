#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <mpi.h>
#include <errno.h>
#include <limits.h>

#include "cumple_functions.h"

int main(int argc, char *argv[]) {

    // 1. Inicializar MPI
    MPI_Init(&argc, &argv);

    int rank;
    int num_procesos;

    // 2. Identificar el proceso actual
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // 3. Obtener la cantidad total de procesos
    MPI_Comm_size(MPI_COMM_WORLD, &num_procesos);

    // 4. Leer argumentos
    if (argc != 3 && argc != 4) {
        if (rank == 0) {
            printf("Uso: ./cumple_mpi K N [semilla]\n");
        }
        MPI_Finalize();
        return 1;
    }

    unsigned long long k_leido, n, semilla = 2026;
    if (!leer_numero(argv[1], &k_leido) || k_leido < 1 || k_leido > INT_MAX ||
        !leer_numero(argv[2], &n) || n == 0 ||
        (argc == 4 && !leer_numero(argv[3], &semilla)) || semilla > UINT_MAX) {
        if (rank == 0) {
            printf("Valores de K, N o semilla invalidos\n");
        }
        MPI_Finalize();
        return 1;
    }

    int k = (int)k_leido;

    // 5. Repartir las simulaciones
    unsigned long long base = n / num_procesos;
    unsigned long long resto = n % num_procesos;

    unsigned long long simulaciones_locales = base;

    if ((unsigned long long)rank < resto) {
        simulaciones_locales++;
    }

    // 6. Cada proceso utiliza una semilla diferente
    srand((unsigned int)semilla + (unsigned int)rank);

    unsigned long long coincidencias_locales = 0;
    unsigned long long coincidencias_totales = 0;

    // 7. Sincronizar antes de medir
    MPI_Barrier(MPI_COMM_WORLD);
    double inicio = MPI_Wtime();

    // Para estos tamaños, el resultado se conoce sin simular.
    if (k > DIAS) {
        coincidencias_locales = simulaciones_locales;
    } else if (k > 1) {
        // 8. Ejecutar Monte Carlo localmente
        for (unsigned long long i = 0;
             i < simulaciones_locales; i++) {

            if (hay_coincidencia(k)) {
                coincidencias_locales++;
            }
        }
    }

    // 9. Sumar coincidencias en el proceso 0
    MPI_Reduce(
        &coincidencias_locales,
        &coincidencias_totales,
        1,
        MPI_UNSIGNED_LONG_LONG,
        MPI_SUM,
        0,
        MPI_COMM_WORLD
    );

    // 10. Medir el tiempo incluyendo la reduccion
    double tiempo_local = MPI_Wtime() - inicio;
    double tiempo_total = 0.0;

    MPI_Reduce(
        &tiempo_local,
        &tiempo_total,
        1,
        MPI_DOUBLE,
        MPI_MAX,
        0,
        MPI_COMM_WORLD
    );

    // 11. Solo el proceso 0 muestra resultados
    if (rank == 0) {
        double probabilidad =
            (double)coincidencias_totales / n;

        printf("\nPersonas por grupo: %d\n", k);
        printf("Procesos MPI: %d\n", num_procesos);
        printf("Semilla base: %llu\n", semilla);
        printf("Simulaciones totales: %llu\n", n);
        printf("Coincidencias: %llu\n",
               coincidencias_totales);
        printf("Probabilidad estimada: %.8f%%\n",
               probabilidad * 100);
        printf("Tiempo paralelo: %.9f segundos\n",
               tiempo_total);
    }

    // 12. Finalizar MPI
    MPI_Finalize();

    return 0;
}

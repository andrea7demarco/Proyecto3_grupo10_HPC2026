
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <mpi.h>

enum { DIAS = 365, PALABRAS_BITSET = (DIAS + 63) / 64 };

// Devuelve 1 si hay al menos un cumpleaños repetido.
int hay_coincidencia(int k) {
    if (k > DIAS) return 1;
    if (k <= 1) return 0;

    uint64_t dias[PALABRAS_BITSET] = {0};

    for (int i = 0; i < k; i++) {
        unsigned int cumple = (unsigned int)(rand() % DIAS);
        uint64_t mascara = UINT64_C(1) << (cumple & 63U);
        uint64_t *palabra = &dias[cumple >> 6U];

        if ((*palabra & mascara) != 0) {
            return 1;
        }

        *palabra |= mascara;
    }

    return 0;
}

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
    if (argc != 3) {
        if (rank == 0) {
            printf("Uso: ./cumple_mpi K N\n");
        }
        MPI_Finalize();
        return 1;
    }

    int k = atoi(argv[1]);
    unsigned long long n = strtoull(argv[2], NULL, 10);

    if (k < 1 || n == 0 || argv[2][0] == '-') {
        if (rank == 0) {
            printf("Valores de K o N invalidos\n");
        }
        MPI_Finalize();
        return 1;
    }

    // 5. Repartir las simulaciones
    unsigned long long base = n / num_procesos;
    unsigned long long resto = n % num_procesos;

    unsigned long long simulaciones_locales = base;

    if ((unsigned long long)rank < resto) {
        simulaciones_locales++;
    }

    // 6. Cada proceso utiliza una semilla diferente
    srand(2026u + (unsigned int)rank);

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
        printf("Simulaciones totales: %llu\n", n);
        printf("Coincidencias: %llu\n",
               coincidencias_totales);
        printf("Probabilidad estimada: %.4f%%\n",
               probabilidad * 100);
        printf("Tiempo paralelo: %.6f segundos\n",
               tiempo_total);
    }

    // 12. Finalizar MPI
    MPI_Finalize();

    return 0;
}

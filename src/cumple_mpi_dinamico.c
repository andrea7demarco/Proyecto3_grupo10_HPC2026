#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <mpi.h>

#include "cumple_functions.h"

enum { TAG_TRABAJO = 1, TAG_RESULTADO = 2 };

static unsigned long long simular_lote(int k, unsigned long long cantidad) {
    if (k > DIAS) return cantidad;
    if (k <= 1) return 0;

    unsigned long long coincidencias = 0;
    for (unsigned long long i = 0; i < cantidad; i++) {
        coincidencias += hay_coincidencia(k);
    }
    return coincidencias;
}

// Un lote de cero simulaciones indica al trabajador que debe terminar.
static unsigned long long asignar_lote(int trabajador,
                                      unsigned long long *pendientes,
                                      unsigned long long lote) {
    unsigned long long cantidad = *pendientes < lote ? *pendientes : lote;
    *pendientes -= cantidad;
    MPI_Send(&cantidad, 1, MPI_UNSIGNED_LONG_LONG, trabajador,
             TAG_TRABAJO, MPI_COMM_WORLD);
    return cantidad;
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int rank, num_procesos;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procesos);

    unsigned long long k_leido = 0, n = 0, semilla = 2026, lote = 0;
    if (argc < 3 || argc > 5 ||
        !leer_numero(argv[1], &k_leido) || k_leido < 1 || k_leido > INT_MAX ||
        !leer_numero(argv[2], &n) || n == 0 ||
        (argc >= 4 && !leer_numero(argv[3], &semilla)) ||
        (argc == 5 && (!leer_numero(argv[4], &lote) || lote == 0)) ||
        semilla > UINT_MAX - (unsigned int)(num_procesos - 1)) {
        if (rank == 0) {
            fprintf(stderr, "Uso: ./cumple_mpi_dinamico K N [semilla [lote]]\n"
                    "K, N y lote deben ser positivos; la semilla debe dejar "
                    "margen para sumar los ranks sin desbordar.\n");
        }
        MPI_Finalize();
        return 1;
    }
    int k = (int)k_leido;
    int trabajadores = num_procesos > 1 ? num_procesos - 1 : 1;

    // Heuristica inicial: unos 32 lotes por trabajador, de hasta 100000 grupos.
    if (lote == 0) {
        unsigned long long divisor = (unsigned long long)trabajadores * 32;
        lote = n / divisor + (n % divisor != 0);
        if (lote > 100000) lote = 100000;
    }
    srand((unsigned int)semilla + (unsigned int)rank);

    unsigned long long simulaciones_totales = 0, coincidencias_totales = 0;
    MPI_Barrier(MPI_COMM_WORLD);
    double inicio = MPI_Wtime();

    if (num_procesos == 1) {
        // MPI1 no necesita mensajes a si mismo y usa la semilla del secuencial.
        simulaciones_totales = n;
        coincidencias_totales = simular_lote(k, n);
    } else if (rank == 0) {
        unsigned long long pendientes = n;
        int activos = 0;
        for (int trabajador = 1; trabajador < num_procesos; trabajador++) {
            if (asignar_lote(trabajador, &pendientes, lote) != 0) activos++;
        }

        while (activos > 0) {
            unsigned long long resultado[2];
            MPI_Status estado;
            // Atiende a quien termine primero, sin esperar por un rank fijo.
            MPI_Recv(resultado, 2, MPI_UNSIGNED_LONG_LONG, MPI_ANY_SOURCE,
                     TAG_RESULTADO, MPI_COMM_WORLD, &estado);
            simulaciones_totales += resultado[0];
            coincidencias_totales += resultado[1];
            if (asignar_lote(estado.MPI_SOURCE, &pendientes, lote) == 0) activos--;
        }
    } else {
        while (1) {
            unsigned long long cantidad;
            MPI_Recv(&cantidad, 1, MPI_UNSIGNED_LONG_LONG, 0,
                     TAG_TRABAJO, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (cantidad == 0) break;

            unsigned long long resultado[2] = {cantidad, simular_lote(k, cantidad)};
            MPI_Send(resultado, 2, MPI_UNSIGNED_LONG_LONG, 0,
                     TAG_RESULTADO, MPI_COMM_WORLD);
        }
    }

    // Incluye reparto, calculo, resultados y avisos de fin. Excluye esta reduccion.
    double tiempo_local = MPI_Wtime() - inicio, tiempo_total = 0.0;
    MPI_Reduce(&tiempo_local, &tiempo_total, 1, MPI_DOUBLE, MPI_MAX,
               0, MPI_COMM_WORLD);

    if (rank == 0) {
        if (simulaciones_totales != n || coincidencias_totales > n) {
            fprintf(stderr, "El conteo final de simulaciones es invalido\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }
        printf("\nModelo MPI: maestro-trabajador dinamico\n");
        printf("Personas por grupo: %d\n", k);
        printf("Procesos MPI: %d\n", num_procesos);
        printf("Trabajadores de calculo: %d\n", trabajadores);
        printf("Tamano de lote: %llu\n", lote);
        printf("Semilla base: %llu\n", semilla);
        printf("Simulaciones totales: %llu\n", n);
        printf("Simulaciones procesadas: %llu\n", simulaciones_totales);
        printf("Coincidencias: %llu\n", coincidencias_totales);
        printf("Probabilidad estimada: %.8f%%\n",
               (double)coincidencias_totales / n * 100);
        printf("Tiempo paralelo: %.9f segundos\n", tiempo_total);
    }
    MPI_Finalize();
    return 0;
}

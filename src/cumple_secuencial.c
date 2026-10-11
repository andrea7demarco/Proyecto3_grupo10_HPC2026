
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

enum { DIAS = 365, PALABRAS_BITSET = (DIAS + 63) / 64 };
/*Función*/
//Representa una simulacion completa de Monte Carlo
//Recibe como parámetro k : cantidad de personas forman parte del grupo.

// Retorna 1: si hay coincidencia de cumpleaños
// Retorna 0: si no hay coincidencia de cumpleaños
int hay_coincidencia(int k) {

    if (k > DIAS) return 1; // Hay más personas que fechas disponibles.
    if (k <= 1) return 0;

    uint64_t dias[PALABRAS_BITSET] = {0};

    //genera los cumples de las k personas
    for (int i = 0; i < k; i++) { 
        unsigned int cumple = (unsigned int)(rand() % DIAS);
        uint64_t mascara = UINT64_C(1) << (cumple & 63U);
        uint64_t *palabra = &dias[cumple >> 6U];

        if ((*palabra & mascara) != 0) {
            return 1;
        }

        // Marcamos el dia como utilizado
        *palabra |= mascara;
    }

    return 0;
}

int main() {
    int k;
    //para no tener desbordamiento
    unsigned long long n;
    unsigned long long coincidencias = 0;

    printf("Cantidad de personas K: ");
    if (scanf("%d", &k) != 1 || k < 1)
        return 1;

    printf("Cantidad de simulaciones N: ");
    if (scanf("%llu", &n) != 1 || n == 0)
        return 1;

    // Semilla fija para repetir el experimento
    srand(2026);

    // Para estos tamaños, el resultado se conoce sin simular.
    if (k > DIAS) {
        coincidencias = n;
    } else if (k > 1) {
        // Ejecutamos N simulaciones
        for (unsigned long long i = 0; i < n; i++) {
            if (hay_coincidencia(k)) {
                coincidencias++;
            }
        }
    }

    double probabilidad = (double)coincidencias / n;

    printf("\nSimulaciones: %llu\n", n);
    printf("Coincidencias: %llu\n", coincidencias);
    printf("Probabilidad estimada: %.4f%%\n",
           probabilidad * 100);

    return 0;
}

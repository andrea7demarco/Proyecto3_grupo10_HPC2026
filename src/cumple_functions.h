#ifndef CUMPLE_FUNCTIONS_H
#define CUMPLE_FUNCTIONS_H

#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

enum { DIAS = 365, PALABRAS_BITSET = (DIAS + 63) / 64 };
/*Función*/
//Representa una simulacion completa de Monte Carlo
//Recibe como parámetro k : cantidad de personas forman parte del grupo.

// Retorna 1: si hay coincidencia de cumpleaños
// Retorna 0: si no hay coincidencia de cumpleaños
static int hay_coincidencia(int k) {

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

// Convierte un argumento a entero sin signo y rechaza texto invalido.
static int leer_numero(const char *texto, unsigned long long *numero) {
    if (*texto == '\0') return 0;
    for (const char *p = texto; *p; p++) {
        if (*p < '0' || *p > '9') return 0;
    }
    char *fin;
    errno = 0;
    *numero = strtoull(texto, &fin, 10);
    return errno != ERANGE && *fin == '\0';
}

#endif

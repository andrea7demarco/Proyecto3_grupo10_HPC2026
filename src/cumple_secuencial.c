
#include <stdio.h>
#include <stdlib.h>
/*Función*/
//Representa una simulacion completa de Monte Carlo
//Recibe como parámetro k : cantidad de personas forman parte del grupo.

// Retorna 1: si hay coincidencia de cumpleaños
// Retorna 0: si no hay coincidencia de cumpleaños
int hay_coincidencia(int k) {

    if (k > 365) return 1; //si hay mas de 366 personas , al menos dos comparten cumple

    int dias[365] = {0};

    //genera los cumples de las k personas
    for (int i = 0; i < k; i++) { 
        int cumple = rand() % 365;

        if (dias[cumple] == 1) {
            return 1;
        }

        // Marcamos el dia como utilizado
        dias[cumple] = 1;
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

    // Ejecutamos N simulaciones
    for (unsigned long long i = 0; i < n; i++) {
        if (hay_coincidencia(k)) {
            coincidencias++;
        }
    }

    double probabilidad = (double)coincidencias / n;

    printf("\nSimulaciones: %llu\n", n);
    printf("Coincidencias: %llu\n", coincidencias);
    printf("Probabilidad estimada: %.4f%%\n",
           probabilidad * 100);

    return 0;
}

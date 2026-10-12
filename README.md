# Proyecto 3: Simulación de Probabilidad Colectiva

## Datos del grupo

- **Grupo:** 10
- **Proyecto:** 3 — Paradoja del cumpleaños
- **Turno:** 11:00 a 13:00 hs

## Integrantes

- Díaz Romero Guillermo Miguel
- De Marco Andrea Sofia
- Díaz Claribel Mariela

## Presentación

La paradoja del cumpleaños plantea una pregunta: ¿cuál es la probabilidad de que al menos dos personas de un grupo compartan la misma fecha de cumpleaños? Aunque el resultado parece poco intuitivo, en un grupo de solo 23 personas esta probabilidad es aproximadamente del 50,73 %, suponiendo 365 días igualmente probables y cumpleaños independientes.

Este proyecto estima esa probabilidad mediante simulaciones de Monte Carlo y compara los resultados con el valor teórico. Se implementan versiones secuenciales y paralelas para analizar los tiempos de ejecución, la aceleración (speedup) y la eficiencia al variar la cantidad de simulaciones y de procesos.

## Tecnologías usadas

- **C (C11):** implementación de las simulaciones de Monte Carlo y detección de coincidencias.
- **MPI (Message Passing Interface):** ejecución paralela con distribución estática del trabajo y con un modelo dinámico de maestro y trabajadores.
- **GCC y mpicc:** compilación de las versiones secuencial y paralelas.
- **Python 3:** automatización de pruebas de rendimiento, cálculo de medianas, speedup y eficiencia, y exportación de resultados en CSV.
- **Google Docs y Google Sheets:** elaboración del informe y organización de los resultados de los experimentos.

## Documento y Excel en Drive

- [Documento del proyecto — PROYECTO HPC](https://docs.google.com/document/d/1e2LdCuayctUgVok3xm-Jij-mvn8vgAp1VcCNUXsbnXo/edit?usp=drivesdk)
- [Planilla de resultados (Excel) — Simulaciones HPC](https://docs.google.com/spreadsheets/d/1T3La9IpofffZZhW85wYCKfwxrHvC706NZwek8Ci5hoo/edit?usp=drivesdk)

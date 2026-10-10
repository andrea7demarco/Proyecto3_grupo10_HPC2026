# Simulaciones_Colectivas_Paradoja_De_Cuplea-os_HPC
Proyecto 3: Simulación de Probabilidad Colectiva (Paradoja del Cumpleaños) Las paradojas probabilísticas suelen ser contraintuitivas. Sin embargo, mediante simulación masiva de eventos (método empírico), es posible verificar la frecuencia relativa de los resultados y validarlos contra la teoría. 


PROYECTO HPC 
___________________________________________________________________________ Proyecto 3: Simulación de Probabilidad Colectiva (Paradoja del Cumpleaños) Las paradojas probabilísticas suelen ser contraintuitivas. Sin embargo, mediante simulación masiva de eventos (método empírico), es posible verificar la frecuencia relativa de los resultados y validarlos contra la teoría. Investigar la siguiente “paradoja” y contrastar la teoría con simulaciones realizadas: Paradoja del Cumpleaños: Determinación de la probabilidad de que al menos dos personas compartan la misma fecha de cumpleaños en un grupo de tamaño 𝐾 

Consignas: 
● Investigar sobre el problema de aplicación, sus objetivos, limitaciones y estrategias de resolución. 
● Realizar una implementación paralela del problema de aplicación asignado teniendo en cuenta la manera más adecuada de paralelizar. Seleccionar o generar los datos necesarios para hacer diferentes tipos de pruebas. 
● Correr la implementación en diferentes sistemas (PC, servidor, etc.)
● Calcular speed up utilizando la mediana de múltiples ejecuciones. 
● Realizar múltiples simulaciones cambiando las variables utilizadas. 
● Concluir sobre los resultados obtenidos 
● ¿Qué mejoras recomendaría hacer a su programa para futuros casos? 

Problema de aplicación
1.1 Introducción 
El problema del cumpleaños, también llamado paradoja del cumpleaños, establece que de un conjunto de 23 personas, hay una probabilidad del 50.7 % de que al menos dos de ellas cumplan años el mismo día. Para 57 o más personas la probabilidad es mayor del 99.166 %. En un sentido estricto esto no sería una paradoja porque no es una contradicción lógica, sin embargo, es una verdad matemática que contradice la intuición común, ya que uno creería que la probabilidad para un poco más del doble de 23 es mucho más baja que el 99.166 % y que hacen falta muchas más personas para alcanzarla.

La clave está en que no buscamos que una persona cumpla anios en una fecha específica, sino que cualquier par de personas del grupo pueda compartir cumpleaños.

Ej: en un grupo de 23 persona existen 253 pares posibles, calculados mediante:

cantidad de pares = K x (K-1) / 2

Este número de pares crece de manera cuadrática a medida que aumenta la cantidad de personas, lo que ayuda a entender por qué la probabilidad de coincidencia aumenta tan rápido.

Para calcular la probabilidad de que al menos dos personas compartan el cumpleaños, primero calculamos la probabilidad del evento contrario, que todas las personas tengan cumpleaños diferentes.

Suponemos que existen 365 fechas igualmente probables y que los cumpleaños son diferentes:

-La primera persona puede cumplir anios cualquiera de los 365 días.
-La segunda debe cumplir anios en alguno de los 364 días restantes.
-La tercera dispone de 363 días para no coincidir con las anteriores.
(consideraciones : desechando años bisiestos, gemelos y asumiendo que existen 365 cumpleaños que tienen la misma probabilidad)



Resultando en la fórmula analítica : 
(Sin coincidencias)

P (al menos una coincidencia) = 1 - P(sin coincidencia)

Para k = 23, se obtiene aproximadamente un 50,73%

Este resultado constituye parte de la referencia teórica con la que se compararán las simulaciones realizadas.

1.2 Objetivos

Objetivos generales

Estimar realizando simulaciones computacionales la probabilidad de que al menos dos personas compartan cumpleaños dentro de un grupo de k personas, contrastando los resultados con la probabilidad teórica y evaluando el rendimiento de una implementación paralela mediante MPI.

Objetivos específicos

Implementar el método Monte Carlo para estimar la probabilidad de coincidencia de cumpleaños.
Desarrollar una versión secuencial y una versión paralela utilizando MPI.
Comparar las probabilidades obtenidas mediante simulación con los valores teóricos.
Analizar como varía la precisión de la estimación al aumentar la cantidad de simulaciones.
Evaluar cómo cambia la probabilidad al modificar el tamaño K del grupo.
Calcular el speedup 
Evaluar el rendimiento en diferentes sistemas de cómputo y con distintas cantidades de procesos.


1.3 Limitaciones 
El problema presenta algunas limitaciones relacionadas tanto al modelo probabilístico como con su implementación computacional.

El modelo supone 365 fechas con la misma probabilidad cada una. Los nacimientos reales pueden presentar estacionalidad y además se omite el 29 de febrero para simplificar los cálculos. Además suponemos independencia de los cumpleaños, algo que podría no cumplirse en la realidad ya que existe una probabilidad de que hayan hermanos gemelos y mellizos.

Al utilizar números pseudoaleatorios, los resultados de las simulaciones se pueden ver afectados por el azar y por lo tanto puede diferir de la probabilidad teórica, esa diferencia se la conoce como error de estimación. Al aumentar el número de las simulaciones normalmente se obtienen resultados más precisos porque la frecuencia de coincidencias tiende a acercarse a la probabilidad real (((( ley de los grandes nums?

Al aumentar la cantidad de simulaciones mejoraríamos la precisión esperada pero también aumenta el trabajo computacional, por eso utilizaremos MPI para distribuir las simulaciones entre varios procesos e intentar reducir el tiempo de ejecución



1.4 Estrategias de resolución

Se utilizarán dos enfoques: el cálculo analítico de la probabilidad y la simulación mediante el método de Monte Carlo.

Resolución analítica: utilizamos la fórmula matemática de la paradoja correspondiente a cada tamaño del grupo K, estos valores nos servirán como referencia para evaluar la precisión de los resultados obtenidos mediante la simulación.

Resolución mediante Monte Carlo: nos permitrá estimar las probabilidades mediante la repetición de experimentos aleatorios. En este problema cada experimento consiste en simular un grupo K de personas, asignando a cada una un cumpleaños generado de forma pseudoaleatoria entre 1 y 365. Luego se verifica si al menos dos personas comparten la misma fecha. El procedimiento a seguir será el siguiente:

Definir el tamaño K del grupo y la cantidad total N de las simulaciones.
Generar K fechas de cumpleaños pseudoaleatorias ??? q semilla usamos?
Comprobar si existe al menos una fecha repetida.
Registrar si la simulación presentó una coincidencia.
Repetir el experimento N veces.
Calcular la proporcón de simulaciones en las que ocurrió al menos una coincidencia.

La probabilidad estimada se calcula como:
P estimada = Nro de simulaciones con coincidencia / Nro total de simulaciones

Cuanto mayor sea el nro de simulaciones, se espera que la esitimación se aproxime a la probabilidad teórica.

       Inicialmente desarrollaremos una implementación secuencial en la que un único                 proceso ejecutará todas las simulaciones lo cual nos permitirá verificar el funcionamiento del algoritmo y obtener un tiempo de ejecución de referencia para evaluar luego las mejoras mediante la paralelización.
Debido a que cada simulación de Monte Carlo es independiente de las otras, el problema permite distribuir el trabajo en múltiples procesos. Utilizaremos MPI (Message Passing Interfence) para paralelizar la ejecución distribuyendo la cantidad total de simulaciones entre los procesos disponibles donde cada uno generará sus propias muestras pseudoaleatorias, realizará las simulaciones asignadas y contará cuántas presentan coincidencias. Además para evitar que los procesos repitan exactamente los mismo experimentos, utilizaremos secuencias pseudoaleatorias distintas. Y finalmente los resultados los combinaremos mediante una operación de reducción (MPI_reduce) para obtener el número total de simulaciones con coincidencias. Ya con estos resultados podremos calcular la probabilidad global estimada.

Validación

Realizaremos experimentos modificando el tamaño K del grupo, la cantidad total de simulaciones y el número de procesos MPI. Los resultados se compararán con las probabilidades teóricas del comienzo para analizar el error de estimación y se medirán los tiempos de ejecución de las implementaciones tanto secuencial como la paralela.

Para evaluar la aceleración obtenida:

Speedup = Tiempo secuencial / Tiempo paralelo

… no se q mas falta para concluir…. agregar dsps buscar
completar a medida q haga la practica


2. Implementación del algoritmo 

# Control y enrutamiento de una red

## Resumen 
En este informe se va a analizar y comparar un algoritmo para las redes de anillo.
El algoritmo que implementamos lo llamamos **"Falso Dijkstra"** y se preguntaran por que el nombre, bueno, es porque en realidad se representa un grafo donde todos caminos son de peso 1. En lugar de utilizar el algoritmo de Dijkstra para calcular las distancias mínimas entre dos nodos, se utiliza el algoritmo de BFS (Breadth-First Search) debido a las características particulares del grafo.

## Introducción
Se introdujo un red de forma de anillo donde cada nodo esta conectado a otros dos.
Cada nodo cuanta con una capa de aplicación y una capa de transporte estas capas trabajan juntas para procesar y dirigir los paquetes dentro del anillo.

La capa de aplicación se encarga de :

- Generar paquetes
- Enviar paquetes

Y la capa de transporte se encarga de:

- Mandar paquetes que viene de la  capa de aplicación
- Recibir paquetes de los vecinos y enviarlo al otro

![Intro](Img_Infomre/Intro.png)

### Metodología de trabajo
Para analizar el comportamiento de la red con el algoritmo y sin el mismo, al igual que en el lab 3 utilizaremos **simulación discreta**.

En la **simulación discreta**, en lugar de considerar el tiempo de manera continua, se divide en pasos o intervalos discretos. Cada uno de estos intervalos representa un punto en el tiempo en el que se realiza una actualización en el estado del sistema que se está simulando.

Esta técnica de modelado es especialmente útil cuando se trabaja con sistemas dinámicos complejos, ya que permite simular su comportamiento de manera más manejable y comprensible. Al dividir el tiempo en pasos discretos, se simplifica el proceso de cálculo y se facilita el seguimiento y análisis de los cambios en el estado del sistema en diferentes puntos en el tiempo.

Las simulaciones se llevan a cabo en una computadora utilizando **Omnet++**, un **entorno de simulación de eventos discretos**. En este entorno, se crea un modelo que representa el comportamiento de una red simplificada. El objetivo es analizar cómo funciona la red en diferentes escenarios, tanto con como sin el algoritmo.

**Omnet++** es una herramienta poderosa que permite realizar estas simulaciones y analizar los resultados obtenidos. Proporciona un entorno flexible y configurable para modelar diferentes aspectos de la red y realizar experimentos virtuales. Esto ayuda a comprender cómo se comporta la red en diferentes situaciones y a evaluar la eficacia de los algoritmos en la optimización del rendimiento de la red.

### Análisis de los experimentos
Evaluaremos el rendimiento mediante dos casos de prueba.

- **Caso 1:** Se deberá correr el modelo con las fuentes de tráfico configuradas (node[0] y
  node[2] transmitiendo datos a node[5]) y estudiar las métricas tomada
- **Caso 2:** Asuma ahora que todos los nodos (0,1,2,3,4,6,7,8) generan tráfico hacia el
  node[5] con *packetByteSize* e *interArrivalTime* idénticos entre todos los nodos.

El funcionamiento de la red anillo dada es bastante siemple.

En el algoritmo inicial, todos los paquetes se envían en una dirección específica, en este caso, hacia el vecino que está a la izquierda del nodo. No se considera ningún criterio para determinar la ruta más eficiente; simplemente se envían todos los paquetes de la misma manera, sin importar la distancia entre el enrutador de origen al enrutador del destino. No se toman decisiones basadas en la eficiencia o la minimización de la ruta, simplemente se siguen enviando los paquetes de esa manera predeterminada.



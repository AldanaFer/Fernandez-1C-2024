# Proyecto_Integrador

 A partir de una señal de EMG tomada de los músculos encargados de cerrar y
 abrir la mano, esta señal pasa primero por una Placa de Bipotenciales. Esta
 placa amplifica y filtra la señal, preparándola para su procesamiento. A
 continuación, la señal analógica resultante se envía a la placa EDU-ESP32.
 La EDU-ESP32 digitaliza y procesa la señal EMG, analizando los patrones de
 contracción muscular. Si se detecta una contracción significativa en la
 señal, se envía una orden a un servo motor. Este servo motor se encarga de
 simular el movimiento de apertura y cierre de la mano, replicando así la
 acción muscular detectada inicialmente. La grafica del EMG se puede observar 
 en un celular que se conecta por bluetooth y se observa la señal entregada 
 por la placa de biopotenciales y la señal luego de aplicarle los filtros. 

 ### Hardware requerido

* ESP-EDU
* Placa de Biopotenciales
* Servo sg90
* Celular (bluetooth)

### Especificaciones del Programa

La lectura de la señal de EMG se realiza cada 5 milisegundos que se guarda 
en un vector de tamaño 4, una vez que se realicen 4 lecturas se aplican los
filtros se guardan en otro vector de tamaño 4, estos valores se rectifican
y se guardan en un vector con un tamaño de 512 valores. Cada 2,56 segundos
se recorre este vector entero y se guarda el maximo valor; si este es 
mayor al umbral se mueve el servo. Continuamente se esta mostrando la señal 
del EMG tanto sin aplicar como luego de aplicar los filtros en dos graficas 
separadas en la app del celular que se conecta por bluetooth.

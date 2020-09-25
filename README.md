# TimerInsol

Cronómetro para insoladora.

# Material:
   Arduino Pro Mini. (Cualquier Arduino configurando los pines).
   
   3 botones
   
   1 relé
   
   TM1637
   
   (Fuente de alimentación externa)
   

 # Configuración para Arduino Pro / Pro Mini
 Pines:
 
 boton set: 4
 
 botón mode: 5
 
 botón: start/pausa: 6
 
 Salida al relé: 8
 
 Configuracion diplay 7 segmentos TM1637:
 
  CLK 2
  
  DIO 3
  
  
 # Funcionamiento general:
  Al iniciar el programa carga tiempo establecido en memoria EEPROM.
  Es posible modificar el tiempo con una pulsación large de SET ( más de un segundo). Se pasará a metodo edición. 
  El digito a editar parpadeará. Las decenas de minuto tienen un valor entre 0 y 2. Para modificar el valor del digito
  elegido se utiliza el botón MODE. Para alternar entre dígitos hay que pulsar SET. Para abandonar este modo hay que hacer
  una pulsación larga de SET.
  Fuera del modo edición, el anterior el triple click de SET guardará el valor del display en la EEPROM.
  Para iniciar el temporizador se ha de pulsar botón START/PAUSA. Una vez iniciado este botón pausa el cronómetro.
  El botón MODE reinicia el contador. Esta funcionalidad se da aunque este en pausa.

# Librerias
  Button2: https://github.com/LennartHennigs/Button2
  TM1637: https://github.com/Seeed-Studio/Grove_4Digital_Display
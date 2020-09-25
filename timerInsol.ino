/*
 * Cronómetro para insoladora.
 * 
 * Material:
 *    Arduino Pro Mini. (Cualquier Arduino configurando los pines).
 *    3 botones
 *    1 relé
 *    TM1637
 *    (Fuente de alimentación externa)
 * 
 * Configuración para Arduino Pro / Pro Mini
 * Pines:
 * boton set: 4
 * botón mode: 5
 * botón: start/pausa: 6
 * Salida al relé: 8
 * Configuracion diplay 7 segmentos TM1637:
 *  CLK 2
 *  DIO 3
 *  
 * Funcionamiento general:
 *  Al iniciar el programa carga tiempo establecido en memoria EEPROM.
 *  Es posible modificar el tiempo con una pulsación large de SET ( más de un segundo). Se pasará a metodo edición. 
 *  El digito a editar parpadeará. Las decenas de minuto tienen un valor entre 0 y 2. Para modificar el valor del digito
 *  elegido se utiliza el botón MODE. Para alternar entre dígitos hay que pulsar SET. Para abandonar este modo hay que hacer
 *  una pulsación larga de SET.
 *  Fuera del modo edición, el anterior el triple click de SET guardará el valor del display en la EEPROM.
 *  Para iniciar el temporizador se ha de pulsar botón START/PAUSA. Una vez iniciado este botón pausa el cronómetro.
 *  El botón MODE reinicia el contador. Esta funcionalidad se da aunque este en pausa.
 *  
 */
#include <TimerOne.h>
#include "TM1637.h"
#include "Button2.h"
#include <EEPROM.h>

#define ON 1
#define OFF 0

#define BUTTON_SET_PIN  4
#define BUTTON_MODE_PIN  5
#define BUTTON_START_PIN  6
#define RELE 8

#define CLK 2 //pins definitions for TM1637 and can be changed to other ports
#define DIO 3

#define ZERO 0
#define UNO 1
#define DOS 2
#define TRES 3
#define CINCUENTA 50
#define VEINTE 20
#define DIEZ 10
#define NUEVE 9


#define ADDRESS ZERO

int8_t TimeDisp[] = {0x00,0x00,0x00,0x00};
unsigned char ClockPoint = UNO;
unsigned char Update;
unsigned char halfsecond = ZERO;
unsigned char second;
unsigned char minute;
bool stop = false;

bool started = false;
bool setting = false;
int settingIndex = ZERO;

TM1637 tm1637(CLK,DIO);

Button2 setButton = Button2(BUTTON_SET_PIN);
Button2 modeButton = Button2(BUTTON_MODE_PIN);
Button2 startButton = Button2(BUTTON_START_PIN);

void setup()
{
  Serial.begin(9600);

  tm1637.set();
  tm1637.init();
  Timer1.initialize(500000);//timing for 500ms
  Timer1.attachInterrupt(TimingISR);//declare the interrupt serve routine:TimingISR

  // Declaración de los tipos de pusado en los botones.
  setButton.setClickHandler(handlerBtn);
  setButton.setLongClickHandler(handlerBtn);
  setButton.setTripleClickHandler(handlerBtn);
  modeButton.setClickHandler(handlerBtn);
  startButton.setClickHandler(handlerBtn);

  // Pin rele
  pinMode(RELE, OUTPUT);

  // rele a bajo
  digitalWrite(RELE, HIGH);

  // Set valor inicio con valor de la EEPROM.
  setValorInicioEEPROM();

  // Activa los puntos del display
  tm1637.point(POINT_ON);
}



void loop()
{
  // inicializar botones
  setButton.loop();
  modeButton.loop();
  startButton.loop();

  // Modo set on
  if(setting) {
    setupUpdate();
    // tm1637.display(TimeDisp);
  }
  
  // Cuenta atras.
  if(started && !stop) {
    if(Update == ON) {
      TimeUpdate();
      setDisplayTime();
      if(!stop) {
          digitalWrite(RELE, LOW);
        } else {
          digitalWrite(RELE, HIGH);
        }
      // tm1637.display(TimeDisp);
    }
  }

  // setDisplayTime();
  tm1637.display(TimeDisp);
  delay(100);
}

/*
* FUNCIONES
*/

// Funcion de la interrupcion del timer. Cuenta medio segundo.
void TimingISR()
{
  halfsecond ++;
  Update = ON;
  if(halfsecond == 2){
    halfsecond = 0;
  }
 // Serial.println(second);
  ClockPoint = (~ClockPoint) & 0x01;
}

// función para cambiar el tiempo del contador.
void setupUpdate() {
  switch(settingIndex) {
    case ZERO:
      if(halfsecond == ZERO) {
        TimeDisp[ZERO] = minute / DIEZ;
      } else {
        TimeDisp[ZERO] = 0xFF;
      }
    break;
    case UNO:
      if(halfsecond == ZERO) {
        TimeDisp[UNO] = minute % DIEZ;
      } else {
        TimeDisp[UNO] = 0xFF;
      }
    break;
    case DOS:
      if(halfsecond == ZERO) {
        TimeDisp[DOS] = second / DIEZ;
      } else {
        TimeDisp[DOS] = 0xFF;
      }
    break;
    case TRES:
      if(halfsecond == ZERO) {
        TimeDisp[TRES] = second % DIEZ;
      } else {
        TimeDisp[TRES] = 0xFF;
      }
    break;
  }
}

// Actualización del contador.
void TimeUpdate(void)
{
  if(ClockPoint) tm1637.point(POINT_ON);
  else tm1637.point(POINT_OFF);

  if(halfsecond == ZERO) {
    if(second == ZERO) {
      if(minute == ZERO) {
        stop = true;
      } else {
        minute --;
        second = 59;
      }
    } else {
      second --;
    }
    halfsecond = ZERO;
  }
  Update = OFF;
}

// Manejo de acciones de los botones.
void handlerBtn(Button2& btn) {
  if(btn == setButton) {
    switch (btn.getClickType()) {
      // Sencillo: cambia entre decenas de minuto, minutos, decenas de segundo y segundos.
      case SINGLE_CLICK:
        if(setting) {
          Serial.println("simple setting. Cambia entre unidadess");
          // Seteo de display para limpiar 0xFF
          setDisplayTime();
          // Cambiar minutos y segundos para editar.
          settingIndex++;
          if(settingIndex > 3) {
            settingIndex = 0;
          }
        }
        break;
      // Largo: Si estamos en el modo setting salir, si no activa el modo setting.
      case LONG_CLICK:
        // Si esta activo setting salir si no esta activar
        setting = !setting;
        // Seteo de display para limpiar 0xFF
        setDisplayTime();
        settingIndex = 0;
        Serial.println("long setting. Activar desactivar setting");
        break;
      // Triple click: Guarda el tiempo en la EEPROM
      case TRIPLE_CLICK:
        // Guarda en EEPROM tiempo
        if(setting == false && started == false) {
          for(int i = ZERO; i < 4; i++) {
            EEPROM.update(ADDRESS + i, TimeDisp[i]);
          }
        }
        Serial.println("triple setting. guarda en EEPROM");
        break;
      default:
        Serial.print("SET");
        break;
    }
  } else if(btn == modeButton) {
      // En modo setting modifica el valor de cronometro.
      if(btn.getClickType() == SINGLE_CLICK && setting == true) {
        Serial.println("mode setting modifica el valor de cronometro");
        // modificar contador
        setTimer(settingIndex);
      }
      // reinicia el contador con el valor de la EEPROM.
      if(btn.getClickType() == SINGLE_CLICK && stop == true && started == true) {
        // reiniciar el contador
        Serial.println("reinicia el contador con el valor de la EEPROM");
        stop = false;
        started = false;
        setValorInicioEEPROM();
        // Activa los puntos del display
        tm1637.point(POINT_ON);
        setDisplayTime();
      }
  } else if(btn == startButton) {
    // Empieza el contador
    if(btn.getClickType() == SINGLE_CLICK) {
      if(started == false && setting == false) {
      Serial.println("Empieza contador");
      started = true;
      stop = false;
      // Activar pin rele.
      digitalWrite(RELE, LOW);
    } else {
      // Pausa el contador
      if(started == true && setting == false) {
        Serial.println("Pausa contador");
        stop = !stop;
        // Cambiar estado pin rele
        if(!stop) {
          digitalWrite(RELE, LOW);
        } else {
          digitalWrite(RELE, HIGH);
        }
      }
    }
  }
  }
}

// Set valores de tiempo en el timer.
void setTimer(int index) {

  int val = TimeDisp[index];
  
  switch (index) {
    case ZERO:
      val = minute / DIEZ;
      val++;
      if(val > DOS) {
        minute = minute - VEINTE;
      } else {
        minute += DIEZ;
      }
    break;
    case UNO:
      val = minute % DIEZ;
      val++;
      if(val > NUEVE) {
        minute = minute - NUEVE;
      } else {
        minute++;
      }
    break;
    case DOS:
      val = second / DIEZ;
      val++;
      if(val > 5) {
        second = second - CINCUENTA;
      } else {
        second += DIEZ;
      }
    break;
    case TRES:
      val = second % DIEZ;
      val++;
      if(val > NUEVE) {
        second = second - NUEVE;
      } else {
        second++;
      }
    break;
    }
}

void setDisplayTime() {
  TimeDisp[ZERO] = minute / 10;
  TimeDisp[UNO] = minute % 10;
  TimeDisp[DOS] = second / 10;
  TimeDisp[TRES] = second % 10;
}

// Temporizador con el tiempo guardado en la EEPROM
void setValorInicioEEPROM() {
  // Set valor inicio con valor de la EEPROM.
  for(int i = 0; i < 4; i++) {
    TimeDisp[i] = EEPROM.read(ADDRESS + i);
  }
  minute = TimeDisp[ZERO] * DIEZ + TimeDisp[UNO];
  second = TimeDisp[DOS] * DIEZ + TimeDisp[TRES];
}
#include "teclado.h"
#include <stdio.h>
#include <stdint.h> 
#include <Arduino.h>

#define TECLA_UP 1
#define TECLA_DOWN 2
#define TECLA_LEFT 3
#define TECLA_RIGHT 0
#define TECLA_SELECT 4
#define SIN_PULSAR -1
#define BOTON_A2 5
#define BOTON_A3 6
#define BOTON_A4 7
#define BOTON_A5 8
#define LED_PIN 1


//DECLARACION DE COMPONENTES DEL ARREGLO
teclado_cfg up;
teclado_cfg down;
teclado_cfg left;
teclado_cfg right;
teclado_cfg select;
teclado_cfg botonA2;
teclado_cfg botonA3;
teclado_cfg botonA4;
teclado_cfg botonA5;


teclado_cfg teclas[9] ={right, up,down, left, select, botonA2, botonA3, botonA4, botonA5 };

//variables...
adc_cfg * adcExterno;

volatile uint8_t portchistory = 0xFF;
volatile int pinInterrupcion=-1; 
volatile int ocurrioInterrupcion=0;
int newKey=-1;
int oldKey=-1;
int botonApretado=0;

int canalAnterior;
int  (*callbackAnterior)(int);

//para lcd 
int newKey2=-1;
int oldKey2=-1;

// Convert ADC value to key number
int get_key(unsigned int input)
{
  int adc_key_val[5] ={30, 180, 360, 535, 760 };
    int k;
    for (k = 0; k < 5; k++)
        if (input < adc_key_val[k])
            return k;

    if (k >= 5)
        k = -1;     // No valid key pressed

    return k;
}

//funcion callback adc_cfg:
int obtenerValorDigital(int input){
  
   newKey2=get_key(input);
   if(newKey2 != oldKey2){     
            
      if (!botonApretado && newKey2>=0 && oldKey2==-1){
        teclas[newKey2].callbackDOWN();
        botonApretado=1;
        oldKey2 = newKey2;
      }
      else if(newKey2==-1 && oldKey2>=0){
          teclas[oldKey2].callbackUP();
          botonApretado=0;
          oldKey2 = newKey2;
      }
  }
  adcExterno->canal=canalAnterior;
  adcExterno->callback=callbackAnterior;
  adc_init(adcExterno);
  
  return 1;
}

void key_down_callback(void (*handler)(), int tecla){
   if(tecla<=9)
      teclas[tecla].callbackDOWN= handler;
  }
void key_up_callback(void (*handler)(), int tecla){
    if(tecla<=9)
      teclas[tecla].callbackUP= handler;
    }

int teclado_init(adc_cfg * adcEx){
  //inicializamos variables:
  adcExterno=adcEx;
  cli(); 
  pinInterrupcion=-1;
  //inicializamos ADC_cfg:
  canalAnterior=adcExterno->canal;
  callbackAnterior=adcExterno->callback;
  adcExterno->canal = 0;
  adcExterno->callback= obtenerValorDigital;
  adc_init(adcExterno);
  
  //seteamos los FLAGS para atrapar interrupciones de los pines A0, A2, A3, A4 y A5:   
  // PCICR |= (1 << PCIE1);     // set PCIE0 to enable PCMSK1 scan
  // PCMSK1|= (1<<PCINT11) | (1<<PCINT10); // | (1<<PCINT12)| (1<<PCINT13);// seteamos PCINT9 al 13 para atrapar interrupciones  

   sei();
  return 1;
  }

void teclado_loop(){
    
  adc_loop();
  if(ocurrioInterrupcion){
 
    //determino que boton corresponde a la interrupcion del pin..
    switch(pinInterrupcion){
      case 2:{
        newKey= BOTON_A2;
      }break;

      case 3:
        newKey= BOTON_A3;
       break;

      case 4:
        newKey=BOTON_A4;
        break;

      case 5:
        newKey=BOTON_A5;
        break;

      default:
        newKey=SIN_PULSAR;
    }
   
   //verifico si se apreto un boton o se solto:
    if(oldKey==-1){          //si no habia ningun boton pulsado..
      if(newKey>=0){
        teclas[newKey].callbackDOWN();
        oldKey=newKey;
      }
    }
    else if (newKey==-1){    //si ya habia un boton pulsado y apreto un boton..
      teclas[oldKey].callbackUP();
      oldKey=newKey;
    }
    ocurrioInterrupcion=0;
  }
  
}


ISR (PCINT1_vect){
    uint8_t changedbits;

    changedbits = PINC ^ portchistory;
    portchistory = PINC;

    if(changedbits & (1 << PINC2)) 
    {
        pinInterrupcion=2;
        if( (PINC & (1 << PINC2)) != 4 ){
          /* HIGH to LOW pin change */
          pinInterrupcion=-1;
        }
           
    }
    else if(changedbits & (1 << PINC3)) 
    {
        pinInterrupcion=3;
        if( (PINC & (1 << PINC3)) != 8 ){
          /* HIGH to LOW pin change */
          pinInterrupcion=-1;
        }    
    }
    else if(changedbits & (1 << PINC4)) 
    {
        pinInterrupcion=4;
        if( (PINC & (1 << PINC4)) != 16 ){
          /* HIGH to LOW pin change */
          pinInterrupcion=-1;
        }    
           
    }
    else if(changedbits & (1 << PINC5)) 
    {
        pinInterrupcion=5;
        if( (PINC & (1 << PINC5)) != 32 ){
          /* HIGH to LOW pin change */
          pinInterrupcion=-1;
        }    
           
    }
    ocurrioInterrupcion=1;
}

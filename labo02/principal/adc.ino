#include "adc.h"
#include <stdio.h>
#include <Arduino.h>
adc_cfg * adc2;

volatile int termino; //volatil indica al compilador que no optimice esta variable (no la mueva o alguna gilada).
int valorDig; 

int adc_init(adc_cfg *cfg){

  cli();   
  adc2 = cfg;
  termino=0;
  //inciializamos ADC
  
  //registros
//seteamos el registro ADMUX:  |REFS1|REFS0|ADLAR|-|MUX3|MUX2|MUX1|MUX0|
  ADMUX=0;
  ADMUX |=  (1<<REFS0); //VOLTAJE DE ref IGUAL A Vcc
  //ADMUX |= (1<<ADLAR); //en 1 -> usamos todos los bits de ADCH y solo 2 de ADCL
            //en 0-> usamos todos los bits de ADCL y solo 2 de ADCH
            //CAMBIA LA MANERA DE LEER.
          //lo dejamos en 0 (RIGHT ADJUSTED)
  //setamos los bits MUX para indicar de que canal leer la tension de entrada:
    //ADMUX |= MUX[0..3]   --> str->canal 
  ADMUX |= adc2->canal;


//seteamos el registro ADCSRA:  |ADEN|ADSC|ADATE|ADIF|ADIE|ADPS2|ADPS1|ADPS0|
  ADCSRA=0;
  ADCSRA |= (1<<ADEN); //habilita la conversion
  ADCSRA |= (1<<ADATE); //autoTrigger -> indica que una vez terminada una conversión habra un evento que desencadene comenzar con una nueva.

  
  // seteo la frecuencia del reloj para que pueda atender la interrupcion , debe ser menor a 200khz
  
  ADCSRA |= (1<<ADPS0);
  ADCSRA |= (1<<ADPS1);
  ADCSRA |= (1<<ADPS2);

//seteamos el registro ADCSRB:
  ADCSRB=0; 
  
  ADCSRA |= (1<<ADIE); //habilita la interrupcion
  
  
  
  ADCSRA |= (1<<ADSC); //arranca la conversion  

sei(); //habilita las interrupciones globales
  return 1;
}

void adc_loop(){
  //esperar hasta que se obt el valor (ISR)--> luego callback
  if(termino){
    valorDig= ADCL;
    valorDig |= (ADCH<<8);
    adc2->callback(valorDig);
    termino=0;
  }
}

ISR(ADC_vect){
  termino=1;
  }


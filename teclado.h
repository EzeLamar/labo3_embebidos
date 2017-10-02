#include "adc.h"
#ifndef TECLADO_H
#define TECLADO_H
typedef struct {
   void (*callbackUP)  (); 
   void (*callbackDOWN)();
   
  }teclado_cfg;


void key_down_callback(void (*handler)(), int tecla);

void key_up_callback(void (*handler)(), int tecla);

//inicializa driver de teclado y acepta como parametro una estructura e tipo teclado_cfg.
//int teclado_init(teclado_cfg *tec);
int teclado_init(adc_cfg *adc);

//ejecuta las funciones de callback corespondientes solo cuando se obtuvo el valor.
void teclado_loop();
#endif

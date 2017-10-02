#include "teclado.h"
#include <Arduino.h>
#include <LiquidCrystal.h>
#define LEDPIN 13
#define LCDCANAL 10

//valores de teclas
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

//valores de Estados para la variable "Estado"
#define ESTADO_LA 0
#define ESTADO_AD 1
#define ESTADO_LP 2
#define ESTADO_LM 3

#define tamanoArreglo 200

int seApreto;
volatile int contador3s;
int Estado; 
int EstadoAnterior;
int porcentajeIluminacion;
float medicion [tamanoArreglo];
int numMuestras;
int ultimaPos;


//Variables para LUX.
volatile int flagObtenerLux;
volatile int contadorOVERFLOW;


//DRIVER ADC PARA SENSOR LUX
adc_cfg adcLux;

float multiMap(float val, float * _in, float * _out, uint8_t size)
{
  // take care the value is within range
  // val = constrain(val, _in[0], _in[size-1]);
  if (val <= _in[0]) return _out[0];
  if (val >= _in[size-1]) return _out[size-1];

  // search right interval
  uint8_t pos = 1;  // _in[0] allready tested
  while(val > _in[pos]) pos++;

  // this will handle all exact "points" in the _in array
  if (val == _in[pos]) return _out[pos];

  // interpolate in the right segment for the rest
  return (val - _in[pos-1]) * (_out[pos] - _out[pos-1]) / (_in[pos] - _in[pos-1]) + _out[pos-1];
}

//FUNCION DRIVER LUX:
int ObtLux(int valorDigital) {
 
  //almaceno el valor en el arreglo:
    if(flagObtenerLux){
        //convierto el valor digital a lux:
        float R =  (5.0f*10000.0f)/(0.0049f*(float)valorDigital)-10000.0f;
        float interLux;
        float in[] = {1.0f,3.0f,5.0f,7.0f,10.0f,16.0f,24.0f,41.0f,92.0f};
        float out[] = {100.0f,80.0f,35.0f,15.0f,10.0f,6.0f,3.0f,1.0f,1.0f};
        interLux=  multiMap(R/1000.0f,in,out,9);
        
        //almaceno el valor lux en el arreglo:
        medicion[ultimaPos]=interLux;       
        ultimaPos++;
        if(ultimaPos==tamanoArreglo)
        ultimaPos=0;    
        if(numMuestras<tamanoArreglo)
        numMuestras++;
        flagObtenerLux=0;    
    }
    
  //si no se esta utilizando el driver para convertir Lux..
  //al terminar de convertir chequea si se apreto algun boton.
  teclado_init(&adcLux);
  adc_init(&adcLux);  
  return 1;
}

//FUNCION CONFG DEL RELOJ PARA SENSAR VALORES:
void ArrancarReloj(){

  cli();          // disable global interrupts
  TCCR2A = 0;     // set entire TCCR1A register to 0
  TCCR2B = 0;     // same for TCCR1B

  // set compare match register to desired timer count:
  OCR2A = 157;
  // turn on CTC mode:
  TCCR2A |= (1 << WGM21);

  //setemos el clock con una frecuencia de clk/64:
  TCCR2B |= (1 << CS22);
  TCCR2B |= (1 << CS21);
  TCCR2B |= (1 << CS20);
  
  // enable timer compare interrupt:
  TIMSK2 |= (1 << OCIE2A);

  sei();          // enable global interrupts
  }



//FUNCION PORCENTAJE ILUMINACION
void cambiarIluminacion(int porcentaje){
  int valorIlum= (porcentaje*255)/100;
  analogWrite(LCDCANAL,valorIlum);
}

void Resetear(){
  contadorOVERFLOW=0;
  Estado=ESTADO_LA;
  EstadoAnterior=-1;
  porcentajeIluminacion=80;
  cambiarIluminacion(porcentajeIluminacion);
  int i=0;
  for(i; i<tamanoArreglo; i++){
    medicion[i]=0;
  }
  ultimaPos=0;
  numMuestras=0;
}

float promedioLux(int numMuestras){
  //ASUMO QUE numMuestras ES MENOR O IGUAL QUE 200.
  int prom=0;
  int i=0;
  //numMuestras=200;    //ESTO ES UN PARCHE PARA DSPS
  for(i; i<numMuestras; i++){
    prom+= medicion[i];
  }
  prom= prom/numMuestras;
  return prom;
}

void MinMaxLux(float *min, float *max){
  float maxActual=medicion[0];
  float minActual=maxActual;
  int i=1;
  float actual;
  for(i; i<tamanoArreglo; i++){
    actual=medicion[i];
    if(minActual>actual)
      minActual=actual;
    if(maxActual<actual)
      maxActual=actual;
  }
  *min= minActual;
  *max=maxActual;
}


//-----------SETEO HANDLERS-----
void aprete_UP(){
  Serial.println("aprete UP");
  
  //si nos encotramos en el estado AD
  if(Estado==ESTADO_AD){
    if(porcentajeIluminacion<100){
      porcentajeIluminacion+=20;
      cambiarIluminacion(porcentajeIluminacion);
    }
	
  }
  seApreto=1;
}

void solte_UP(){
  Serial.println("solte UP");
  seApreto=0;
}
  

void aprete_DOWN(){
  Serial.println("aprete DOWN");
   //si nos encotramos en el estado AD
  if(Estado==ESTADO_AD){
    if(porcentajeIluminacion>20){
      porcentajeIluminacion-=20;
      cambiarIluminacion(porcentajeIluminacion);
    }
	
  }
  seApreto=1;
}

void solte_DOWN(){
  Serial.println("solte DOWN");
  seApreto=0;
 
}

void aprete_LEFT(){
  Serial.println("aprete LEFT");
  switch(Estado){
    case ESTADO_LA:{
      Estado=ESTADO_AD;
    }break;

    case ESTADO_AD:{
      Estado=ESTADO_LP;
    }break;

    case ESTADO_LP:{
      Estado=ESTADO_LM;
    }break;

    case ESTADO_LM:{
      Estado=ESTADO_LA;
    }break;
  }
  seApreto=1;
}

void solte_LEFT(){
  Serial.println("solte LEFT");
  seApreto=0;
}

void aprete_RIGHT(){
  Serial.println("aprete RIGHT");
  switch(Estado){
    case ESTADO_LA:{
      Estado=ESTADO_LM;
    }break;

    case ESTADO_LM:{
      Estado=ESTADO_LP;
    }break;

    case ESTADO_LP:{
      Estado=ESTADO_AD;
    }break;

    case ESTADO_AD:{
      Estado=ESTADO_LA;
    }break;
  }
  seApreto=1;
}

void solte_RIGHT(){
  Serial.println("solte RIGHT");
  seApreto=0;
}

void aprete_SELECT(){
  Serial.println("aprete SELECT");
}

void solte_SELECT(){
  Serial.println("solte SELECT");
  
}

void aprete_A2(){
  Serial.println("aprete A2-----------------------------------------------------");
  //reseteo el arreglo, ilum=80 y estado=LA..
  Resetear();
  seApreto=1;
}

void solte_A2(){
  Serial.println("solte A2");
  seApreto=0;
}

void aprete_A3(){
//  Serial.println("aprete A3");
  Estado++;
  if(Estado==4)
    Estado=0;
    
  seApreto=1;
}

void solte_A3(){
  Serial.println("solte A3");
  seApreto=0;
}

void aprete_A4(){
  Serial.println("aprete A4");
  seApreto=1;
}

void solte_A4(){
  Serial.println("solte A4");
  seApreto=0;
}

void aprete_A5(){
  Serial.println("aprete A5");
  seApreto=1;
}

void solte_A5(){
  Serial.println("solte A5");
  seApreto=0;
}

//--------FIN HANDLERS------


//para LCD
const int numRows = 2;
const int numCols = 16;
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);


// teclado_cfg tec;

int msje_solto_boton(int tecla){
  }

int msje_apreto_boton(int tecla){
  }


void setup() {
  //inicializamos driver Lux
  adcLux.canal=5;
  adcLux.callback= ObtLux;

  flagObtenerLux=0;
   
  //MOSTRAMOS CARTEL DE INICIO:
  pinMode(LCDCANAL, OUTPUT);
  cambiarIluminacion(100);
  lcd.begin(numRows,numCols);
  lcd.setCursor(0, 0);
  lcd.print("Labo_2");
  delay(2000);
  lcd.setCursor(0, 1);
  lcd.print("Sist.Emb. 2017");
  delay(2000);
  lcd.setCursor(0, 1);
  lcd.print("2do Cuatrimestre");
  delay(2000);
  //INICIALIZAMOS EN EL ESTADO AL:
  Resetear();
  ArrancarReloj();
  seApreto=0;
  contador3s=0; 
  
  Serial.begin(9600);

  //digo cual es la tecla asociada a que funcion de up y down
  key_up_callback(&solte_UP, TECLA_UP);
  key_up_callback( solte_DOWN, TECLA_DOWN);
  key_up_callback( solte_LEFT, TECLA_LEFT);
  key_up_callback( solte_RIGHT, TECLA_RIGHT);
  key_up_callback( solte_SELECT, TECLA_SELECT);
  key_up_callback( solte_A2, BOTON_A2);
  key_up_callback( solte_A3, BOTON_A3);
  key_up_callback( solte_A4, BOTON_A4);
  key_up_callback( solte_A5, BOTON_A5);

  key_down_callback( aprete_UP, TECLA_UP);
  key_down_callback( aprete_DOWN, TECLA_DOWN);
  key_down_callback( aprete_LEFT, TECLA_LEFT);
  key_down_callback( aprete_RIGHT, TECLA_RIGHT);
  key_down_callback( aprete_SELECT, TECLA_SELECT);
  key_down_callback( aprete_A2, BOTON_A2);
  key_down_callback( aprete_A3, BOTON_A3);
  key_down_callback( aprete_A4, BOTON_A4);
  key_down_callback( aprete_A5, BOTON_A5);


  //INICIALIZO DRIVER DE LUX (QUEDA CONFIGURADO ADC PARA OBTENER VALORES LUX):
  adc_init(&adcLux);  //PISO adc_cfg DEL TECLADO POR EL DEL SENSOR EN EL DRIVER ADC
  
}

void loop() {
  //ARRANCAMOS LOOP DE DRIVER DE TECLADO:
  teclado_loop();
  
  if(flagObtenerLux){ //limpio la pantalla segun la frecuencia con la que obtienen nuevos valores.
 
    
    switch(Estado){
  	  
  	case ESTADO_LA:{
      if(EstadoAnterior!=Estado){
        EstadoAnterior=Estado;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Valor actual:");
        }
        lcd.setCursor(0,1);
        for(int i=0; i<numCols; i++)
          lcd.write(' ');
        lcd.setCursor(0,1);
        lcd.print(medicion[ultimaPos-1]);
  	}break;
  	
  	case ESTADO_AD:{
      if(EstadoAnterior!=Estado){
        EstadoAnterior=Estado;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Iluminacion:");
      }
      lcd.setCursor(0,1);
      for(int i=0; i<numCols; i++)
        lcd.write(' ');
      lcd.setCursor(0,1);
      lcd.print(porcentajeIluminacion);
      lcd.print("%");
  	}break;
  	
  	case ESTADO_LP:{
      if(EstadoAnterior!=Estado){
        EstadoAnterior=Estado;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Valor Promedio:");
      }
      lcd.setCursor(0,1);
      for(int i=0; i<numCols; i++)
        lcd.write(' ');
  		float luxProm= promedioLux(numMuestras);
      
      lcd.setCursor(0,1);
      lcd.print(luxProm);
  	}break;
  	 
  	case ESTADO_LM:{
      if(EstadoAnterior!=Estado){
        EstadoAnterior=Estado;
      }
      lcd.clear();  		
  		float max;
  		float min;
  		MinMaxLux(&min,&max);
      lcd.setCursor(0,0);
      lcd.print("MIN: ");
      lcd.print(min);
      lcd.setCursor(0,1);
      lcd.print("MAX: ");
      lcd.print(max);	
  	}break;  
  	  
    }
  }
  
}

ISR(TIMER2_COMPA_vect){
    //flag de 3 SEGUNDOS---> VUELVO A LA, si no aprete botones en 3 segundos
	//si el estado de un boton no es -1, reinicio el contador 
	
	 //contar hasta 6 con el valor overflow
      
      
      contadorOVERFLOW++;
      if(contadorOVERFLOW==5){
       contadorOVERFLOW=0;
       contador3s++;
       flagObtenerLux=1;
      }
       //SI APRETO UN BOTON ANTES DE LOS 3 SEGUNDOS
      if(seApreto)
		    contador3s=0;
	  
	  //Pasaron 3 segundos
	  if(contador3s==50){
		 Estado=ESTADO_LA;
		  contador3s=0;	 
	  }  

}

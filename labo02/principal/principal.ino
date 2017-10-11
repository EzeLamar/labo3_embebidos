#include "teclado.h"
#include <Arduino.h>
#include <LiquidCrystal.h>
#include <Wire.h>

//PARA LCD:
#define LEDPIN 13
#define LCDCANAL 10

//valores de teclas
#define TECLA_RIGHT 0
#define TECLA_UP 1
#define TECLA_DOWN 2
#define TECLA_LEFT 3
#define TECLA_SELECT 4
#define BOTON_A2 5
#define BOTON_A3 6
#define BOTON_A4 7
#define BOTON_A5 8
#define SIN_PULSAR -1


//Para comunicacion

//LO QUE RECIBO COMO SLAVE
#define OBTENER_LUX '0'
#define OBTENER_MAX '1'
#define OBTENER_MIN '2'
#define OBTENER_PROM '3'
#define OBTENER_TODO '4'

//LO QUE ENVIO COMO SLAVE

#define RESPONDER_LUX '5'
#define RESPONDER_MAX '6'
#define RESPONDER_MIN '7'
#define RESPONDER_PROM '8'
#define RESPONDER_TODO '9'
#define ERROR 'E'

//defino tamaño de los paquetes de cada tipo

#define TAMANO5 27
#define TAMANO6 18
#define TAMANO7 18
#define TAMANO8 27
#define TAMANO9 34

#define tamanioMaxPaquete 80
#define tamanioMinPaquete 7



//CANT MAX DE MUESTRAS:
#define tamanoArreglo 200


//para comunicacion 

char pedido; //bandera
int i;

uint8_t send_buf[tamanioMaxPaquete]; //arreglo que almacena la informacion a transmitir

//valores de Estados para la variable "Estado"
enum Estados { ESTADO_LA, ESTADO_AD, ESTADO_LP, ESTADO_LM};

//variables para cambio de estado:
Estados Estado; 
Estados EstadoAnterior;

//variables para calculos y backlight:
int porcentajeIluminacion;
float medicion [tamanoArreglo];
int numMuestras;
int ultimaPos;


//Variables para ISR Timer:
volatile int flagObtenerLux;
volatile int contadorOVERFLOW;
volatile int contador3s;
volatile int seApreto;


//para LCD
const int numRows = 2;
const int numCols = 16;
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

//DRIVER ADC PARA SENSOR LUX
adc_cfg adcLux;

float multiMap(float val, float * _in, float * _out, uint8_t size){

	// take care the value is within range
	// val = constrain(val, _in[0], _in[size-1]);
	if (val <= _in[0]) 
		return _out[0];
	if (val >= _in[size-1]) 
		return _out[size-1];

	// search right interval
	uint8_t pos = 1;  // _in[0] allready tested
	
	while(val > _in[pos]) 
		pos++;

	// this will handle all exact "points" in the _in array
	if (val == _in[pos]) 
		return _out[pos];

	// interpolate in the right segment for the rest
	return (val - _in[pos-1]) * (_out[pos] - _out[pos-1]) / (_in[pos] - _in[pos-1]) + _out[pos-1];
}

//CALLBACK DRIVER LUX:
int ObtLux(int valorDigital){
 
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

//FUNCION PORCENTAJE ILUMINACION
void cambiarIluminacion(int porcentaje){
  int valorIlum= (porcentaje*255)/100;
  analogWrite(LCDCANAL,valorIlum);
}

//FUNCION PARA REINICIAR EN LA
void Resetear(){
  contadorOVERFLOW=0;
  Estado=ESTADO_LA;
  EstadoAnterior=ESTADO_LM;
  porcentajeIluminacion=80;
  cambiarIluminacion(porcentajeIluminacion);
  int i=0;
  for(i; i<tamanoArreglo; i++){
    medicion[i]=0;
  }
  ultimaPos=0;
  numMuestras=0;
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
  seApreto=1;
}

void solte_SELECT(){
  Serial.println("solte SELECT");
  seApreto=0;
}

void aprete_A2(){
  //reseteo el arreglo, ilum=80 y estado=LA..
  Resetear();
  seApreto=1;
}

void solte_A2(){
  Serial.println("solte A2");
  seApreto=0;
}

void aprete_A3(){
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

// teclado_cfg tec;
int msje_solto_boton(int tecla){
}

int msje_apreto_boton(int tecla){
}


void setup(){
  //inicializamos driver Lux
  adcLux.canal=3;
  adcLux.callback= ObtLux;
   
  //SETEAMOS LCD:
  pinMode(LCDCANAL, OUTPUT);
  cambiarIluminacion(100);
  lcd.begin(numRows,numCols);

  //MOSTRAMOS CARTEL DE INICIO:
  lcd.setCursor(0, 0);
  lcd.print("Labo_2");
  delay(2000);
  lcd.setCursor(0, 1);
  lcd.print("Sist.Emb. 2017");
  delay(2000);
  lcd.setCursor(0, 1);
  lcd.print("2do Cuatrimestre");
  delay(2000);
  
  //Comunicacion (inicializacion):
  
  Wire.begin(8);                // join i2c bus with address #8 
  Wire.onReceive(receiveEvent); // register event
  Wire.onRequest(requestEvent); // register event

  //fin de Comunicacion

  //INICIALIZAMOS EN EL ESTADO "AL":
  Resetear();

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
  //..FIN SETEO DE HANDLERS
  
  //INICIALIZAMOS EL CLOCK PARA SENSAR VALORES:
  cli();          // disable global interrupts

  flagObtenerLux=0;	// SETEAMOS FLAG EN 0
  seApreto=0;
  contador3s=0; 

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
  //..FIN INICIALIZAR CLOCK..

  Serial.begin(9600);

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

				//CALCULO EL PROMEDIO:
				float promedioLux=0;
				//numMuestras=200;    //ESTO ES UN PARCHE PARA DSPS
				for(int i=0; i<numMuestras; i++){
					promedioLux+= medicion[i];
				}
				promedioLux= promedioLux/numMuestras;
				//..fin calculo promedio.

		    	lcd.setCursor(0,1);
		    	lcd.print(promedioLux);
		  	}break;
		  	 
		  	case ESTADO_LM:{
		      	if(EstadoAnterior!=Estado){
		        	EstadoAnterior=Estado;
		      	}
		      	lcd.clear();  		

		  		//OBTENGO MIN Y MAX:
		  		float maxActual=medicion[0];
				float minActual=maxActual;
				float actual;
				for(int i=1; i<tamanoArreglo; i++){
					actual=medicion[i];
				    if(minActual>actual)
				    	minActual=actual;
				    if(maxActual<actual)
				    	maxActual=actual;
				}
				//..fin obt min max.

			    lcd.setCursor(0,0);
			    lcd.print("MIN: ");
			    lcd.print(minActual);
			    lcd.setCursor(0,1);
			    lcd.print("MAX: ");
			    lcd.print(maxActual);	
		  	}break;  
	  	  
	    }//fin switch

	}//fin if

}//fin loop

ISR(TIMER2_COMPA_vect){
    //flag de 3 SEGUNDOS---> VUELVO A LA, si no aprete botones en 3 segundos
	//si el estado de un boton no es -1, reinicio el contador 
	
	 //contar hasta 6 con el valor overflow
    contadorOVERFLOW++;
    if(contadorOVERFLOW==6){
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
// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
//en esta seccion se arman las tramas a ser transmitidas
	
	//OBTENGO MIN Y MAX:
	float maxActual=medicion[0];
	float minActual=maxActual;
	float actual;
	
	//Obtengo min y max
	for(int i=1; i<tamanoArreglo; i++){
					actual=medicion[i];
				    if(minActual>actual)
				    	minActual=actual;
				    if(maxActual<actual)
				    	maxActual=actual;
				}
	//obtengo promedio
	float promedioLux=0;
				//numMuestras=200;    //ESTO ES UN PARCHE PARA DSPS
				for(int i=0; i<numMuestras; i++){
					promedioLux+= medicion[i];
				}
				promedioLux= promedioLux/numMuestras;
				//..fin calculo promedio.
				
	int tamanoTipo;
	
	switch(pedido){
		//en esta seccion armo las tramas y guardo el tamanio de trama en la variable tamanoTipo--> luego es usado en el Segundomensaje 
		case '5':
		{
			//Serial.println(medicion[ultimaPos-1]);
        float numero= medicion[ultimaPos-1];
        char arreglo[6];
       sprintf(arreglo,"%d.%02d", (int) (numero), (int)(numero * 100) - (int) numero * 100);
       Serial.println(arreglo);
       int cantCifras=strlen(arreglo);       
      tamanoTipo=TAMANO5-6+cantCifras;

       
      
			sprintf( (char*)send_buf, "<%i$%c$Valor actual: %s>",  tamanoTipo, pedido, arreglo);	//concatena tipo_msje y "$>".
      Serial.println((char *) send_buf);
			//tamanoTipo=TAMANO5;
			
		}
		break;
		
		case '6':
		{

       float numero= maxActual;
        char arreglo[6];
       sprintf(arreglo,"%d.%02d", (int) (numero), (int)(numero * 100) - (int) numero * 100);
       Serial.println(arreglo);
       int cantCifras=strlen(arreglo);       
      tamanoTipo=TAMANO6-6+cantCifras;
      
              
      
      
			sprintf( (char*)send_buf, "<%i$%c$MAX: %s>", tamanoTipo, pedido, arreglo);
			//tamanoTipo=TAMANO6;
		}
		break;
		
		case '7':
		{

      float numero= minActual;
        char arreglo[6];
       sprintf(arreglo,"%d.%02d", (int) (numero), (int)(numero * 100) - (int) numero * 100);
       Serial.println(arreglo);
       int cantCifras=strlen(arreglo);       
      tamanoTipo=TAMANO7-6+cantCifras;
      
			sprintf( (char*)send_buf, "<%i$%c$MIN: %s>", tamanoTipo, pedido, arreglo);
			//tamanoTipo=TAMANO7;
		}
		break;
		
		case '8':
		{

      float numero= promedioLux;
        char arreglo[6];
       sprintf(arreglo,"%d.%02d", (int) (numero), (int)(numero * 100) - (int) numero * 100);
       Serial.println(arreglo);
       int cantCifras=strlen(arreglo);       
      tamanoTipo=TAMANO8-6+cantCifras;
      
			sprintf( (char*)send_buf, "<%i$%c$PROMEDIO lux: %s>", tamanoTipo, pedido, arreglo);
			//tamanoTipo=TAMANO8;
		}
		break;
		
		case '9':
		{
      float ACTlux= medicion[ultimaPos-1];
      float mini= minActual;
      float maxi=maxActual; 
      float prom= promedioLux;
        char arregloLux[6];
        char arregloMin[6];
        char arregloMax[6];
        char arregloProm[6];
       sprintf(arregloLux,"%d.%02d", (int) (ACTlux), (int)(ACTlux * 100) - (int) ACTlux * 100);
       sprintf(arregloMin,"%d.%02d", (int) (mini), (int)(mini * 100) - (int) mini * 100);
       sprintf(arregloMax,"%d.%02d", (int) (maxi), (int)(maxi * 100) - (int) maxi * 100);
       sprintf(arregloProm,"%d.%02d", (int) (prom), (int)(prom * 100) - (int) prom * 100);
       
       
       
       int cantCifrasLux=strlen(arregloLux); 
       int cantCifrasMin=strlen(arregloMin);    
       int cantCifrasMax=strlen(arregloMax);
       int cantCifrasProm=strlen(arregloProm);
      tamanoTipo=TAMANO9-24+cantCifrasLux+cantCifrasMin+cantCifrasMax+cantCifrasProm;
      
			sprintf( (char*)send_buf, "<%i$%c$%s$%s$%s$%s>", tamanoTipo, pedido,arregloLux, arregloMax, arregloMin, arregloProm);
			//tamanoTipo=TAMANO9;
		}
		break;
		
	}
	
	
	

	 uint8_t auxiliar[3];
	 uint8_t segundoMensaje[tamanoTipo-4];
	 
	 switch(i){
		  case 0:
		  {
			Wire.write(send_buf[0]); 
			i++;
		  } break;
		   
		  case 1:
		  {
		   
			for(int j=1; j<4;j++){
			  auxiliar[j-1]=send_buf[j];
			}
			 Wire.write((char*) auxiliar);
			 i++;
		  }break;
		   
		   case 2:
		   {
			 int k=4; //posicion donde me quede leyendo del buffer 7
			 
			 //voy a pasar el resto del mensaje a OTRO buffer para enviar al master 
			 while(k<tamanoTipo && send_buf[k]!='>'){
				 
				 segundoMensaje[k-4]=send_buf[k];
				 k++;
			 }
			 segundoMensaje[k-4]=send_buf[k];
			 
			 Wire.write((char*) segundoMensaje);
			 //FALTA MODELAR LA PARTE DEL MENSAJE RESPONDER_TODO CON UN SWITCH y una variable que funcione como bandera para saber que rafaga mandar
			 i=0;
			 
			 }
			 break;
		} 
	 
	 
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {
  //en esta seccion se leen las tramas 
  char c;
  char tipo;
  char arTamTotal[2]; //arreglo que contendra el tamanio de trama
  int cont=0;//mantiene una cuenta del tamanio de trama y lo comparo al final con el tamanio almacenado arTamTotal
  if(Wire.read()=='<'){
	  //sirve para llenar el arreglo que contendra el tamanio
	  int l =0;
	  cont++;
	  while (1 < Wire.available() && c!='$') { // loop through all but the last
		c = Wire.read(); // receive byte as a character
		//guardo en s el tamanio total
		if(c!='$')
		arTamTotal[l]=c;
	    l++;
		cont++;
		Serial.print(c);         // print the character
	  }
	  
	  //convierto el valor leido en un entero (TamañoTotal):
		    	int tamTotal=0;
				int digito;
				for(int i=0; i<2; i++){
					digito=  arTamTotal[i]- '0';
					tamTotal= tamTotal*10 + digito;
				}
	  
	  tipo=Wire.read();
	  cont++;
	  
	  switch(tipo){
		case '0':
		pedido=RESPONDER_LUX;
		break;
		
		case '1':
		pedido=RESPONDER_MAX;
		break;
		
		case '2':
		pedido=RESPONDER_MIN;
		break;
		
		case '3':
		pedido=RESPONDER_PROM;
		break;
		
		case '4':
		pedido=RESPONDER_TODO;
		break;
		
	  }
	 
	 while (1 < Wire.available()&& c!='>') { // loop through all but the last
		c = Wire.read(); // receive byte as a character
		Serial.print(c);         // print the character
		cont++;
	  }
	  if(tamTotal!=cont)
		  Serial.println("datos corruptos");
		  
	  //Serial.print(c); 
	  
	  int x = Wire.read();    // receive byte as an integer
	  Serial.println();         // print the integer
  }
  else Serial.println("el mensaje se recibio con error");
}





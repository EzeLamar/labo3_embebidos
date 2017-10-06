#include "mraa.hpp"
#include <iostream>
#include <stdint.h>		//libreria para "sprintf()""
using namespace std;

#define OBTENER_LUX '0'
#define OBTENER_MAX '1'
#define OBTENER_MIN '2'
#define OBTENER_PROM '3'
#define OBTENER_TODO '4'

#define RESPONDER_LUX '5'
#define RESPONDER_MAX '6'
#define RESPONDER_MIN '7'
#define RESPONDER_PROM '8'
#define RESPONDER_TODO '9'
#define ERROR 'E'

#define tamanioMaxPaquete 73
#define tamanioMinPaquete 7

int main() {
	// Inicializar led conectado a GPIO y controlador de I2C
	mraa::Gpio* d_pin = NULL;

	d_pin = new mraa::Gpio(3, true, true); //primer parametro es el PIN que va a utilizar para gpio.

    mraa::I2c* i2c;
    i2c = new mraa::I2c(0); //UTILIZA EL CONSTRUCTOR ISC(BUS ,RAW) CON RAW=NULL Y BUS=0.
    i2c->address(8); //LE INDICA QUE EL ESCLAVO A ESCUCHAR ESTA EN LA DIR 8.

    //inicializo el tipo y el arreglo que almacenara la respuesta:
	char tipo_enviado= OBTENER_TODO;
	uint8_t receive_buf[tamanioMaxPaquete];	//arreglo que almacena la respuesta.
	uint8_t send_buf[tamanioMinPaquete]={"<07$"};		//arreglo que almacena lo recibido

	sprintf( (char*)send_buf, "%s%c$>", send_buf, tipo_enviado);	//concatena tipo_msje y "$>".
    //..fin inializar variables paquete.

    // Indefinidamente
    for (;;){
    	//printf("%s", send_buf);
    	fflush(stdout);
	    //enviar paquete de solicitud al slave:
	    i2c->write(send_buf,tamanioMinPaquete+1); //parametros: (1)datos a enviar y (2) cant de datos a enviar
	    //..fin enviar paquete.

	    //una vez enviado el pedido lee del buffer la respuesta del slave.
	    sleep(1);
	    d_pin->write(0);
	    //leo el primer byte
	    i2c->read(receive_buf,1);

	    printf("paquete: %c\n",(char)receive_buf[0]);
	    if(receive_buf[0]=='<'){	//comienza un paquete..
	    	//leemos del bus el tamanio total del paquete:
	    	i2c->read(receive_buf,3);		//leo campos "tamañoTotal" y separador "$"

	    	printf("token: %c\n", receive_buf[0]);
	    	if(receive_buf[2]=='$'){		//si el 3er Byte es el separador de TamañoTotal..

		    	//convierto el valor leido en un entero (TamañoTotal):
		    	int tamTotal=0;
				int digito;
				for(int i=0; i<2; i++){
					digito=  receive_buf[i]- '0';
					tamTotal= tamTotal*10 + digito;
				}
				printf("el tamaño es: %d", tamTotal);
				//..fin conversion.

				//conociendo el tamaño total, leemos del bus el resto de los Bytes:
				i2c->read(receive_buf,tamTotal-4);	//tamTotal-( "<" + "tamañoTotal" + "$" )=4 Bytes menos.
				//..fin lectura total.
				if(receive_buf[1]=='$'){	//si el 5to Byte es el separador de Tipo..

					//determino y convierto a int el tipo del paquete:

					char tipo_recibido= receive_buf[0];
					int tipoCorrecto=0;
					//chequeo que lo solicitado se corresponda con lo recibido:
					switch(tipo_enviado){
						case OBTENER_LUX: {
							if(tipo_recibido==RESPONDER_LUX)
								tipoCorrecto=1;
						}break;

						case OBTENER_MIN: {
							if(tipo_recibido==RESPONDER_MIN)
								tipoCorrecto=1;
						}break;

						case OBTENER_MAX: {
							if(tipo_recibido==RESPONDER_MAX)
								tipoCorrecto=1;
						}break;

						case OBTENER_PROM: {
							if(tipo_recibido==RESPONDER_PROM)
								tipoCorrecto=1;
						}break;

						case OBTENER_TODO: {
							if(tipo_recibido==RESPONDER_TODO)
								tipoCorrecto=1;
						}break;

						default: {		//en caso de recibir un paquete ERROR!
							tipoCorrecto=0;
						}

					}//fin switch.

					if(tipoCorrecto){	//si se corresponde lo pedido con lo recibido:
						uint8_t payload[tamTotal-7];		//TamañoTotal-7 Bytes
						int finPaquete=0;	//encontre el simbolo de fin de paquete ">"
						int posBuffer=2;
						int posPayload=0;
						while(posBuffer<tamTotal-4 && !finPaquete){	//leo y agrego al arreglo hasta llegar al ">"
							if(receive_buf[posBuffer]=='>')
								finPaquete=1;
							else {
								payload[posPayload]=receive_buf[posBuffer];
								posBuffer++;
								posPayload++;
							}
						}
						if(finPaquete)
							printf((char *) payload[tamTotal-7], "%s\n");
						else
							printf("ERROR! los datos recibidos estan corruptos\n" );
					}
					else
						printf("ERROR! los datos recibidos no concuerdan con los solicitados\n" );

					//ENCIENDO LED DE CONTROL..
    				sleep(1);
    				d_pin->write(1);
    				// Forzar la salida de stdout

				}//..fin separador Tipo
			}//..fin separador TamañoTotal
	    }//..fin comienza Paquete
	    fflush(stdout);
    }//fin BUCLE INFINITO..
    return 0;
}//FIN MAIN..

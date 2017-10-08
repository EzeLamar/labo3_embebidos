#include "iostream"
#include <stdint.h>		//libreria para "sprintf()""
#include <string.h>		//libreria para IO
#include <stdio.h>		//libreria para IO 


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


#define tamanioMinPaquete 7
#define tamanioMaxPaquete 80

using namespace std;


int main(){
	while(1){
		string input="";

		uint8_t send_buf[tamanioMinPaquete];
		uint8_t receive_buf[tamanioMaxPaquete];		//arreglo que almacena la respuesta.

		char tipo_enviado = {0};
		int opcionValida=0;
		while (!opcionValida) {
			cout << "Ingrese la opción a realizar: ";
			getline(cin, input);
			if (input.length() == 1){
			    	tipo_enviado = input[0];
			    	if(tipo_enviado>=OBTENER_LUX && tipo_enviado<=OBTENER_TODO)
				 	opcionValida=1;
				
				else	cout << "Invalid character, please try again" << endl;
			}
	 	}

	 	cout << "You entered: " << tipo_enviado << endl << endl;		//MUESTRO LO QUE LEI

	 	sprintf( (char*)send_buf, "<07$%c$>", tipo_enviado);	//concatena tipo_msje y "$>".
	 	cout << "Paquete Enviado: " << send_buf << endl << endl;

	 	sprintf( (char*)receive_buf, "<27$5$Valor Actual: 100.00>" );
	 	cout << "Paquete Recibido: " << receive_buf << endl << endl;

	 	sprintf( (char*)receive_buf, "<" );
	//	printf("paquete: %c\n",(char)receive_buf[0]);				
		    if(receive_buf[0]=='<'){	//comienza un paquete..
		    	//leemos del bus el tamanio total del paquete:
	//	    	i2c->read(receive_buf,3);		//leo campos "tamañoTotal" y separador "$"
		    	sprintf( (char*)receive_buf, "27$" );
	//	    	printf("token: %c\n", receive_buf[0]);
		    	if(receive_buf[2]=='$'){		//si el 3er Byte es el separador de TamañoTotal..

			    	//convierto el valor leido en un entero (TamañoTotal):
			    	int tamTotal=0;
					int digito;
					for(int i=0; i<2; i++){
						digito=  receive_buf[i]- '0';	//obtengo int correspondiente al char.
						tamTotal= tamTotal*10 + digito;
					}
	//				printf("el tamaño es: %d", tamTotal);
					//..fin conversion.

					//conociendo el tamaño total, leemos del bus el resto de los Bytes:
	//				i2c->read(receive_buf,tamTotal-4);	//tamTotal-( "<" + "tamañoTotal" + "$" )=4 Bytes menos.
					sprintf( (char*)receive_buf, "5$Valor Actual: 100.00>" );
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
							int encontreSeparador=0;
							int contadorSeparadores=0;
							while(posBuffer<tamTotal-4 && !finPaquete){	//leo y agrego al arreglo hasta llegar al ">"
								if(receive_buf[posBuffer]=='>'){
									if(encontreSeparador){
										payload[posPayload]='>';
										encontreSeparador=0;
										contadorSeparadores++;
										posPayload++;
										posBuffer++;
									}
									else
										finPaquete=1;
								}
								else {
									if(receive_buf[posBuffer]=='/'){
										encontreSeparador=1;
										posBuffer++;
									}
									else {
										payload[posPayload]=receive_buf[posBuffer];
										posPayload++;
										posBuffer++;
									}
								}
							}
							//si llego el terminador del paquete y recibi todos los datos (según tamTotal)..
							if((finPaquete)&&(posPayload+contadorSeparadores==tamTotal-7))	 
								printf("%s\n", (char*)payload);
							else
								printf("ERROR! los datos recibidos estan corruptos\n" );
						}
						else
							printf("ERROR! los datos recibidos no concuerdan con los solicitados\n" );

						//ENCIENDO LED DE CONTROL..
	//    				sleep(1);
	//    				d_pin->write(1);
	    				// Forzar la salida de stdout

					}//..fin separador Tipo
				}//..fin separador TamañoTotal
		    }//..fin comienza Paquete
	}
}
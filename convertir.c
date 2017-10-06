#include <stdio.h>

#include <stdint.h>

enum Tipos { OBTENER_LUX,OBTENER_MAX, OBTENER_MIN, OBTENER_PROM, OBTENER_TODO,
RESPONDER_LUX,RESPONDER_MAX, RESPONDER_MIN, RESPONDER_PROM, RESPONDER_TODO, ERROR};

int main(){
	enum Tipos tipo= OBTENER_LUX;
	uint8_t send_buf[7]={"<17$"};
	sprintf( send_buf, "%s%d>", send_buf, tipo);
	printf("secuencia:\n%s\n\n",send_buf);
	//printf("digito: %c\n\n", send_buf[2]);
	int numero=0;
	int digito;
	for(int i=0; i<2; i++){
		digito=  send_buf[1+i]- '0';
		numero= numero*10 + digito;
	}
	printf("numero= %d",numero);
	return 1;
}


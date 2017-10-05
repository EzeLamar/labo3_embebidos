#include <stdio.h>

#include <stdint.h>

enum Tipos { OBTENER_LUX,OBTENER_MAX, OBTENER_MIN, OBTENER_PROM, OBTENER_TODO,
RESPONDER_LUX,RESPONDER_MAX, RESPONDER_MIN, RESPONDER_PROM, RESPONDER_TODO, ERROR};

int main(){
	enum Tipos tipo= OBTENER_LUX;
	uint8_t send_buf[7]={"<07$"};
	sprintf( send_buf, "%s%d", send_buf, tipo);
	printf("%s",send_buf);
	return 1;
}


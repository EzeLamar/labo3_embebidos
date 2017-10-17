#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define numCommands 6
#define port 8080

char clientIP[16];

int handleError(const char *s)
{
	printf("%s: %s\n", s, strerror(errno));
	exit(1);
}

int main(){
	int sockfd, newsockfd, sin_size, numbytes;
		struct sockaddr_in sv_addr;
		struct sockaddr_in cl_addr;
		//INICIALIZO SOCKET
		//
		//DECLARACION --> int socket(int domain,int type,int protocol)
		//
		//DOMAIN --> AF_INET O AF_UNIX
		//AF_INET --> Para usar protocolos ARPA de internet
		//AF_UNIX --> Para crear sockets de comunicacion interna del sistema
		//
		//
		//TYPE --> SOCK_STREAM O SOCK_DGRAM
		//SOCK_STREAM --> Setea que la clase de socket sea de tipo Stream (de flujo)
		//SOCK_DGRAM  --> Setea que la clase de socket sea de Datagramas
		//
		//PROTOCOL -->PROXIMAMENTE
		//-->Por el momento asignarle 0
		//
		// En caso de que la funcion de creacion del socket nos devuelva -1, nos indica un error.
		if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			handleError("Error al crear el socket");
		sv_addr.sin_family = AF_INET;
		sv_addr.sin_port = htons(port);
		sv_addr.sin_addr.s_addr = INADDR_ANY;
		bzero(&(sv_addr.sin_zero), 8);
		int op = 1;
		setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&op, sizeof(op));
		/*BIND
		 * 
		 * -->Su función esencial es asociar un socket con un puerto
		 * -->devolverá -1 en caso de error. 
		 * 
		 * DECLARACION --> int bind(int fd, struct sockaddr *my_addr,int addrlen)
		 * 
		 * FD --> Es el descriptor de fichero socket devuelto por la llamada a socket(). 
		 * 
		 * MY_ADDR --> es un puntero a una estructura sockaddr
		 * 
		 * ADDRLEN --> contiene la longitud de la estructura sockaddr a la cuál apunta el puntero my_addr. Se debe establecer como sizeof(struct sockaddr).
		 * 
		*/
		if(bind(sockfd, (struct sockaddr*)&sv_addr, sizeof(struct sockaddr)) < 0)
			handleError("Error en el bind");
		/*	LISTEN
		 * 
		 * La función listen() se usa si se están esperando conexiones entrantes
		 * --> listen() devolverá -1 en caso de error 
		 * 
		 * DECLARACION --> int listen(int fd,int backlog)
		 * 
		 * FD -->  Es el fichero descriptor del socket, el cual fue devuelto por la llamada a socket()
		 * 
		 * BACKLOG --> Es el número de conexiones permitidas
		 */
		if(listen(sockfd, 1) < 0)
			handleError("Error en el listen");
		
		
		printf("Esperando conexion..\n");
		/*
		 * ACCEPT
		 * 
		 * Acepta una solicitud de conexion entrante (cliente usa "connect()")
		 * --> 
		 * 
		 * FD --> Es el fichero descriptor del socket, que fue devuelto por la llamada a listen(). 
		 * 
		 * ADDR --> Es un puntero a una estructura sockaddr_in en la quel se pueda determinar qué nodo nos está contactando y desde qué puerto
		 * 
		 * ADDRLEN --> Es la longitud de la estructura a la que apunta el argumento addr, por lo que conviene establecerlo como sizeof(struct sockaddr_in)
		 * 
		 * 
		 */
		sin_size = sizeof(struct sockaddr_in);
		if((newsockfd = accept(sockfd, (struct sockaddr*)&cl_addr, (socklen_t*)&sin_size)) < 0)
			handleError("Error al aceptar conexion");
		sprintf(clientIP, "%s", inet_ntoa(cl_addr.sin_addr));
		printf("Conexion establecida con %s\n", clientIP);
return 1;
}
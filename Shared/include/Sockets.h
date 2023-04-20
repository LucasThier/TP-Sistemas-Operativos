#define _POSIX_C_SOURCE 200112L

#ifndef CONEXIONES_H_
#define CONEXIONES_H_

#include <sys/socket.h>
#include <commons/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>

int iniciar_servidor_sin_logger(char* ip, char* puerto);
int esperar_cliente_sin_logger(int socket_servidor);
int crear_conexion_sin_logger(char* ip, char* puerto);
void liberar_conexion(int* socket_cliente);
void enviar_int(int socket_destino, const int* int_a_enviar);
void recibir_int(int socket_origen);

#endif
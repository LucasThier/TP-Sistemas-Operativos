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

int iniciar_servidor(t_log* logger, const char* name, char* ip, char* puerto);
int esperar_cliente(t_log* logger, const char* nombreProceso, int socket_servidor);
int crear_conexion(t_log* logger, const char* nobre_servidor, char* ip, char* puerto);
void liberar_conexion(int socket_cliente);
void enviar_int(t_log* logger, const char* name, int socket_destino, int int_a_enviar);
int recibir_int(t_log* logger, const char* name, int socket_origen);

#endif
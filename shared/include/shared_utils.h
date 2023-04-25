#define _POSIX_C_SOURCE 200112L

#ifndef SHAREDUTILS_H_
#define SHAREDUTILS_H_

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/process.h>
#include <commons/txt.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include <string.h>
#include <commons/collections/list.h>
#include <assert.h>
#include <readline/readline.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

typedef enum
{
	MENSAJE,
	PAQUETE
} op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;


int iniciar_servidor(t_log* logger, const char* name, char* ip, char* puerto);
int esperar_cliente(t_log* logger, const char* nombreProceso, int socket_servidor);
int crear_conexion(t_log* logger, const char* nobre_servidor, char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
void enviar_int(t_log* logger, const char* name, int socket_destino, int int_a_enviar);
int recibir_int(t_log* logger, const char* name, int socket_origen);
t_paquete* crear_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void crear_buffer(t_paquete* paquete);

/*int iniciar_servidor(t_log* logger, const char* name, char* ip, char* puerto);
int esperar_cliente(t_log* logger, const char* nombreProceso, int socket_servidor);
int crear_conexion(t_log* logger, const char* nobre_servidor, char* ip, char* puerto);
void liberar_conexion(int socket_cliente);
void enviar_int(t_log* logger, const char* name, int socket_destino, int int_a_enviar);
int recibir_int(t_log* logger, const char* name, int socket_origen);
void paquete(int);
t_paquete* crear_paquete(void);
t_paquete* crear_super_paquete(void);*/
#endif
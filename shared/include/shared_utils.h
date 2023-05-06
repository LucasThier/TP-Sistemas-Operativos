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


#pragma region Sockets
/**
 * Intenta crear un servidor escucha.
 * @param ip ip de escucha del servidor
 * @param puerto puerto de escucha del servidor
 * @return Devuelve el socket del servidor escucha o 0 en caso de error.
 */
int iniciar_servidor(t_log* logger, const char* name, char* ip, char* puerto);

/**
 * Esperar a que llegue una conexion de un cliente.
 * [BLOQUEANTE]
 * @param socket_servidor el socket del servidor que va a escuchar
 * @return Devuelve el socket del nuevo cliente o 0 en caso de error.
 */
int esperar_cliente(t_log* logger, const char* nombreProceso, int socket_servidor);

/**
 * Intenta conectar con un servidor escucha.
 * @param ip ip de escucha del servidor
 * @param puerto puerto de escucha del servidor
 * @return Devuelve el socket del servidor al que se conecto o 0 en caso de error.
 */
int conectar_servidor(t_log* logger, const char* nobre_servidor, char* ip, char* puerto);

#pragma endregion


#pragma region Paquetes

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

typedef enum
{
	ABRIRPROGRAMA,
	OTROS
} tipo_operacion;

typedef struct
{
	int size;
	void* bytes;
} contenido_paquete;


void enviar_mensaje(char* mensaje, int socket_cliente);
t_paquete* crear_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void crear_buffer(t_paquete* paquete);
void paquete(int);
t_paquete* crear_super_paquete(void);

#pragma endregion

//temporal
void enviar_int(t_log* logger, const char* name, int socket_destino, int int_a_enviar);
int recibir_int(int socket_origen);


#endif
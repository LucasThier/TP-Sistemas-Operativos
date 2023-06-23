#define _POSIX_C_SOURCE 200112L

#ifndef SHAREDUTILS_H_
#define SHAREDUTILS_H_

//seguro que se usan:
#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include <signal.h>
#include <commons/collections/list.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/time.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

//El resto no se si se usan(habria que chequear):
#include <commons/process.h>
#include <commons/txt.h>
#include <string.h>
#include <assert.h>
#include <readline/readline.h>
#include <sys/types.h>
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


/**
 * Libera todos los recursos usados para el socket.
 * @param socket el socket a liberar.
 */
void liberar_conexion(int socket);
#pragma endregion


#pragma region Paquetes

typedef enum
{
	INSTRUCCIONES,
	MENSAGE,
	CPU_PCB,
	KILL
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

t_paquete* crear_paquete(op_code tipoDeOperacion);
void agregar_a_paquete(t_paquete *paquete, void *valor, int tamanio);
void enviar_paquete(t_paquete *paquete, int socket_cliente);
void *serializar_paquete(t_paquete *paquete, int bytes);
void* recibir_paquete(int socket_cliente);
void EnviarMensage(char* Mensage, int SocketDestino);
void eliminar_paquete(t_paquete *paquete);

#pragma endregion

//temporal
void enviar_int(t_log* logger, const char* name, int socket_destino, int int_a_enviar);
int recibir_int(int socket_origen);


#endif

#pragma region TablasDeSegmentos
typedef struct
{
    int id;
    int direccionBase;
    int limite;
} t_Segmento;

typedef struct
{
    int PID;
    t_list* Segmentos;
} t_tablaSegmentos;

t_tablaSegmentos* CrearTablaSegmentos(int PID, t_Segmento* segmento0);
void EliminarTablaSegmentos(t_tablaSegmentos* tabla);

/**
 * Crea un segmento, lo agrega a la tabla y lo retorna
 * @param tabla Pasar NULL si no se quiere agregar a ninguna tabla
 */
t_Segmento* CrearSegmento(t_tablaSegmentos* tabla, int id, int direccionBase, int limite);
void EliminarSegmento(t_tablaSegmentos* tabla, int id);

t_tablaSegmentos* ObtenerTablaSegmentosDelPaquete(t_list* DatosRecibidos);

/*
*   Agregar una tabla de segmentos a un paquete. EL PAQUETE DEBE SER DE TIPO PCB_CPU
*   @param paquete: Si pasas NULL se crea un paquete nuevo
*/
t_paquete* AgregarTablaSegmentosAlPaquete(t_paquete* paquete, t_tablaSegmentos* tabla);

#pragma endregion

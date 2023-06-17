#ifndef MOD_MEMORIA_H_
#define MOD_MEMORIA_H_

#include "../../shared/include/shared_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int InicializarConexiones();
void* EscucharConexiones();
void* AdministradorDeModulo(void*);
void LeerConfigs(char*);
void crearSegmento(Memoria*, int, int);
void eliminarSegmento(Memoria*, int);
void compactarSegmentos(Memoria*);
void inicializarMemoria(Memoria*);


t_config* config;
char* PUERTO_ESCUCHA;
int TAM_MEMORIA;
int TAM_SEGMENTO_0;
int CANT_SEGMENTOS;
int RETARDO_MEMORIA;
int RETARDO_COMPACTACION;
char* ALGORITMO_ASIGNACION;


typedef struct {
    int idSegmento;
    void* direccionBase;
    int limite;
} Segmento;

typedef struct {
    Segmento* segmentos;
    int cantidadSegmentos;
} TablaSegmentos;

typedef struct {
    void* espacioUsuario;
    TablaSegmentos tablaSegmentos;
} Memoria;


char NOMBRE_PROCESO[8] = "Memoria";


//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//PARA EVITAR DEADLOCK HACER LOS WAIT A LOS MUTEX EN EL ORDEN QUE ESTAN DECLARADOS ABAJO
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
void InicializarSemaforos();
sem_t m_UsoDeMemoria;


#endif

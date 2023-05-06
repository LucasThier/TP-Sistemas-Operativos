#ifndef MOD_KERNEL_H_
#define MOD_KERNEL_H_

#include "../../shared/include/shared_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int InicializarConexiones();
void* EscucharConexiones();
void* AdministradorDeModulo(void* arg);

t_config* config;

char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* IP_FILESYSTEM;
char* PUERTO_FILESYSTEM;
char* IP_CPU;
char* PUERTO_CPU;
char* PUERTO_ESCUCHA;
void LeerConfigs(char* path);

typedef struct
{
    char AX[5];
    char BX[5];
    char CX[5];
    char DX[5];

    char EAX[9];
    char EBX[9];
    char ECX[9];
    char EDX[9];

    char RAX[17];
    char RBX[17];
    char RCX[17];
    char RDX[17];
} t_registrosCPU;

typedef struct
{
    int id;
    int direccionBase;
    int tamanoSegmento;
} t_tablaSegmentos;

typedef struct
{
    char* pathArchivoAbierto;
    int posicionPuntero;
} t_tablaArchivosAbiertos;

typedef struct
{
    int PID;
    void* listaInstrucciones;
    int programCounter;
    t_registrosCPU registrosCPU;
    int estimacionProximaRafaga;
    t_list* tablaDeSegmentos;
    unsigned int tiempoLlegadaRedy;
    t_list* tablaArchivosAbiertos;
} t_PCB;

char NOMBRE_PROCESO[7] = "KERNEL";

#endif
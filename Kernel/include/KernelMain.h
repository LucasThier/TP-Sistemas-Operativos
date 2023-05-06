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

char NOMBRE_PROCESO[7] = "KERNEL";

#endif
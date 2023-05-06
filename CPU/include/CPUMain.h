#ifndef MOD_CPU_H_
#define MOD_CPU_H_

#include "../../shared/include/shared_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int InicializarConexiones();
void* EscuchaKernel();

void LeerConfigs(char* path);
t_config* config;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PUERTO_ESCUCHA;

char NOMBRE_PROCESO[4] = "CPU";

#endif
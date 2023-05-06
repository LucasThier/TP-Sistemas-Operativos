#ifndef MOD_FS_H_
#define MOD_FS_H_

#include "../../shared/include/shared_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int InicializarConexiones();
void* EscuchaKernel();

t_config* config;

char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PUERTO_ESCUCHA;

void LeerConfigs(char* path);

char NOMBRE_PROCESO[11] = "FileSystem";

#endif
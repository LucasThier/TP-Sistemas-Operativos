#ifndef MOD_FS_H_
#define MOD_FS_H_

#include "../../shared/include/shared_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int InicializarConexiones();
void* EscuchaKernel();

t_config *config, *configSB;


char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PUERTO_ESCUCHA;
char* BLOCK_SIZE;
char* BLOCK_COUNT;

void LeerConfigs(char* path, char* path2);

char NOMBRE_PROCESO[11] = "FileSystem";

#endif

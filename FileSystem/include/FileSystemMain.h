#ifndef MOD_FS_H_
#define MOD_FS_H_

#include "../../shared/include/shared_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/mman.h>


void InicializarConexiones();
void* EscuchaKernel();
void LeerConfigs(char* p, char* p2);
void LevantarArchivos();
void LiberarMemoria();

t_config *config, *configSB;


char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PUERTO_ESCUCHA;
char* BLOCK_SIZE;
char* BLOCK_COUNT;
FILE* BLOQUES;
FILE* BITMAP;



char NOMBRE_PROCESO[11] = "FileSystem";

#endif

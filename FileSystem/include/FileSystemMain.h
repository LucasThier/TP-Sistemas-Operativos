#ifndef MOD_FS_H_
#define MOD_FS_H_

#include "../../shared/include/shared_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/mman.h>


void InicializarConexiones();
void* EscuchaKernel();
void LeerConfigs(char*, char*);
void LiberarMemoria();

t_config *config, *configSB;

typedef struct{
	char* nombreArchivo;
		int tamanoArchivo;

}BLOQUE;

typedef struct {
	char* nombreArchivo;
	int tamanoArchivo;
	uint32_t* punteroDirecto;
	uint32_t* punteroIndirecto;
}FCB;


char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PUERTO_ESCUCHA;
char* BLOCK_SIZE;
char* BLOCK_COUNT;


char NOMBRE_PROCESO[11] = "FileSystem";

#endif

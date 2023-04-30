#ifndef MOD_KERNEL_H_
#define MOD_KERNEL_H_

#include "../../shared/include/shared_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int InicializarConexiones();
void* AdministradorDeConexiones();
void* AdministradorDeMensajes(void* arg);



char NOMBRE_PROCESO[7] = "KERNEL";

#endif
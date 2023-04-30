#ifndef MOD_MEMORIA_H_
#define MOD_MEMORIA_H_

#include "../../shared/include/shared_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int InicializarConexiones();
void* AdministradorDeConexion();

char NOMBRE_PROCESO[8] = "Memoria";

#endif
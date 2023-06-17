#ifndef MOD_MEMORIA_H_
#define MOD_MEMORIA_H_

#include "../../shared/include/shared_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int InicializarConexiones();
void* EscucharConexiones();
void* AdministradorDeModulo(void* arg);

t_config* config;
char* PUERTO_ESCUCHA;
void LeerConfigs(char* path);


char NOMBRE_PROCESO[8] = "Memoria";


//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//PARA EVITAR DEADLOCK HACER LOS WAIT A LOS MUTEX EN EL ORDEN QUE ESTAN DECLARADOS ABAJO
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
void InicializarSemaforos();
sem_t m_UsoDeMemoria;


#endif
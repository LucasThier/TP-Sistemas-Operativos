#define _POSIX_C_SOURCE 200112L

#ifndef CONEXIONES_H_
#define CONEXIONES_H_

#include <sys/socket.h>
#include <commons/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>

void Iniciar_Servidor(void);

#endif
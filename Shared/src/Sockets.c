#define _POSIX_C_SOURCE 200112L

#include "../include/Sockets.h"


void main(){
    
}

void iniciar_servidor(char* ip, char* puerto)
{
    bool conectado;
    int socket_servidor;
    struct addrinfo hints, *infoServer;

    // setear las hints
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    //buscar direccion del servidor siguiendo las hints
    getaddrinfo(ip, puerto, &hints, &infoServer);
    
    //intentar conectar a cada direccion encontrada hasta
    for (struct addrinfo *p = infoServer; p != NULL; p = p->ai_next)
    {
        if (socket(p->ai_family, p->ai_socktype, p->ai_protocol) == -1){
            //LOGUEAR ERROR AL CREAR EL SOCKET
            continue;
        }

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1)
        {
            //LOGUEAR ERROR EN EL BIND
            close(socket_servidor);
            continue;
        }
        conectado = true;
        break;
    }
    
    if(!conectado)
    {

    }
}
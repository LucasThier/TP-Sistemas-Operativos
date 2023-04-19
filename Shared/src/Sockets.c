#include "../include/Sockets.h"

void iniciar_servidor(char* ip, char* puerto)
{
    int socket_servidor;
    struct addrinfo hints;
    struct addrinfo  *servinfo;

    // Inicializando hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    



}

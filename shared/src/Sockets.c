#include "../include/Sockets.h"

/**
 * Crea un servidor escucha sin logear
 * @param ip ip de escucha del nuevo servidor
 * @param puerto puerto de escucha del nuevo servidor
 * @return Devuelve el socket del nuevo servidor
 */
int iniciar_servidor_sin_logger(char* ip, char* puerto)
{
    bool conectado;
    int socket_servidor;
    struct addrinfo hints, *infoServer;

    // setear las hints
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    //establece la direccion del servidor siguiendo las hints
    getaddrinfo(ip, puerto, &hints, &infoServer);
    
    //intentar conectar a cada direccion encontrada hasta
    for (struct addrinfo *aux = infoServer; aux != NULL; aux = aux->ai_next)
    {
		socket_servidor = socket(aux->ai_family, aux->ai_socktype, aux->ai_protocol);
        
		if (socket_servidor == -1){
            //ERROR AL CREAR EL SOCKET
			printf("ERROR AL CREAR EL SOCKET\n");
            continue;
        }

        if (bind(socket_servidor, aux->ai_addr, aux->ai_addrlen) == -1)
        {
			printf("ERROR BIND%d\n", socket_servidor);
            //ERROR EN EL BIND
            close(socket_servidor);
            continue;
        }
        conectado = true;
        break;
    }
    
    if(!conectado)
    {
        free(infoServer);
        return 0;
    }

    listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(infoServer);

    return socket_servidor;
}

/**
 * Esperar a que llegue una conexion de un cliente sin logear
 * [BLOQUEANTE]
 * @param socket_servidor el socket del servidor que va a escuchar
 * @return Devuelve el socket del nuevo cliente
 */
int esperar_cliente_sin_logger(int socket_servidor)
{
    struct sockaddr_in dir_cliente;
    socklen_t tam_direccion = sizeof(struct sockaddr_in);

    int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

    return socket_cliente;
}

/**
 * Intenta conectar con un servidor escucha sin logear
 * @param ip ip de escucha del servidor
 * @param puerto puerto de escucha del servidor
 * @return Devuelve el socket del nuevo servidor
 */
int crear_conexion_sin_logger(char* ip, char* puerto)
{
    struct addrinfo hints, *infoServer;

    // setear las hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    //establece la direccion del servidor a conectarse siguiendo las hints
    getaddrinfo(ip, puerto, &hints, &infoServer);

    // Crea un socket con la informacion obtenida
    int socket_cliente = socket(infoServer->ai_family, infoServer->ai_socktype, infoServer->ai_protocol);

    // Error al crear el socket
    if(socket_cliente == -1) {
		printf("ERROR AL CREAR EL SOCKET\n");
        return 0;
    }

    // Error al conectar con el servidor
    if(connect(socket_cliente, infoServer->ai_addr, infoServer->ai_addrlen) == -1) {
		printf("ERROR AL conectar con el servidor\n");
        freeaddrinfo(infoServer);
        return 0;
    } else

    freeaddrinfo(infoServer);

    return socket_cliente;
}

// Cierra la coneccion con el servidor y libera toda la memoria utilizada
void liberar_conexion(int* socket_cliente)
{
    close(*socket_cliente);
    *socket_cliente = -1;
}

//envia un int (Temporal para probar las conexiones)
void enviar_int(int socket_destino, const int int_a_enviar)
{
    send(socket_destino, &int_a_enviar, sizeof(int), 0);
}

//recibe un int (Temporal para probar las conexiones) y lo imprime
int recibir_int(int socket_origen)
{
    int int_recibido;
    recv(socket_origen, &int_recibido, sizeof(size_t), 0);

    return int_recibido;
}
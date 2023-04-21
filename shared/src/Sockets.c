#include "../include/Sockets.h"

/**
 * Crea un servidor escucha sin logear
 * @param ip ip de escucha del nuevo servidor
 * @param puerto puerto de escucha del nuevo servidor
 * @return Devuelve el socket del nuevo servidor
 */
int iniciar_servidor(t_log* logger, const char* nombreProceso, char* ip, char* puerto)
{
    bool conectado = false;
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
            log_error(logger,"Error al crear el socket servidor (%s)\n", nombreProceso);
            continue;
        }

        if (bind(socket_servidor, aux->ai_addr, aux->ai_addrlen) == -1)
        {
        	liberar_conexion(socket_servidor);
            log_error(logger,"Error al bindear el socket servidor (%s)\n", nombreProceso);
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
    log_info(logger, "Escuchando en %s:%s (%s)\n", ip, puerto, nombreProceso);

    freeaddrinfo(infoServer);

    return socket_servidor;
}

/**
 * Esperar a que llegue una conexion de un cliente sin logear
 * [BLOQUEANTE]
 * @param socket_servidor el socket del servidor que va a escuchar
 * @return Devuelve el socket del nuevo cliente
 */
int esperar_cliente(t_log* logger, const char* nombreProceso, int socket_servidor)
{
    struct sockaddr_in dir_cliente;
    socklen_t tam_direccion = sizeof(struct sockaddr_in);

    log_info(logger, "Esperando cliente... (%s)\n", nombreProceso);

    int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);
    if(socket_cliente == -1){
        log_error(logger,"Error al aceptar cliente (%s)\n", nombreProceso);
        return 0;
    }else
        log_info(logger, "Nuevo Cliente conectado! (%s) %d\n", nombreProceso, socket_cliente);

    return socket_cliente;
}

/**
 * Intenta conectar con un servidor escucha sin logear
 * @param ip ip de escucha del servidor
 * @param puerto puerto de escucha del servidor
 * @return Devuelve el socket del nuevo servidor
 */
int crear_conexion(t_log* logger, const char* nobre_servidor, char* ip, char* puerto)
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
		log_error(logger, "Error creando el socket para %s:%s", ip, puerto);
        return 0;
    }

    // Error al conectar con el servidor
    if(connect(socket_cliente, infoServer->ai_addr, infoServer->ai_addrlen) == -1) {
		log_error(logger, "Error al conectar (a %s)\n", nobre_servidor);
        freeaddrinfo(infoServer);
        return 0;
    } else
        log_info(logger, "Cliente conectado en %s:%s (a %s)\n", ip, puerto, nobre_servidor);


    freeaddrinfo(infoServer);

    return socket_cliente;
}

// Cierra la coneccion con el servidor y libera toda la memoria utilizada
void liberar_conexion(int socket_cliente)
{
    close(socket_cliente);
    socket_cliente = -1;
}

//envia un int (Temporal para probar las conexiones)
void enviar_int(t_log* logger, const char* nombreProceso, int socket_destino, int int_a_enviar)
{
    log_info(logger, "Numero enviado: %d\n", int_a_enviar);
    send(socket_destino, &int_a_enviar, sizeof(int), 0);
}

//recibe un int (Temporal para probar las conexiones) y lo imprime
int recibir_int(t_log* logger, const char* nombreProceso, int socket_origen)
{
    int int_recibido;
    recv(socket_origen, &int_recibido, sizeof(int), 0);
    //printf(int_recibido);
    return int_recibido;
}
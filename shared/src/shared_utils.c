#include "../include/shared_utils.h"

void eliminar_paquete(t_paquete *paquete)
{
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

void *serializar_paquete(t_paquete *paquete, int bytes)
{
    void *magic = malloc(bytes);
    int desplazamiento = 0;

    memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
    desplazamiento += paquete->buffer->size;

    return magic;
}

void enviar_mensaje(char *mensaje, int socket_cliente)
{
    t_paquete *paquete = malloc(sizeof(t_paquete));

    paquete->codigo_operacion = MENSAJE;
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = strlen(mensaje) + 1;
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

    int bytes = paquete->buffer->size + 2 * sizeof(int);

    void *a_enviar = serializar_paquete(paquete, bytes);

    send(socket_cliente, a_enviar, bytes, 0);

    free(a_enviar);
    eliminar_paquete(paquete);
}

t_paquete *crear_paquete(void)
{
    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = PAQUETE;
    crear_buffer(paquete);
    return paquete;
}

void crear_buffer(t_paquete *paquete)
{
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = 0;
    paquete->buffer->stream = NULL;
}

void agregar_a_paquete(t_paquete *paquete, void *valor, int tamanio)
{
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

    memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
    memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

    paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete *paquete, int socket_cliente)
{
    int bytes = paquete->buffer->size + 2 * sizeof(int);
    void *a_enviar = serializar_paquete(paquete, bytes);

    send(socket_cliente, a_enviar, bytes, 0);

    free(a_enviar);
}

void liberar_conexion(int socket_cliente)
{
    close(socket_cliente);
}

t_log *iniciar_logger(void)
{
    t_log *nuevo_logger;

    nuevo_logger = log_create("tp0.log", "tp0", 1, LOG_LEVEL_INFO);

    return nuevo_logger;
}

t_config *iniciar_config(void)
{
    t_config *config;

    config = config_create("/home/utnso/tp0/client/tp0.config");

    return config;
}

void leer_consola(t_log *logger)
{
    char *linea;

    linea = readline(">");

    while (strcmp(linea, "\0"))
    {
        free(linea);
        linea = readline(">");
    }

    free(linea);
}

void paquete(int conexion)
{
    char *leido;

    t_paquete *paquete = crear_paquete();

    leido = readline(">");

    // Leemos y esta vez agregamos las lineas al paquete
    while (strcmp(leido, "\0"))
    {
        agregar_a_paquete(paquete, leido, strlen(leido) + 1);
        free(leido);
        leido = readline(">");
    }

    enviar_paquete(paquete, conexion);

    // ¡No te olvides de liberar las líneas y el paquete antes de regresar!
    free(leido);
    eliminar_paquete(paquete);
}

void terminar_programa(int conexion, t_log *nuevo_logger, t_config *config)
{
    log_destroy(nuevo_logger);
    config_destroy(config);
    liberar_conexion(conexion);

    /* Y por ultimo, hay que liberar lo que utilizamos (conexion, log y config)
      con las funciones de las commons y del TP mencionadas en el enunciado */
}

/*
int recibir_operacion(int socket_cliente)
{
    int cod_op;
    if (recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
        return cod_op;
    else
    {
        close(socket_cliente);
        return -1;
    }
}*/

void *recibir_buffer(int *size, int socket_cliente)
{
    void *buffer;

    recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
    buffer = malloc(*size);
    recv(socket_cliente, buffer, *size, MSG_WAITALL);

    return buffer;
}

void recibir_mensaje(int socket_cliente)
{
    int size;
    char *buffer = recibir_buffer(&size, socket_cliente);
    //log_info(logger, "Me llego el mensaje %s", buffer);
    free(buffer);
}

t_list *recibir_paquete(int socket_cliente)
{
    int size;
    int desplazamiento = 0;
    void *buffer;
    t_list *valores = list_create();
    int tamanio;

    buffer = recibir_buffer(&size, socket_cliente);
    while (desplazamiento < size)
    {
        memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
        desplazamiento += sizeof(int);
        char *valor = malloc(tamanio);
        memcpy(valor, buffer + desplazamiento, tamanio);
        desplazamiento += tamanio;
        list_add(valores, valor);
    }
    free(buffer);
    return valores;
}

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
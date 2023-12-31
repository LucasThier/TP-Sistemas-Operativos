#include "../include/shared_utils.h"

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
            log_error(logger,"Error al crear el socket servidor");
            continue;
        }

        if (bind(socket_servidor, aux->ai_addr, aux->ai_addrlen) == -1)
        {
        	liberar_conexion(socket_servidor);
            log_error(logger,"Error al bindear el socket servidor");
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
    log_info(logger, "Escuchando en %s:%s", ip, puerto);

    freeaddrinfo(infoServer);

    return socket_servidor;
}

int esperar_cliente(t_log* logger, const char* nombreProceso, int socket_servidor)
{
    struct sockaddr_in dir_cliente;
    socklen_t tam_direccion = sizeof(struct sockaddr_in);

    log_info(logger, "Esperando cliente...");

    int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);
    if(socket_cliente == -1){
        log_error(logger,"Error al aceptar cliente");
        return 0;
    }else
        log_info(logger, "Nuevo Cliente conectado!");

    return socket_cliente;
}


int conectar_servidor(t_log* logger, const char* nombre_servidor, char* ip, char* puerto)
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
    int socket_servidor = socket(infoServer->ai_family, infoServer->ai_socktype, infoServer->ai_protocol);

    // Error al crear el socket
    if(socket_servidor == -1) {
		log_error(logger, "Error creando el socket para %s:%s", ip, puerto);
        return 0;
    }

    // Error al conectar con el servidor
    if(connect(socket_servidor, infoServer->ai_addr, infoServer->ai_addrlen) == -1) {
		log_error(logger, "Error al conectar a %s", nombre_servidor);
        freeaddrinfo(infoServer);
        return 0;
    } else
        log_info(logger, "Cliente conectado a %s en %s:%s", nombre_servidor, ip, puerto);


    freeaddrinfo(infoServer);

    return socket_servidor;
}


//envia un int (Temporal para probar las conexiones)
void enviar_int(t_log* logger, const char* nombreProceso, int socket_destino, int int_a_enviar)
{
    log_info(logger, "Numero enviado: %d", int_a_enviar);
    send(socket_destino, &int_a_enviar, sizeof(int), 0);
}

//recibe un int (Temporal para probar las conexiones) y lo imprime
int recibir_int(int socket_origen)
{
    int int_recibido;
    recv(socket_origen, &int_recibido, sizeof(int), MSG_WAITALL);
    return int_recibido;
}

void liberar_conexion(int socket)
{
    close(socket);
}



t_paquete* crear_paquete(op_code tipoDeOperacion)
{
    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = tipoDeOperacion;
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = 0;
    paquete->buffer->stream = NULL;
    return paquete;
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
    int bytesTotales = paquete->buffer->size + 2 * sizeof(int);
    void *a_enviar = serializar_paquete(paquete, bytesTotales);

    send(socket_cliente, a_enviar, bytesTotales, 0);

    free(a_enviar);
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

    return magic;
}


void* recibir_paquete(int socket_cliente)
{
    op_code cod_op = 0;
    int size;
    int desplazamiento = 0;
    void* buffer;
    int tamanio;

    if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) == -1)
        printf("Error al recibir el codigo de operacio");

    t_list* valores = list_create();
    
    switch (cod_op)
    {
    case INSTRUCCIONES:    
            recv(socket_cliente, &size, sizeof(int), MSG_WAITALL);
            buffer = malloc(size);
            recv(socket_cliente, buffer, size, MSG_WAITALL);

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
            return (void*)valores;    

    case MENSAGE:
            char *mensage;
            recv(socket_cliente, &size, sizeof(int), MSG_WAITALL);
            buffer = malloc(size);
            recv(socket_cliente, buffer, size, MSG_WAITALL);

            memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
            desplazamiento += sizeof(int);
            mensage = malloc(tamanio);
            memcpy(mensage, buffer + desplazamiento, tamanio);
            free(buffer);
            list_destroy(valores);
            return mensage;

        break;

    case CPU_PCB:
            recv(socket_cliente, &size, sizeof(int), MSG_WAITALL);
            buffer = malloc(size);
            recv(socket_cliente, buffer, size, MSG_WAITALL);

            while (desplazamiento < size)
            {
                memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
                desplazamiento += sizeof(int);
                void* valor = malloc(tamanio);
                memcpy(valor, buffer + desplazamiento, tamanio);
                desplazamiento += tamanio;
                list_add(valores, valor);
            }
            free(buffer);
            return (void*)valores;  

    default:
        break;
    }
    return 0;
}

void EnviarMensage(char* Mensage, int SocketDestino)
{
	t_paquete* Msg = crear_paquete(MENSAGE);
	agregar_a_paquete(Msg, Mensage, strlen(Mensage)+1);
	enviar_paquete(Msg, SocketDestino);
	eliminar_paquete(Msg);
}


void eliminar_paquete(t_paquete *paquete)
{
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}


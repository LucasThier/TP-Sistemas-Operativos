#include "../include/shared_utils.h"

#pragma region Sockets
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

#pragma endregion

#pragma region Paquetes

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
    t_list* valores = list_create();
    int tamanio;

    if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) == -1)
        printf("Error al recibir el codigo de operacio");

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

#pragma endregion


t_tablaSegmentos* CrearTablaSegmentos(int PID, t_Segmento* segmento0)
{
    t_tablaSegmentos* tabla = malloc(sizeof(t_tablaSegmentos));
    tabla->PID = PID;
    tabla->Segmentos = list_create();
    if(segmento0 != NULL)
        list_add(tabla->Segmentos, segmento0);
    return tabla;
}

void EliminarTablaSegmentos(t_tablaSegmentos* tabla)
{
    list_destroy_and_destroy_elements(tabla->Segmentos, free);
    free(tabla);
}

t_Segmento* CrearSegmento(t_tablaSegmentos* tabla, int id, int direccionBase, int limite)
{
    t_Segmento* segmento = malloc(sizeof(t_Segmento));
    segmento->id = id;
    segmento->direccionBase = direccionBase;
    segmento->limite = limite;
    if(tabla->Segmentos != NULL)       
        list_add(tabla->Segmentos, segmento);
    return segmento;
}


void EliminarSegmento(t_tablaSegmentos* tabla, int id)
{
    for(int i = 0; i < list_size(tabla->Segmentos); i++)
    { 
        t_Segmento* segmento = (t_Segmento*)list_get(tabla->Segmentos, i);
        if(segmento->id == id)
        {
            list_remove(tabla->Segmentos, i);
            free(segmento);
            break;
        }
    }
}

t_tablaSegmentos* ObtenerTablaSegmentosDelPaquete(t_list* DatosRecibidos)
{
    int PID = *(int*)list_remove(DatosRecibidos, 0);
    t_tablaSegmentos* Tabla = CrearTablaSegmentos(PID, NULL);
    int CantSegmentos = *(int*)list_remove(DatosRecibidos, 0);

    for(int i=0; i < CantSegmentos; i++)
    {
        int ID = *(int*)list_remove(DatosRecibidos, 0);
        int DireccionBase = *(int*)list_remove(DatosRecibidos, 0);
        int Limite = *(int*)list_remove(DatosRecibidos, 0);

        CrearSegmento(Tabla, ID, DireccionBase, Limite);
    }
    return Tabla;
}

t_paquete* AgregarTablaSegmentosAlPaquete(t_paquete* paquete, t_tablaSegmentos* tabla)
{
    t_paquete* pqt = paquete;
    if (paquete == NULL)
        pqt = crear_paquete(CPU_PCB);
    
    agregar_a_paquete(pqt, &(tabla->PID), sizeof(int));
    int cantSegmentos = list_size(tabla->Segmentos);
    agregar_a_paquete(pqt, &cantSegmentos, sizeof(int));

    for(int i=0; i < cantSegmentos; i++)
    {
        t_Segmento* segmento = (t_Segmento*)list_get(tabla->Segmentos, i);
        agregar_a_paquete(pqt, &(segmento->id), sizeof(int));
        agregar_a_paquete(pqt, &(segmento->direccionBase), sizeof(int));
        agregar_a_paquete(pqt, &(segmento->limite), sizeof(int));
    }
    return pqt;
}
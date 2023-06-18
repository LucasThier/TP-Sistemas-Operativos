#include "../include/MemoriaMain.h"


t_log* Memoria_Logger;

int SocketMemoria;

void sighandler(int s) {
	liberar_conexion(SocketMemoria);
	exit(0);
}


int main(int argc, char* argv[])
{
	signal(SIGINT, sighandler);

	Memoria_Logger = log_create("Memoria.log", NOMBRE_PROCESO, true, LOG_LEVEL_INFO);
	
	LeerConfigs(argv[1]);

	inicializarMemoria(MEMORIA);

	InicializarSemaforos();

	InicializarConexiones();

	while(true){
		log_info(Memoria_Logger, "hilo principal esta ejecutando");
		sleep(10);
	}

	return EXIT_SUCCESS;
}

//Crea un Hilo que escucha conexiones de los modulos
int InicializarConexiones()
{
	//Crear hilo escucha
	pthread_t HiloEscucha;
    if (pthread_create(&HiloEscucha, NULL, EscucharConexiones, NULL) != 0) {
        exit(EXIT_FAILURE);
    }
	return 1;
}

//Inicia un servidor en el que escucha por modulos permanentemente y cuando recibe uno crea un hilo para administrar esaa conexion
void* EscucharConexiones()
{
	SocketMemoria = iniciar_servidor(Memoria_Logger, NOMBRE_PROCESO, "127.0.0.1", PUERTO_ESCUCHA);
	
	if(SocketMemoria != 0)
	{
		//Escuchar por modulos en bucle
		while(true)
		{
			int SocketCliente = esperar_cliente(Memoria_Logger, NOMBRE_PROCESO, SocketMemoria);

			if(SocketCliente != 0)
			{	
				//Crea un hilo para el modulo conectado
				pthread_t HiloAdministradorDeMensajes;
				if (pthread_create(&HiloAdministradorDeMensajes, NULL, AdministradorDeModulo, (void*)&SocketCliente) != 0) {
					exit(EXIT_FAILURE);
				}
			}
		}
	}

	liberar_conexion(SocketMemoria);
	return (void*)EXIT_FAILURE;
}

//Acciones a realizar para cada modulo conectado
void* AdministradorDeModulo(void* arg)
{
	int* SocketClienteConectado = (int*)arg;

	//bucle que esperara peticiones del modulo conectado
	//y realiza la operacion
	while(true)
	{
		char* PeticionRecibida = (char*)recibir_paquete(*SocketClienteConectado);

		char* Pedido = strtok(PeticionRecibida, " ");

		//crear estructuras administrativas y enviar la tabla de segmentos
		if(strcmp(Pedido, "CREAR_PROCESO")==0)
		{
			//PID del proceso a inicializar
			char* PID = strtok(NULL, " ");
			
			//EnviarTablaDeSegmentos(TablaInicial, *SocketClienteConectado);
		}
		else if(strcmp(Pedido, "FINALIZAR_PROCESO")==0)
		{
			//PID del proceso a finalizar
			char* PID = strtok(NULL, " ");

			//finalizar_proceso(PID);
			//no enviar nada al kernel, solo finalizar el proceso
		}
		//enviar el valor empezando desde la direccion recibida y con la longitud recibida
		else if(strcmp(Pedido, "MOV_IN")==0)
		{
			//PID del proceso
			char* PID = strtok(NULL, " ");
			//direccion donde buscar el contenido
			char* NumSegmento = strtok(NULL, " ");
			char* Desplazamiento = strtok(NULL, " ");
			//longitud del contenido a buscar
			char* Longitud = strtok(NULL, " ");	

			
			char*  Contenido;// = leer_contenido(Direccion, Longitud);

			//enviar el contenido encontrado o SEG_FAULT en caso de error
			EnviarMensage(Contenido, *SocketClienteConectado);
		}
		//guardar el valor recibido en la direccion recibida
		else if(strcmp(Pedido, "MOV_OUT")==0)
		{
			//PID del proceso
			char* PID = strtok(NULL, " ");
			//direccion donde guardar el contenido
			char* NumSegmento = strtok(NULL, " ");
			char* Desplazamiento = strtok(NULL, " ");
			//valor a guardar TERMINADO EN \0
			char* Valor = strtok(NULL, " ");

			//guardar_contenido(Valor, Direccion);
			//eviar SEG_FAULT en caso de error sino enviar cualquier otra cadena de caracteres
		}
		//crear un segmento para un proceso
		else if(strcmp(Pedido, "CREATE_SEGMENT")==0)
		{
			//PID del proceso
			char* PID = strtok(NULL, " ");
			//ID del segmento a crear
			char* ID = strtok(NULL, " ");
			//Tamano del segmento a crear
			char* Tamanio = strtok(NULL, " ");

			//crear_segmento(PID, ID, Tamanio);
			//La pagina 11 detalla lo que tiene que enviar esta funcion al kernel
		}
		//eliminar un segmento de un proceso
		else if(strcmp(Pedido, "DELETE_SEGMENT")==0)
		{
			//PID del proceso
			char* PID = strtok(NULL, " ");
			//ID del segmento a eliminar
			char* ID = strtok(NULL, " ");

			//eliminar_segmento(PID, ID);
			//la funcion debe retornar la tabla de segmentos del proceso actualizada			
		}

		//agregar el resto de operaciones entre FS y memoria
	}

	liberar_conexion(*SocketClienteConectado);
	return NULL;
}


void LeerConfigs(char* path)
{
    config = config_create(path);

    PUERTO_ESCUCHA = config_get_string_value(config, "PUERTO_ESCUCHA");

	char *tam_memoria = config_get_string_value(config, "TAM_MEMORIA");
	TAM_MEMORIA= atoi(tam_memoria);

	char *tam_segmento_0 = config_get_string_value(config, "TAM_SEGMENTO_0");
	TAM_SEGMENTO_0= atoi(tam_segmento_0);

	char *cant_segmentos = config_get_string_value(config, "CANT_SEGMENTOS");
	CANT_SEGMENTOS= atoi(cant_segmentos);

	char *retardo_memoria = config_get_string_value(config, "RETARDO_MEMORIA");
	RETARDO_MEMORIA= atoi(retardo_memoria);

	char *retardo_compactacion = config_get_string_value(config, "RETARDO_COMPACTACION");
	RETARDO_COMPACTACION= atoi(retardo_compactacion);

	ALGORITMO_ASIGNACION = config_get_string_value(config, "ALGORITMO_ASIGNACION");



}

//Inicializa los semaforos
void InicializarSemaforos()
{
	sem_init(&m_UsoDeMemoria, 0, 1);
}

void inicializarMemoria(Memoria* memoria) {
    // Asignar memoria para el espacio de usuario
    memoria->espacioUsuario = malloc(TAM_MEMORIA);

    // Crear segmento compartido (segmento 0)
    memoria->tablaSegmentos.segmentos[0].idSegmento = 0;
    memoria->tablaSegmentos.segmentos[0].direccionBase = memoria->espacioUsuario;
    memoria->tablaSegmentos.segmentos[0].limite = TAM_SEGMENTO_0;
}

void crearSegmento(Memoria* memoria, int idSegmento, int tamanoSegmento) {
    // Buscar espacio contiguo disponible para el segmento
    // utilizando el algoritmo de asignación especificado (FIRST, BEST, WORST)

    // Actualizar la tabla de segmentos con la información del nuevo segmento
}

void eliminarSegmento(Memoria* memoria, int idSegmento) {
    // Marcar el segmento como libre en la tabla de segmentos

    // Consolidar huecos libres aledaños si los hay
}

void compactarSegmentos(Memoria* memoria) {
    // Mover los segmentos para eliminar los huecos libres

    // Actualizar la tabla de segmentos con las nuevas direcciones bases
}


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

	inicializarMemoria();

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

	char* alg_asig = config_get_string_value(config, "ALGORITMO_ASIGNACION");
	ALGORITMO_ASIGNACION = atoi(alg_asig);
	if(alg_asig=="BEST")ALGORITMO_ASIGNACION=0;
	else if(alg_asig=="WORST")ALGORITMO_ASIGNACION=1;
	else ALGORITMO_ASIGNACION=2;

}

//Inicializa los semaforos
void InicializarSemaforos()
{
	sem_init(&m_UsoDeMemoria, 0, 1);
}

void inicializarMemoria() {
	TABLA_SEGMENTOS=list_create();  // Establecer la cantidad de segmentos
	TABLA_HUECOS=list_create();  // Establecer la cantidad de segmentos

	Segmento* seg= malloc(sizeof(Segmento*));

    // Asignar memoria para el espacio de usuario
    MEMORIA = (void *) malloc(TAM_MEMORIA);
    memset(MEMORIA,'0',TAM_MEMORIA);
    log_info(Memoria_Logger,"la direccion base del seg0 es: %p",MEMORIA);

    seg->PID=-1;
	seg->idSegmento=0;
	seg->direccionBase=MEMORIA;
	seg->limite=TAM_SEGMENTO_0;
	list_add(TABLA_SEGMENTOS,seg);

	
	// memset(seg->direccionBase,'1', seg->limite);
	// log_info(Memoria_Logger,"la direccion base del seg0 es: %p",seg->direccionBase);

	// //Obtener un puntero al tipo de datos deseado (por ejemplo, char*)
	// char* datos = (char*)seg->direccionBase;

	// //Acceder y leer los datos asignados
	// for (int i = 0; i < seg->limite +5; i++) {
	//     log_info(Memoria_Logger,"dato %c, posicion %p", datos[i],(seg->direccionBase)+i);
	// }
}

int validarSegmento(int idSeg,int desplazamientoSegmento){
	//valida la existencia del segmento y checkea que el desplazamiento no tire seg_fault
	int aux=0;
	Segmento* seg=list_get(TABLA_SEGMENTOS,aux);

	while(list_size(TABLA_SEGMENTOS)>aux){
		if(seg->idSegmento==idSeg) {
			if(((int) seg->direccionBase + seg->limite) >= ((int) seg->direccionBase + desplazamientoSegmento)) return 1;
			else return 0;
		}
		aux++;
		log_info(Memoria_Logger,"tam tabla y aux %d %d",list_size(TABLA_SEGMENTOS),aux);
		if(list_size(TABLA_SEGMENTOS)>aux)seg=list_get(TABLA_SEGMENTOS,aux);
	}

	return 0;
}

void crearSegmento(int idSeg, int tamanoSegmento, int PID) {
    // Buscar espacio contiguo disponible para el segmento
    // utilizando el algoritmo de asignación especificado (FIRST, BEST, WORST)
	if(list_size(TABLA_HUECOS)!=0){
		switch(ALGORITMO_ASIGNACION){
			case 0:
				BestFit();
				break;
			case 1:
				WorstFit();
				break;
			case 2:
				FirstFit();
				break;
		}
		}else if(list_size(TABLA_SEGMENTOS)!=1) {
			Segmento* lastSeg =list_get(TABLA_SEGMENTOS,list_size(TABLA_SEGMENTOS));
			AgregarSegmento(lastSeg,PID,tamanoSegmento,idSeg);

		}else{
			void* seg0=list_get(TABLA_SEGMENTOS,0);
			AgregarSegmento(seg0,PID,tamanoSegmento,idSeg);
		}

    // Actualizar la tabla de segmentos con la información del nuevo segmento
	//list_add(TABLA_SEGMENTOS,seg);
}
void AgregarSegmento(Segmento* lastSeg,int PID,int tamanoSegmento,int idSeg){
	Segmento* seg;
	seg->PID=PID;
	seg->direccionBase=(lastSeg->direccionBase) + (lastSeg->limite+1);
	seg->limite=tamanoSegmento;
	seg->idSegmento=idSeg;
	list_add(TABLA_SEGMENTOS,seg);
	memset(seg->direccionBase,'\0',seg->limite);
}

void eliminarSegmento(int idSegmento, int PID){
	int indice = buscarSegmento(PID, idSegmento);

	if(indice != -1) {
		Segmento* seg=list_get(TABLA_SEGMENTOS,indice);

		memset(seg->direccionBase,'\0',seg->limite);
		list_remove(TABLA_SEGMENTOS,indice);
		list_add(TABLA_HUECOS,seg);
		log_info(Memoria_Logger,"Eliminado con exito");	
	}
	else 
		log_info(Memoria_Logger,"SEG_FAULT");	
}

char* leerSegmento(int PID, int IdSeg, int Offset, int Longitud){
	int indice = buscarSegmento(PID, IdSeg);

	if(indice != -1) {
		Segmento* seg=list_get(TABLA_SEGMENTOS,indice);

		char buffer[Longitud];
		if(validarSegmento(IdSeg,(Offset + Longitud)) == 1){
			memcpy(buffer,(seg->direccionBase + Offset),Longitud);
			log_info(Memoria_Logger,"dato es: %s",buffer);	
			return buffer;
		}
		else{
			log_info(Memoria_Logger,"error");	
			return "SEG_FAULT";
		} 
	}
}

char* escribirSegmento(int PID, int IdSeg, int Offset, int Longitud, char* datos){
	int indice = buscarSegmento(PID, IdSeg);

	log_info(Memoria_Logger,"dato es: %s",datos);	

	if(indice != -1) {
		Segmento* seg=list_get(TABLA_SEGMENTOS,indice);

		char buffer[Longitud];
		if(validarSegmento(IdSeg,(Offset + Longitud)) == 1){
			for(int i = 0; i < Longitud; i ++)
				memset(seg->direccionBase,datos,Longitud);

			return "OK";
		}
		else
			return "SEG_FAULT";
	}
}

int buscarSegmento(int PID, int idSeg){
	int aux=0;
	Segmento* seg=list_get(TABLA_SEGMENTOS,aux);

	while(list_size(TABLA_SEGMENTOS)>aux){
		if(seg->idSegmento==idSeg && seg->PID == PID) return aux;
		aux++;
		if(list_size(TABLA_SEGMENTOS)>aux)seg=list_get(TABLA_SEGMENTOS,aux);
	}
	return -1;
}

void compactarSegmentos() {
    // Mover los segmentos para eliminar los huecos libres

    // Actualizar la tabla de segmentos con las nuevas direcciones bases
}

void BestFit(){

}

void WorstFit(){

}

void FirstFit(){

}

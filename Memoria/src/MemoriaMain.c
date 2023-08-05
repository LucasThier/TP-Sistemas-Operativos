#include "../include/MemoriaMain.h"


t_log* Memoria_Logger;

int SocketMemoria;

void sighandler(int s) {
	ModuloDebeTerminar = true;

	free(MEMORIA);

	log_destroy(Memoria_Logger);

	config_destroy(ConfigsIps);
	config_destroy(config);
	
	list_destroy_and_destroy_elements(TABLA_SEGMENTOS, free);
	list_destroy_and_destroy_elements(TABLA_HUECOS, free);

	liberar_conexion(SocketMemoria);
	exit(0);
}


int main(int argc, char* argv[])
{
	//signal(SIGINT, sighandler);
	//signal(SIGSEGV, sighandler);

	Memoria_Logger = log_create("Memoria.log", NOMBRE_PROCESO, true, LOG_LEVEL_INFO);
	
	LeerConfigs(argv[1]);

	inicializarMemoria();

/*	VerHuecos();
	crearSegmento(1, 256, 0);
	VerHuecos();
	crearSegmento(2, 256, 0);
	VerHuecos();
	crearSegmento(3, 256, 0);
	VerHuecos();
	crearSegmento(4, 256, 0);
	VerHuecos();
*/

/*

	crearSegmento(1, 128, 0);
	crearSegmento(1, 64, 1);
	crearSegmento(2, 64, 1);
	crearSegmento(3, 64, 1);
	crearSegmento(4, 64, 1);
	crearSegmento(5, 64, 1);
	crearSegmento(6, 64, 1);
	crearSegmento(7, 64, 1);
	crearSegmento(8, 64, 1);
	VerHuecos();
	eliminarSegmento(1, 0);
	VerHuecos();
	crearSegmento(9, 64, 1);
	VerHuecos();
	crearSegmento(1, 256, 2);
	VerHuecos();*/



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
	SocketMemoria = iniciar_servidor(Memoria_Logger, NOMBRE_PROCESO, "0.0.0.0", PUERTO_ESCUCHA);
	
	if(SocketMemoria != 0)
	{
		//Escuchar por modulos en bucle
		while(!ModuloDebeTerminar)
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
	int SocketClienteConectado = *(int*)arg;

	//bucle que esperara peticiones del modulo conectado
	//y realiza la operacion
	while(!ModuloDebeTerminar)
	{
		char* PeticionRecibida = (char*)recibir_paquete(SocketClienteConectado);

		printf("PeticionRecibida: %s\n",PeticionRecibida);

		char* Pedido = strtok(PeticionRecibida, " ");

		if(strcmp(Pedido, "CREAR_PROCESO")==0)
		{
			char* PID = strtok(NULL, " ");
			
			//Crear estructuras administrativas NO enviar nada a Kernel
			log_info(Memoria_Logger,"Creación de Proceso PID: %s",PID);
		}
		else if(strcmp(Pedido, "FINALIZAR_PROCESO")==0)
		{
			//PID del proceso a finalizar
			char* PID = strtok(NULL, " ");

			//finalizar_proceso(PID);
			//NO enviar nada al kernel, solo borrar las estructuras y liberar la memoria
			finalizarProceso(atoi(PID));
			log_info(Memoria_Logger,"Eliminación de Proceso PID: %s",PID);
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
			char* Remitente = strtok(NULL, " ");

			sem_wait(&m_UsoDeMemoria);
			char*  Contenido = leerSegmento(atoi(PID),atoi(NumSegmento),atoi(Desplazamiento),atoi(Longitud), Remitente);
			sem_post(&m_UsoDeMemoria);

			printf("Contenido: %s\n",Contenido);

			sleep(RETARDO_MEMORIA);

			sleep(RETARDO_MEMORIA);
			//enviar el contenido encontrado o SEG_FAULT en caso de error
			EnviarMensage(Contenido, SocketClienteConectado);
		}
		//guardar el valor recibido en la direccion recibida
		else if(strcmp(Pedido, "MOV_OUT")==0)
		{
			printf("MOV_OUT\n");

			//PID del proceso
			char* PID = strtok(NULL, " ");
			//direccion donde guardar el contenido
			char* NumSegmento = strtok(NULL, " ");
			char* Desplazamiento = strtok(NULL, " ");
			//valor a guardar TERMINADO EN \0
			char* Valor = strtok(NULL, " ");
			char* Remitente = strtok(NULL, " ");

			printf("Valor a escribir en el MOV_OUT: %s\n",Valor);

			sem_wait(&m_UsoDeMemoria);
			char* Contenido = escribirSegmento(atoi(PID),atoi(NumSegmento),atoi(Desplazamiento), Valor, Remitente);
			sem_post(&m_UsoDeMemoria);
			
			sleep(RETARDO_MEMORIA);
			//eviar SEG_FAULT en caso de error sino enviar cualquier otra cadena de caracteres
			EnviarMensage(Contenido, SocketClienteConectado);
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
			
			/*enviar la respuesta correspondiente a la situacion, ya sea:
			OUT_OF_MEMORY
			COMPACTAR <OCUPADO/cualquier otra cadena si no hay operacion entre FS y Mem>
			Cualquier otra cadena si hay memoria disponible y no hay que compactar
			*/
			char* Contenido = crearSegmento(atoi(ID),atoi(Tamanio),atoi(PID));

			if(strcmp(Contenido,"COMPACTAR")==0)
			{
				EnviarMensage("COMPACTAR", SocketClienteConectado);

				//Espero orden de compactacion
				recibir_paquete(SocketClienteConectado);

				compactarSegmentos();
				crearSegmento(atoi(ID),atoi(Tamanio),atoi(PID));
				sleep(RETARDO_COMPACTACION);
				EnviarMensage("COMPACTACION TERMINADA", SocketClienteConectado);				
			}
			else
			{
				sleep(RETARDO_MEMORIA);
				EnviarMensage(Contenido, SocketClienteConectado);
			}			
		}
		//eliminar un segmento de un proceso
		else if(strcmp(Pedido, "DELETE_SEGMENT")==0)
		{
			//PID del proceso
			char* PID = strtok(NULL, " ");
			//ID del segmento a eliminar
			char* ID = strtok(NULL, " ");

			//eliminar_segmento(PID, ID);
			//enviar SEG_FAULT en caso de error sino enviar cualquier otra cadena de caracteres			
			char* Contenido = eliminarSegmento(atoi(ID), atoi(PID));
			EnviarMensage(Contenido, SocketClienteConectado);
		}
		free(PeticionRecibida);
	}
	liberar_conexion(SocketClienteConectado);
	pthread_exit(NULL);
}


void LeerConfigs(char* path)
{

	ConfigsIps = config_create("cfg/IPs.cfg");
	PUERTO_ESCUCHA = config_get_string_value(ConfigsIps, "PUERTO_ESCUCHA");

    config = config_create(path);
	char *tam_memoria = config_get_string_value(config, "TAM_MEMORIA");
	TAM_MEMORIA= atoi(tam_memoria);
	char *tam_segmento_0 = config_get_string_value(config, "TAM_SEGMENTO_0");
	TAM_SEGMENTO_0= atoi(tam_segmento_0);
	char *cant_segmentos = config_get_string_value(config, "CANT_SEGMENTOS");
	CANT_SEGMENTOS= atoi(cant_segmentos);
	char *retardo_memoria = config_get_string_value(config, "RETARDO_MEMORIA");
	RETARDO_MEMORIA= atoi(retardo_memoria)/1000;
	char *retardo_compactacion = config_get_string_value(config, "RETARDO_COMPACTACION");
	RETARDO_COMPACTACION= atoi(retardo_compactacion)/1000;
	char* alg_asig = config_get_string_value(config, "ALGORITMO_ASIGNACION");

	if(strcmp(alg_asig,"BEST")==0)ALGORITMO_ASIGNACION=0;
	else if(strcmp(alg_asig,"WORST")==0)ALGORITMO_ASIGNACION=1;
	else ALGORITMO_ASIGNACION=2;
}

//Inicializa los semaforos
void InicializarSemaforos()
{
	sem_init(&m_UsoDeMemoria, 0, 1);
}

void VerMem(){		
	
	for(int i = 0; i<list_size(TABLA_SEGMENTOS); i++){
		Segmento* aux = list_get(TABLA_SEGMENTOS,i);

		log_info(Memoria_Logger,"direccion base es: %p",aux->direccionBase);
		log_info(Memoria_Logger,"ID: %d",aux->idSegmento);
		log_info(Memoria_Logger,"PID: %d",aux->PID);
		log_info(Memoria_Logger,"LIMITE: %d",aux->limite);

		//Obtener un puntero al tipo de datos deseado (por ejemplo, char*)
		char* datos = (char*)aux->direccionBase;

		//Acceder y leer los datos asignados
		for (int i = 0; i < aux->limite; i++) {
			log_info(Memoria_Logger,"dato %c, posicion %p", datos[i],(aux->direccionBase)+i);
		}
	}
}

void VerHuecos(){		
	
	if(list_size(TABLA_HUECOS) > 0){
		for(int i = 0; i<list_size(TABLA_HUECOS); i++){
			Hueco* aux = list_get(TABLA_HUECOS,i);

			log_info(Memoria_Logger,"direccion base es(HUECO): %p",aux->direccionBase);
			log_info(Memoria_Logger,"LIMITE(HUECO): %d",aux->limite);
		}
	}
}

void inicializarMemoria() {
	TABLA_SEGMENTOS=list_create();  // Establecer la cantidad de segmentos
	TABLA_HUECOS=list_create();  // Establecer la cantidad de segmentos

	Segmento* seg = malloc(sizeof(Segmento));

    // Asignar memoria para el espacio de usuario
    MEMORIA = (void *) malloc(TAM_MEMORIA);
    memset(MEMORIA, 0,TAM_MEMORIA);

    seg->PID=-1;
	seg->idSegmento=0;
	seg->direccionBase=MEMORIA;
	seg->limite=TAM_SEGMENTO_0;
	memset(seg->direccionBase, 0, seg->limite);
	list_add(TABLA_SEGMENTOS,seg);
	Hueco* hueco = malloc(sizeof(Hueco));
	hueco->direccionBase=seg->direccionBase + seg->limite;
	hueco->limite= TAM_MEMORIA - seg ->limite;
	list_add(TABLA_HUECOS,hueco);


}

//FALTA VALIDAR QUE SEA EL MISMO PID
int validarSegmento(int PID, int idSeg, int desplazamientoSegmento)
{
	//valida la existencia del segmento y checkea que el desplazamiento no tire seg_fault

	int aux=0;
	Segmento* seg=list_get(TABLA_SEGMENTOS,aux);

	while(list_size(TABLA_SEGMENTOS)>aux){
		if(seg->idSegmento==idSeg && seg->PID==PID)
		{
			if(seg->limite > desplazamientoSegmento) return 1;
			else return 0;
		}
		aux++;
		if(list_size(TABLA_SEGMENTOS)>aux)seg=list_get(TABLA_SEGMENTOS,aux);
	}

	return 0;
}

char* crearSegmento(int idSeg, int tamanoSegmento, int PID) {
    // Buscar espacio contiguo disponible para el segmento
    // utilizando el algoritmo de asignación especificado (FIRST, BEST, WORST)
	char* estado = validarMemoria(tamanoSegmento);

	printf("estado: %s\n",estado);

	if(strcmp(estado,"HUECO") == 0){
		switch(ALGORITMO_ASIGNACION){
			case 0:
				BestFit(idSeg,tamanoSegmento,PID);
				return "OK";
				break;
			case 1:
				WorstFit(idSeg,tamanoSegmento,PID);
				return "OK";
				break;
			case 2:
				FirstFit(idSeg,tamanoSegmento,PID);
				return "OK";
				break;
		}
	}
	/*else if(strcmp(estado, "CONTIGUO") == 0)
	{
		Segmento* lastSeg =list_get(TABLA_SEGMENTOS,list_size(TABLA_SEGMENTOS)-1);	
		AgregarSegmento(lastSeg,PID,tamanoSegmento,idSeg);
		return "OK";
	}*/

	return estado;
}

char* validarMemoria(int Tamano){
	//Segmento* lastSeg = list_get(TABLA_SEGMENTOS,0);
	//void* maxPos = lastSeg->direccionBase + lastSeg->limite;
	int TamSegmentos = 0;

	for(int i = 0; i < list_size(TABLA_SEGMENTOS); i++){
		Segmento* Seg = list_get(TABLA_SEGMENTOS,i);

		TamSegmentos += Seg->limite;	
	}

	/*if(maxPos + 1 + Tamano <= MEMORIA + TAM_MEMORIA)
		return "CONTIGUO";*/

		if(list_size(TABLA_HUECOS) != 0){
			for(int i = 0; i < list_size(TABLA_HUECOS); i++){
				Hueco* hueco = list_get(TABLA_HUECOS,i);

				if(Tamano <= hueco->limite)return "HUECO";

		}
		int EspacioLibre = (TAM_MEMORIA - TamSegmentos);
		printf("Espacio libre: %d\n",EspacioLibre);
		if(EspacioLibre >= Tamano)
			return "COMPACTAR";
		else
			return "OUT_OF_MEMORY";
	}
	return "OUT_OF_MEMORY";
}

void AgregarSegmento(Segmento* lastSeg,int PID,int tamanoSegmento,int idSeg){
	Segmento* seg = malloc(sizeof(Segmento));
	seg->PID = PID;
	seg->direccionBase=(lastSeg->direccionBase) + (lastSeg->limite);
	seg->limite=tamanoSegmento;
	seg->idSegmento=idSeg;
	list_add(TABLA_SEGMENTOS,seg);
	memset(seg->direccionBase,'\0',seg->limite);

	log_info(Memoria_Logger,"PID: %d - Crear Segmento: %d - Base: %p - TAMAÑO: %d",PID,idSeg,seg->direccionBase,tamanoSegmento);
}

char* eliminarSegmento(int idSegmento, int PID){
	log_info(Memoria_Logger,"buscando segmento");
	int indice = buscarSegmento(PID, idSegmento,false);
log_info(Memoria_Logger,"segmento encontrado");
	if(indice != -1) {
		Segmento* seg=list_get(TABLA_SEGMENTOS,indice);
		log_info(Memoria_Logger,"creando hueco");
		Hueco* hueco = malloc(sizeof(Hueco));
		hueco->direccionBase = seg->direccionBase;
		hueco->limite = seg->limite;
		bool agregado=false;
		
		for(int i = 0; i < list_size(TABLA_HUECOS); i++){
			Hueco* h = list_get(TABLA_HUECOS,i);

			if(h->direccionBase + h->limite == hueco->direccionBase){
				hueco->direccionBase = h->direccionBase;
				hueco->limite += h->limite;
				log_info(Memoria_Logger,"ENCONTRO ANTERIOR");
				list_remove(TABLA_HUECOS,i);
				list_add(TABLA_HUECOS,hueco);
				agregado=true;
			}
			else if(hueco->direccionBase + hueco->limite == h->direccionBase){
				hueco->limite += h->limite;
				hueco->direccionBase = h->direccionBase;
				log_info(Memoria_Logger,"ENCONTRO siguiente");
				list_remove(TABLA_HUECOS,i);
				list_add(TABLA_HUECOS,hueco);
				agregado=true;
			}
		}

		if(!agregado) list_add(TABLA_HUECOS,hueco);

		memset(seg->direccionBase,'\0',seg->limite);
		list_remove(TABLA_SEGMENTOS,indice);

		log_info(Memoria_Logger,"PID: %d - Eliminar Segmento: %d - Base: %p - TAMAÑO: %d",PID,idSegmento,seg->direccionBase,seg->limite);
		return "OK";
	}
	else 
		return "SEG_FAULT";	
}

char* leerSegmento(int PID, int IdSeg, int Offset, int Longitud, char* Remitente)
{
	printf("Lee segmento\n");
	int indice = buscarSegmento(PID, IdSeg,false);
	printf("indice: %d\n",indice);
	Segmento* seg=list_get(TABLA_SEGMENTOS,indice);
	
	log_info(Memoria_Logger,"PID: %d - Acción: LEER - Dirección física: %p - Tamaño: %d - Origen: %s",PID,seg->direccionBase,Longitud,Remitente);

	if(indice != -1)
	{
		printf("indice distinto de -1\n");
		char* contenido = malloc(Longitud);
		if(validarSegmento(PID, IdSeg, (Offset + Longitud)) == 1)
		{		
			printf("validar segmento = 1\n");
			char* segmento = (char*)seg->direccionBase;
			memcpy(contenido,(segmento + Offset),Longitud);
			contenido[Longitud] = '\0';
			return contenido;
		}
		else{
			printf("validar segmento != 1\n");
			return "SEG_FAULT";
		} 
	}
	printf("indice igual a -1\n");
	return "SEG_FAULT";
}

char* escribirSegmento(int PID, int IdSeg, int Offset, char* datos, char* Remitente){
	int indice = buscarSegmento(PID, IdSeg,false);
	Segmento* seg = list_get(TABLA_SEGMENTOS,indice);

	log_info(Memoria_Logger,"PID: %d - Acción: ESCRIBIR- Dirección física: %p - Tamaño: %ld - Origen: %s",PID,seg->direccionBase,strlen(datos),Remitente);
	
	if(indice != -1) {
		if(validarSegmento(PID, IdSeg, (Offset + strlen(datos))) == 1){
			
			char* segmento = (char*)seg->direccionBase;

			memcpy(segmento + Offset,datos,strlen(datos));

			return "OK\0";
		}
		else
			return "SEG_FAULT\0";
	}
	return "SEG_FAULT\0";
}

int buscarSegmento(int PID, int idSeg,bool Finish){
	//printf("Buscando segmento, finish: %d\n",Finish);
	int aux=0;
	Segmento* seg=list_get(TABLA_SEGMENTOS,aux);

	while(list_size(TABLA_SEGMENTOS)>aux){
		//printf("PID: %d - Segmento: %d - Segmento->PID: %d - Segmento->IdSegmento: %d \n",PID,idSeg,seg->PID,seg->idSegmento);
		if(Finish && seg->PID == PID) return aux;
		if(seg->idSegmento==idSeg && seg->PID == PID) return aux;
		aux++;
		if(list_size(TABLA_SEGMENTOS)>aux)seg=list_get(TABLA_SEGMENTOS,aux);
	}
	return -1;
}

void finalizarProceso(int PID){
	int indice = 0;
	while(indice != -1){
		indice = buscarSegmento(PID,0,true);
		if(indice != -1){
			Segmento* seg = list_get(TABLA_SEGMENTOS,indice); 
			eliminarSegmento(seg->idSegmento,PID);
		}
	}
}

void compactarSegmentos() {
    // Mover los segmentos para eliminar los huecos libres
	log_info(Memoria_Logger,"Solicitud de Compactación");
	t_list* tablaOrdenada= list_create();
	t_list* tablaSeg= list_duplicate(TABLA_SEGMENTOS);
	list_clean(TABLA_HUECOS);
	void* menorBase=MEMORIA+TAM_MEMORIA+1;
	int indice;
	int tamanioOcupado=0;
	Segmento* seg= malloc(sizeof(Segmento));
	Segmento* segAnterior= malloc(sizeof(Segmento));
	//SEGMENTO 0 no se modifica
	list_add(tablaOrdenada,list_get(TABLA_SEGMENTOS,0));

	//crea una nueva lista con los segmentos ordenados
	for(int j=1;j<list_size(TABLA_SEGMENTOS);j++){
		for(int i=0;i<list_size(tablaSeg);i++){
			seg=list_get(tablaSeg,i);
			if(menorBase>seg->direccionBase){
				menorBase = seg->direccionBase;
				indice=i+1;
			}
		}
		seg = list_remove(tablaSeg, indice);
//		log_info(Memoria_Logger,"agrego el segm %d",indice);
		list_add(tablaOrdenada,seg);
	}

    // Actualizar la tabla de segmentos ordenada con las nuevas direcciones base
	for(int i=1;i<list_size(tablaOrdenada);i++){
		seg=list_get(tablaOrdenada,i);
		tamanioOcupado+= seg->limite;
//		log_info(Memoria_Logger,"indices %d id%d PID%d",i,seg->idSegmento,seg->PID);
		segAnterior=list_get(tablaOrdenada,i-1);
		seg->direccionBase=segAnterior->direccionBase + segAnterior->limite + 1;
		list_replace(tablaOrdenada,i,seg);
		log_info(Memoria_Logger,"PID: %d - Cambiar base de Segmento: %d - Base: %p - TAMAÑO: %d",seg->PID,seg->idSegmento,seg->direccionBase,seg->limite);
	}

	Segmento* seg1 = malloc(sizeof(Segmento));
	seg1 = list_get(tablaOrdenada,(list_size(tablaOrdenada)-1));
	Hueco* hueco = malloc(sizeof(Hueco));

	hueco->direccionBase=seg1->direccionBase + seg1->limite;
	hueco->limite=  TAM_MEMORIA - tamanioOcupado;

	list_add(TABLA_HUECOS,hueco);
	TABLA_SEGMENTOS=list_duplicate(tablaOrdenada);
	list_destroy(tablaOrdenada);
	list_destroy(tablaSeg);

}

void BestFit(int idSeg, int tam, int PID){
	Hueco* MinHole = malloc(sizeof(Hueco));
	Hueco* h = malloc(sizeof(Hueco));
	int Diff = INT_MAX;
	int indice = 0;

	for(int i = 0; i < list_size(TABLA_HUECOS); i++){
		h = list_get(TABLA_HUECOS,i);

		if(h->limite >= tam && (h->limite - tam) < Diff){
			Diff = h->limite - tam;
			MinHole = h;
			indice = i;
		} 
	}

	Segmento* seg = malloc(sizeof(Segmento));
	seg->direccionBase = MinHole->direccionBase;
	seg->idSegmento = idSeg;
	seg->PID = PID;
	seg->limite = tam;

	int newHoleLimit = MinHole->limite - tam;

	if(newHoleLimit>0){
		MinHole->limite = newHoleLimit;
		MinHole->direccionBase = MinHole->direccionBase + tam + 1;

		list_remove(TABLA_HUECOS,indice);
		memset(MinHole->direccionBase,'\0',MinHole->limite);
		list_add(TABLA_HUECOS,MinHole);
	}
	else list_remove(TABLA_HUECOS,indice);

	list_add(TABLA_SEGMENTOS,seg);
	log_info(Memoria_Logger,"PID: %d - Crear Segmento: %d - Base: %p - TAMAÑO: %d",PID,idSeg,seg->direccionBase,tam);
	memset(seg->direccionBase,'\0',seg->limite);

}

void WorstFit(int idSeg, int tam, int PID){
	Hueco* MaxHole = malloc(sizeof(Hueco));
	Hueco* h = malloc(sizeof(Hueco));
	MaxHole->limite = 0;
	int indice = 0;

	for(int i = 0; i < list_size(TABLA_HUECOS); i++){

		h = list_get(TABLA_HUECOS,i);

		if(h->limite >= tam && h->limite > MaxHole->limite){
			MaxHole = h;
			indice = i;
		} 
	}

	Segmento* seg = malloc(sizeof(Segmento));
	seg->direccionBase = MaxHole->direccionBase;
	seg->idSegmento = idSeg;
	seg->PID = PID;
	seg->limite = tam;

	int newHoleLimit = MaxHole->limite - tam;

	if(newHoleLimit>0){
		MaxHole->limite = newHoleLimit;
		MaxHole->direccionBase = MaxHole->direccionBase + tam + 1;

		list_remove(TABLA_HUECOS,indice);
		memset(MaxHole->direccionBase,'\0',MaxHole->limite);
		list_add(TABLA_HUECOS,MaxHole);

	}else list_remove(TABLA_HUECOS,indice);

	list_add(TABLA_SEGMENTOS,seg);
	log_info(Memoria_Logger,"PID: %d - Crear Segmento: %d - Base: %p - TAMAÑO: %d",PID,idSeg,seg->direccionBase,tam);
	memset(seg->direccionBase,'\0',seg->limite);
}

void FirstFit(int idSeg, int tam, int PID){
	int aux = 0;

	Hueco* hueco= malloc(sizeof(Hueco));
	void* pos= ((Hueco*)list_get(TABLA_HUECOS,0))->direccionBase;
	int ind=0;
	//ordenamiento Huecos
	for(int j=1;j<list_size(TABLA_HUECOS);j++){
		hueco=list_get(TABLA_HUECOS,j);
				if(pos>hueco->direccionBase){
					pos = hueco->direccionBase;
					ind=j;
				}
		}

		Hueco* h = list_get(TABLA_HUECOS,ind);
		Hueco* newHole = malloc(sizeof(Hueco));


			Segmento* seg = malloc(sizeof(Segmento));
			seg->direccionBase = h->direccionBase;
			seg->idSegmento = idSeg;
			seg->PID = PID;
			seg->limite = tam;

			int newHoleLimit = h->limite - tam;

			if(newHoleLimit>0){
				newHole->limite = newHoleLimit;
				newHole->direccionBase = h->direccionBase + tam + 1;

				list_remove(TABLA_HUECOS,aux);
				memset(newHole->direccionBase,'\0',newHole->limite);
				list_add(TABLA_HUECOS,newHole);
			}
			else list_remove(TABLA_HUECOS,aux);		

			list_add(TABLA_SEGMENTOS,seg);
			memset(seg->direccionBase,'\0',seg->limite);
			log_info(Memoria_Logger,"PID: %d - Crear Segmento: %d - Base: %p - TAMAÑO: %d",PID,idSeg,seg->direccionBase,tam);
}

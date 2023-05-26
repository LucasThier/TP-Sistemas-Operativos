#include "../include/KernelMain.h"

t_log* Kernel_Logger;

int SocketKernel;
int SocketCPU;
int SocketMemoria;
int SocketFileSystem;
bool planificadorFIFO;

t_config* config;


void sighandler(int s) 
{
	TerminarModulo();
	exit(0);	
}

int main(int argc, char* argv[])
{
	signal(SIGINT, sighandler);
	
	LeerConfigs("Kernel.cfg"/*argv[1]*/);

	InicializarSemaforos();

	Kernel_Logger = log_create("Kernel.log", NOMBRE_PROCESO, true, LOG_LEVEL_INFO);

	if(InicializarPlanificadores() == 0)
		return EXIT_FAILURE;

	if(InicializarConexiones() == 0)
		return EXIT_FAILURE;
	//temporal
	while(true){
		log_info(Kernel_Logger, "hilo principal esta ejecutando");
		sleep(10);
	}

	TerminarModulo();
	return EXIT_SUCCESS;
}

//borrar ifs de todos los archivos
void LeerConfigs(char* path)
{
    config = config_create(path);

    IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");

    PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");

    IP_FILESYSTEM = config_get_string_value(config, "IP_FILESYSTEM");

    PUERTO_FILESYSTEM = config_get_string_value(config, "PUERTO_FILESYSTEM");

    IP_CPU = config_get_string_value(config, "IP_CPU");

    PUERTO_CPU = config_get_string_value(config, "PUERTO_CPU");

    PUERTO_ESCUCHA = config_get_string_value(config, "PUERTO_ESCUCHA");

	GRADO_MULTIPROGRAMACION = config_get_int_value(config, "GRADO_MAX_MULTIPROGRAMACION");

	ALGORITMO_PLANIFICACION = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
	
	ESTIMACION_INICIAL = config_get_int_value(config, "ESTIMACION_INICIAL");
	
	HRRN_ALFA = config_get_double_value(config, "HRRN_ALFA");
}

//Inicializa los semaforos
void InicializarSemaforos()
{
	sem_init(&m_PIDCounter, 0, 1);
	sem_init(&m_NEW, 0, 1);
	sem_init(&m_READY, 0, 1);
	sem_init(&m_EXEC, 0, 1);
	sem_init(&m_EXIT, 0, 1);
	sem_init(&m_BLOCKED, 0, 1);
	sem_init(&c_MultiProg, 0, GRADO_MULTIPROGRAMACION);
}

int InicializarPlanificadores()
{
	g_Lista_NEW = list_create();
	g_Lista_READY = list_create();
	g_EXEC = malloc(sizeof(t_PCB));
	g_EXEC = NULL;
	g_Lista_EXIT = list_create();
	g_Lista_BLOCKED = list_create();

	pthread_t HiloPlanificadorDeLargoPlazo;//FIFO
	if (pthread_create(&HiloPlanificadorDeLargoPlazo, NULL, PlanificadorLargoPlazo, NULL) != 0) {
		return 0;
	}

	planificadorFIFO = strcmp(ALGORITMO_PLANIFICACION, "HRRN") != 0;

	pthread_t HiloPlanificadorDeCortoPlazo;
	if (pthread_create(&HiloPlanificadorDeCortoPlazo, NULL, PlanificadorCortoPlazo, NULL)) {
		return 0;
	}

	return 1;
}

//Planificador de largo plazo FIFO (New -> Ready)
void* PlanificadorLargoPlazo()
{
	while(true)
	{
		//Si hay espacio en la cola de READY y hay procesos esperando en NEW,
		//obtener el primero de la cola de NEW y mandarlo a READY
		if(list_size(g_Lista_NEW) != 0)
		{
			sem_wait(&c_MultiProg);

			sem_wait(&m_NEW);
			sem_wait(&m_READY);
			t_PCB* PCB = (t_PCB*) list_remove(g_Lista_NEW, 0);
			AgregarAReady(PCB);
			sem_post(&m_NEW);
			sem_post(&m_READY);
		}
		sleep(2);
	}
}

//Planificador de corto plazo (Ready -> Exec)
void* PlanificadorCortoPlazo()
{	
	if(planificadorFIFO)
	{
		PlanificadorCortoPlazoFIFO();
	}
	else
	{
		PlanificadorCortoPlazoHRRN();
	}

	return (void*) EXIT_FAILURE;
}


void PlanificadorCortoPlazoFIFO()
{
	while(true)
	{
		//Si no hay ningun proceso ejecutando y hay procesos esperando en ready,
		//obtener el primero de la cola de READY y mandarlo a EXEC
		if(g_EXEC == NULL && list_size(g_Lista_READY) != 0){

			sem_wait(&m_READY);
			sem_wait(&m_EXEC);
			//obtener PCB de la cola de READY
			t_PCB* PCB = (t_PCB*) list_remove(g_Lista_READY, 0);
			//Agregar PCB a la cola de EXEC
			g_EXEC = PCB;
			sem_post(&m_READY);
			sem_post(&m_EXEC);

			//Enviar PCB a CPU
			Enviar_PCB_A_CPU(PCB);

			//Recibir respuesta de CPU
			printf("PID del proceso enviado a la CPU: %d\n", g_EXEC->PID);
			char* respuesta = (char*) recibir_paquete(SocketCPU);
			printf("Respuesta de CPU: %s\n", respuesta);
			
			RealizarRespuestaDelCPU(respuesta);
		}
	}
}

void PlanificadorCortoPlazoHRRN()
{
	while(true)
	{
		//Si no hay ningun proceso ejecutando y hay procesos esperando en ready,
		//Calcular el proximo que deberia ejecutar
		if(g_EXEC == NULL && list_size(g_Lista_READY) != 0){

			double RatioMasAlto = 0;
			int IndiceListaRatioMasAlto = 0;

			sem_wait(&m_READY);
			for(int i = 0; i < list_size(g_Lista_READY); i++)
			{				
				t_PCB* aux = (t_PCB*) list_get(g_Lista_READY, i);

				//si nunca ejecuto, entonces tomo la estimacion inicial
				if(aux->programCounter == 0 && RatioMasAlto < ESTIMACION_INICIAL)
				{
					RatioMasAlto = ESTIMACION_INICIAL;
					IndiceListaRatioMasAlto = i;
				}
				//si ya ejecuto al menos una vez entonces calculo el ratio
				else
				{
					//calculo el ratio
					double Ratio = (TiempoEsperadoEnReady(aux) + EstimacionProximaRafaga(aux)) / EstimacionProximaRafaga(aux);
					
					//guardo la estimacion de la ultima rafaga para la proxima que calcule el ratio
					aux->estimacionUltimaRafaga = EstimacionProximaRafaga(aux);

					//si el ratio es mas alto que el mas alto encontrado hasta ahora,
					// entonces lo reemplazo por este
					if(Ratio > RatioMasAlto)
					{
						RatioMasAlto = Ratio;
						IndiceListaRatioMasAlto = i;
					}
				}
			}

			sem_wait(&m_EXEC);
			//obtener PCB de la cola de READY
			t_PCB* PCB = (t_PCB*) list_remove(g_Lista_READY, IndiceListaRatioMasAlto);
			//Agregar PCB a la cola de EXEC
			g_EXEC = PCB;
			sem_post(&m_READY);
			sem_post(&m_EXEC);

			//Enviar PCB a CPU
			Enviar_PCB_A_CPU(PCB);

			//Recibir respuesta de CPU
			printf("PID del proceso enviado a la CPU: %d\n", g_EXEC->PID);
			char* respuesta = (char*) recibir_paquete(SocketCPU);
			printf("Respuesta de CPU: %s \n", respuesta);
			
			RealizarRespuestaDelCPU(respuesta);
		}
	}
}

//devuelve el tiempo que paso desde que el proceso llego a ready
double TiempoEsperadoEnReady(t_PCB * PCB)
{
	return difftime(time(NULL), PCB->tiempoLlegadaRedy);
}

//calcula y devuelve la estimacion de la proxima rafaga de un PCB
double EstimacionProximaRafaga(t_PCB* PCB)
{
	return (HRRN_ALFA * PCB->tiempoUltimaRafaga) + ((1 - HRRN_ALFA) * PCB->estimacionUltimaRafaga);
}


void Enviar_PCB_A_CPU(t_PCB* PCB_A_ENVIAR)
{
	t_paquete* Paquete_PCB_Actual = crear_paquete(CPU_PCB);

	//Agrega el Program Counter
	agregar_a_paquete(Paquete_PCB_Actual, &(g_EXEC->programCounter), sizeof(int));

	//Agrega los registros de la CPU
	agregar_a_paquete(Paquete_PCB_Actual, &(g_EXEC->registrosCPU->AX), sizeof(g_EXEC->registrosCPU->AX));
	agregar_a_paquete(Paquete_PCB_Actual, &(g_EXEC->registrosCPU->BX), sizeof(g_EXEC->registrosCPU->BX));
	agregar_a_paquete(Paquete_PCB_Actual, &(g_EXEC->registrosCPU->CX), sizeof(g_EXEC->registrosCPU->CX));
	agregar_a_paquete(Paquete_PCB_Actual, &(g_EXEC->registrosCPU->DX), sizeof(g_EXEC->registrosCPU->DX));

	agregar_a_paquete(Paquete_PCB_Actual, &(g_EXEC->registrosCPU->EAX), sizeof(g_EXEC->registrosCPU->EAX));
	agregar_a_paquete(Paquete_PCB_Actual, &(g_EXEC->registrosCPU->EBX), sizeof(g_EXEC->registrosCPU->EBX));
	agregar_a_paquete(Paquete_PCB_Actual, &(g_EXEC->registrosCPU->ECX), sizeof(g_EXEC->registrosCPU->ECX));
	agregar_a_paquete(Paquete_PCB_Actual, &(g_EXEC->registrosCPU->EDX), sizeof(g_EXEC->registrosCPU->EDX));

	agregar_a_paquete(Paquete_PCB_Actual, &(g_EXEC->registrosCPU->RAX), sizeof(g_EXEC->registrosCPU->RAX));
	agregar_a_paquete(Paquete_PCB_Actual, &(g_EXEC->registrosCPU->RBX), sizeof(g_EXEC->registrosCPU->RBX));
	agregar_a_paquete(Paquete_PCB_Actual, &(g_EXEC->registrosCPU->RCX), sizeof(g_EXEC->registrosCPU->RCX));
	agregar_a_paquete(Paquete_PCB_Actual, &(g_EXEC->registrosCPU->RDX), sizeof(g_EXEC->registrosCPU->RDX));

	//Agrega las instrucciones
	//como pueden ser n instrucciones, las agrego ultimas;
	for(int i=0; i < list_size(g_EXEC->listaInstrucciones); i++)
	{
		char* instruccion = (char*) list_get(g_EXEC->listaInstrucciones, i);
					
		agregar_a_paquete(Paquete_PCB_Actual, instruccion, strlen(instruccion)+1);
	}

	enviar_paquete(Paquete_PCB_Actual, SocketCPU);
	eliminar_paquete(Paquete_PCB_Actual);
}

//agrega un PCB a la cola de READY y actualiza su tiempo de llegada a ready,
// asume que ya se hicieron los wait y signal correspondientes
void AgregarAReady(t_PCB* PCB)
{
	PCB->tiempoLlegadaRedy = time(NULL);
	list_add(g_Lista_READY, PCB);
}

//Realiza la Accion correspondiente a la respuesta del CPU
void RealizarRespuestaDelCPU(char* respuesta)
{
	if(strcmp(respuesta, "YIELD\n")== 0)
	{
		Recibir_Y_Actualizar_PCB();

		//Agregar PCB a la cola de READY
		sem_wait(&m_READY);
		sem_wait(&m_EXEC);
		AgregarAReady(g_EXEC);
		g_EXEC = NULL;
		sem_post(&m_READY);
		sem_post(&m_EXEC);
	}

	else if(strcmp(respuesta, "EXIT\n")== 0)
	{
		Recibir_Y_Actualizar_PCB();

		//Notificar a la consola que el proceso termino exitosamente
		t_paquete* MSGFinalizacion = crear_paquete(MENSAGE);
		agregar_a_paquete(MSGFinalizacion, "Proceso finalizo correctamente\n", strlen("Proceso finalizo correctamente")+1);
		enviar_paquete(MSGFinalizacion, g_EXEC->socketConsolaAsociada);
		eliminar_paquete(MSGFinalizacion);

		//imprimir los valores del registro
		ImprimirRegistrosPCB(g_EXEC);

		//Agregar PCB a la cola de EXIT
		sem_wait(&m_EXEC);
		sem_wait(&m_EXIT);
		list_add(g_Lista_EXIT, g_EXEC);
		g_EXEC = NULL;
		sem_post(&m_EXEC);
		sem_post(&m_EXIT);

		sem_post(&c_MultiProg);		
	}
}

void ImprimirRegistrosPCB(t_PCB* PCB_Registos_A_Imprimir)
{
	printf("Valores de los registros del proceso PID: %d\n", PCB_Registos_A_Imprimir->PID);
	printf("Valor de AX: %.*s\n", (int) sizeof(PCB_Registos_A_Imprimir->registrosCPU->AX), PCB_Registos_A_Imprimir->registrosCPU->AX);
	printf("Valor de BX: %.*s\n", (int) sizeof(PCB_Registos_A_Imprimir->registrosCPU->BX), PCB_Registos_A_Imprimir->registrosCPU->BX);
	printf("Valor de CX: %.*s\n", (int) sizeof(PCB_Registos_A_Imprimir->registrosCPU->CX), PCB_Registos_A_Imprimir->registrosCPU->CX);
	printf("Valor de DX: %.*s\n", (int) sizeof(PCB_Registos_A_Imprimir->registrosCPU->DX), PCB_Registos_A_Imprimir->registrosCPU->DX);
	printf("Valor de EAX: %.*s\n", (int) sizeof(PCB_Registos_A_Imprimir->registrosCPU->EAX), PCB_Registos_A_Imprimir->registrosCPU->EAX);
	printf("Valor de EBX: %.*s\n", (int) sizeof(PCB_Registos_A_Imprimir->registrosCPU->EBX), PCB_Registos_A_Imprimir->registrosCPU->EBX);
	printf("Valor de ECX: %.*s\n", (int) sizeof(PCB_Registos_A_Imprimir->registrosCPU->ECX), PCB_Registos_A_Imprimir->registrosCPU->ECX);
	printf("Valor de RAX: %.*s\n", (int) sizeof(PCB_Registos_A_Imprimir->registrosCPU->RAX), PCB_Registos_A_Imprimir->registrosCPU->RAX);
	printf("Valor de RBX: %.*s\n", (int) sizeof(PCB_Registos_A_Imprimir->registrosCPU->RBX), PCB_Registos_A_Imprimir->registrosCPU->RBX);
	printf("Valor de RCX: %.*s\n", (int) sizeof(PCB_Registos_A_Imprimir->registrosCPU->RCX), PCB_Registos_A_Imprimir->registrosCPU->RCX);
	printf("Valor de RDX: %.*s\n", (int) sizeof(PCB_Registos_A_Imprimir->registrosCPU->RDX), PCB_Registos_A_Imprimir->registrosCPU->RDX);
}

//recibe el paquete conteniendo el nuevo pcb y lo actualiza
//!!SOLO ACTUALIZA EL PCB DEL PROCESO EN EJECUCION!!
void Recibir_Y_Actualizar_PCB()
{
	t_list* PCB_A_Actualizar = recibir_paquete(SocketCPU);
	g_EXEC->programCounter = *(int*) list_remove(PCB_A_Actualizar, 0);
	g_EXEC->registrosCPU = ObtenerRegistrosDelPaquete(PCB_A_Actualizar);
	g_EXEC->tiempoUltimaRafaga = *(double*) list_remove(PCB_A_Actualizar, 0);
}

t_registrosCPU* ObtenerRegistrosDelPaquete(t_list* Lista)
{
	t_registrosCPU* Registros = malloc(sizeof(t_registrosCPU));

	strncpy(Registros->AX, (char*)list_remove(Lista, 0), sizeof(Registros->AX));
	strncpy(Registros->BX, (char*)list_remove(Lista, 0), sizeof(Registros->BX));
	strncpy(Registros->CX, (char*)list_remove(Lista, 0), sizeof(Registros->CX));
	strncpy(Registros->DX, (char*)list_remove(Lista, 0), sizeof(Registros->DX));

	strncpy(Registros->EAX, (char*)list_remove(Lista, 0), sizeof(Registros->EAX));
	strncpy(Registros->EBX, (char*)list_remove(Lista, 0), sizeof(Registros->EBX));
	strncpy(Registros->ECX, (char*)list_remove(Lista, 0), sizeof(Registros->ECX));
	strncpy(Registros->EDX, (char*)list_remove(Lista, 0), sizeof(Registros->EDX));
	
	strncpy(Registros->RAX, (char*)list_remove(Lista, 0), sizeof(Registros->RAX));
	strncpy(Registros->RBX, (char*)list_remove(Lista, 0), sizeof(Registros->RBX));
	strncpy(Registros->RCX, (char*)list_remove(Lista, 0), sizeof(Registros->RCX));
	strncpy(Registros->RDX, (char*)list_remove(Lista, 0), sizeof(Registros->RDX));
	
	return Registros;
}


//Conecta con los modulos (CPU, FS, Mem) y luego crea un Hilo que escucha conexiones de consolas
int InicializarConexiones()
{
	//Conectar con los modulos
	SocketCPU = conectar_servidor(Kernel_Logger, "CPU", IP_CPU, PUERTO_CPU);
	SocketMemoria = conectar_servidor(Kernel_Logger, "FileSystem", IP_MEMORIA, PUERTO_MEMORIA);
	SocketFileSystem = conectar_servidor(Kernel_Logger, "Memoria", IP_FILESYSTEM, PUERTO_FILESYSTEM);

	printf("inicializar conexiones \n");

	//Crear hilo escucha
	pthread_t HiloEscucha;
    if (pthread_create(&HiloEscucha, NULL, EscucharConexiones, NULL) != 0) {
		printf("Error creando hilo escucha \n");
        return 0;
    }
	return 1;
}

//Inicia un servidor en el que escucha consolas permanentemente y crea un hilo que la administre cuando recibe una
void* EscucharConexiones()
{
	printf("Escuchar Conexiones\n");
	SocketKernel = iniciar_servidor(Kernel_Logger, NOMBRE_PROCESO, "0.0.0.0", PUERTO_ESCUCHA);
	
	if(SocketKernel != 0)
	{
		//Escuchar por consolas en bucle
		while(true)
		{
			printf("Creando socket Cliente\n");
			int SocketCliente = esperar_cliente(Kernel_Logger, NOMBRE_PROCESO, SocketKernel);
			
			if(SocketCliente != 0)
			{	
				//Crea un hilo para la consola conectada
				pthread_t HiloAdministradorDeMensajes;
				if (pthread_create(&HiloAdministradorDeMensajes, NULL, AdministradorDeModulo, (void*)&SocketCliente) != 0) {
					exit(EXIT_FAILURE);
				}
			}
		}
	}

	printf("error al crear socket kernel\n");
	liberar_conexion(SocketKernel);
	return (void*)EXIT_FAILURE;
}

//Funcion que se ejecuta para cada consola conectada
void* AdministradorDeModulo(void* arg)
{
	int* SocketClienteConectado = (int*)arg;

	//--------------------------------------------------------------------------------------------------------------------------------------------------
	//Acciones a realizar para cada consola conectado:
	 
	t_list* instruccionesRecibidas = (t_list*)recibir_paquete(*SocketClienteConectado);

	printf("Cantidad de instrucciones recibidas: %d\n", list_size(instruccionesRecibidas));

	//itera la lista y hace lo que este en la funcion para cada elemento
	list_iterate(instruccionesRecibidas, (void*) ForEach_Instrucciones);

	t_PCB* PCBConsola = CrearPCB(instruccionesRecibidas, *SocketClienteConectado);

	//Verificar si el grado de multiprogramacion lo permite, si lo permite, pasar a ready directamente
	if(sem_trywait(&c_MultiProg) == 0)
	{
		printf("agregando a ready ya que el grado de multiprogramacion lo permite");
		sem_wait(&m_READY);
		list_add(g_Lista_READY, PCBConsola);
		sem_post(&m_READY);
	}
	//pasar a new
	else
	{
		printf("agregando a new ya que el grado de multiprogramacion no lo permite");
		sem_wait(&m_NEW);
		list_add(g_Lista_NEW, PCBConsola);
		sem_post(&m_NEW);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------------------
	return NULL;
}

//Repite el contenido de la funcion para cada instruccion en la lista
void ForEach_Instrucciones(char* value)
{
	log_info(Kernel_Logger,"Instruccion Recibida: %s", value);
}

//Crea un PCB con los datos recibidos
t_PCB* CrearPCB(t_list* instrucciones, int socketConsola)
{
	t_PCB* pcb = malloc(sizeof(t_PCB));

	sem_wait(&m_PIDCounter);
	pcb->PID = g_PIDCounter;
	g_PIDCounter++;
	sem_post(&m_PIDCounter);

	pcb->registrosCPU = CrearRegistrosCPU();
	pcb->programCounter = 0;
	pcb->socketConsolaAsociada = socketConsola;
	pcb->listaInstrucciones = instrucciones;

	return pcb;
}

t_registrosCPU* CrearRegistrosCPU()
{
	t_registrosCPU* registrosCPU = malloc(sizeof(t_registrosCPU));
	
	strncpy(registrosCPU->AX, "\0\0\0\0", sizeof(registrosCPU->AX));
	strncpy(registrosCPU->BX, "\0\0\0\0", sizeof(registrosCPU->BX));
	strncpy(registrosCPU->CX, "\0\0\0\0", sizeof(registrosCPU->CX));
	strncpy(registrosCPU->DX, "\0\0\0\0", sizeof(registrosCPU->DX));

	strncpy(registrosCPU->EAX, "\0\0\0\0\0\0\0\0", sizeof(registrosCPU->EAX));
	strncpy(registrosCPU->EBX, "\0\0\0\0\0\0\0\0", sizeof(registrosCPU->EBX));
	strncpy(registrosCPU->ECX, "\0\0\0\0\0\0\0\0", sizeof(registrosCPU->ECX));
	strncpy(registrosCPU->EDX, "\0\0\0\0\0\0\0\0", sizeof(registrosCPU->EDX));
	
	strncpy(registrosCPU->RAX, "\0\0\0\0\0\0\0\0\0\0\0\0", sizeof(registrosCPU->RAX));
	strncpy(registrosCPU->RBX, "\0\0\0\0\0\0\0\0\0\0\0\0", sizeof(registrosCPU->RBX));
	strncpy(registrosCPU->RCX, "\0\0\0\0\0\0\0\0\0\0\0\0", sizeof(registrosCPU->RCX));
	strncpy(registrosCPU->RDX, "\0\0\0\0\0\0\0\0\0\0\0\0", sizeof(registrosCPU->RDX));

	return registrosCPU;
}

//Funcion que se ejecuta al recibir una señal de finalizacion de programa (Ctrl+C)
void TerminarModulo()
{
	liberar_conexion(SocketKernel);
	liberar_conexion(SocketCPU);
	liberar_conexion(SocketMemoria);
	liberar_conexion(SocketFileSystem);

	log_destroy(Kernel_Logger);

	config_destroy(config);

	sem_destroy(&m_PIDCounter);
	sem_destroy(&c_MultiProg);
	sem_destroy(&m_NEW);
	sem_destroy(&m_READY);
	sem_destroy(&m_EXEC);
	sem_destroy(&m_EXIT);
	sem_destroy(&m_BLOCKED);

	exit(EXIT_SUCCESS);
}
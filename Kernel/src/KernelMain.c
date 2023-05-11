#include "../include/KernelMain.h"

t_log* Kernel_Logger;

int SocketKernel;
int SocketCPU;
int SocketMemoria;
int SocketFileSystem;

t_config* config;


void sighandler(int s) 
{
	liberar_conexion(SocketKernel);
	exit(0);	
}

int main(int argc, char* argv[])
{
	signal(SIGINT, sighandler);
	Kernel_Logger = log_create("Kernel.log", NOMBRE_PROCESO, true, LOG_LEVEL_INFO);

	//leer las config
	LeerConfigs(argv[1]);

	InicializarConexiones();

	//temporal
	while(true){
		log_info(Kernel_Logger, "hilo principal esta ejecutando");
		sleep(10);
	}

	liberar_conexion(SocketKernel);
}

//Conecta con los modulos (CPU, FS, Mem) y luego crea un Hilo que escucha conexiones de consolas
int InicializarConexiones()
{
	//Conectar con los modulos
	SocketCPU = conectar_servidor(Kernel_Logger, "CPU", IP_CPU, PUERTO_CPU);
	SocketMemoria = conectar_servidor(Kernel_Logger, "FileSystem", IP_MEMORIA, PUERTO_MEMORIA);
	SocketFileSystem = conectar_servidor(Kernel_Logger, "Memoria", IP_FILESYSTEM, PUERTO_FILESYSTEM);

	//Crear hilo escucha
	pthread_t HiloEscucha;
    if (pthread_create(&HiloEscucha, NULL, EscucharConexiones, NULL) != 0) {
        exit(EXIT_FAILURE);
    }
	return 0;
}

//Inicia un servidor en el que escucha consolas permanentemente y crea un hilo que la administre cuando recibe una
void* EscucharConexiones()
{
	SocketKernel = iniciar_servidor(Kernel_Logger, NOMBRE_PROCESO, "0.0.0.0", PUERTO_ESCUCHA);
	
	if(SocketKernel != 0)
	{
		//Escuchar por consolas en bucle
		while(true)
		{
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

	liberar_conexion(SocketKernel);
	return (void*)EXIT_FAILURE;
}

//Funcion que se ejecuta para cada consola conectada
void* AdministradorDeModulo(void* arg)
{
	int* SocketClienteConectado = (int*)arg;

	//--------------------------------------------------------------------------------------------------------------------------------------------------
	//Acciones a realizar para cada consola conectado:
	//creo el PCB y meto el socket y las instrucciones a la lista de new PCB
	//y si el grado de multiprogramacion lo permite, lo paso a ready;
	//crear listas globales con mutex para cada estado de los procesos
	 
	t_list* instruccionesRecibidas = recibir_paquete(*SocketClienteConectado);

	printf("Cantidad de instrucciones recibidas: %d\n", list_size(instruccionesRecibidas));

	//las dos hacen lo mismo, ver cual es la menos confusa para el resto----------------------------------
	list_iterate(instruccionesRecibidas, (void*) ForEach_Instrucciones);

	for(int i=0; i < list_size(instruccionesRecibidas); i++)
	{
		log_info(Kernel_Logger, "Instruccion recibida: %s", (char*)list_get(instruccionesRecibidas, i));
	}
	//Ver Cual es la menos confusa para el resto^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	eliminar_paquete(instruccionesRecibidas);
	//--------------------------------------------------------------------------------------------------------------------------------------------------
	liberar_conexion(*SocketClienteConectado);
	return NULL;
}

//Repite en contenido de la funcion para cada instruccion en la lista
void ForEach_Instrucciones(char* value)
{
	log_info(Kernel_Logger,"Instruccion Recibida: %s", value);
}

//borrar ifs de todos los archivos
void LeerConfigs(char* path)
{
    config = config_create(path);

    if(config_has_property(config, "IP_MEMORIA"))
        IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");

    if(config_has_property(config, "PUERTO_MEMORIA"))
        PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");

    if(config_has_property(config, "IP_FILESYSTEM"))
        IP_FILESYSTEM = config_get_string_value(config, "IP_FILESYSTEM");

    if(config_has_property(config, "PUERTO_FILESYSTEM"))
        PUERTO_FILESYSTEM = config_get_string_value(config, "PUERTO_FILESYSTEM");

    if(config_has_property(config, "IP_CPU"))
        IP_CPU = config_get_string_value(config, "IP_CPU");

    if(config_has_property(config, "PUERTO_CPU"))
        PUERTO_CPU = config_get_string_value(config, "PUERTO_CPU");

    if(config_has_property(config, "PUERTO_ESCUCHA"))
        PUERTO_ESCUCHA = config_get_string_value(config, "PUERTO_ESCUCHA");
}
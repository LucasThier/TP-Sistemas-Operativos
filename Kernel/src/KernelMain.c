#include "../include/KernelMain.h"

t_log* Kernel_Logger;

typedef void (*t_CallbackAdministradorMensajes)(void*);

typedef struct
{
	t_CallbackAdministradorMensajes Callback;
	t_log* logger;
	int SocketConectado;
	//Resto de parametros

} t_ArgsConexiones;


int SocketKernel;
int SocketCPU;
int SocketMemoria;
int SocketMemoria;

void sighandler(int s) {
	liberar_conexion(SocketKernel);
	exit(0);
}

int main(void)
{
	signal(SIGINT, sighandler);
	Kernel_Logger = log_create("Kernel.log", NOMBRE_PROCESO, true, LOG_LEVEL_INFO);

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
	SocketCPU = conectar_servidor(Kernel_Logger, "CPU", "0.0.0.0", "35001");
	SocketMemoria = conectar_servidor(Kernel_Logger, "FileSystem", "0.0.0.0", "35002");
	SocketMemoria = conectar_servidor(Kernel_Logger, "Memoria", "0.0.0.0", "35003");

	//Crear hilo escucha
	pthread_t HiloEscucha;
    if (pthread_create(&HiloEscucha, NULL, EscucharConexiones, NULL) != 0) {
        exit(EXIT_FAILURE);
    }
}

//Inicia un servidor en el que escucha consolas permanentemente y crea un hilo que la administre cuando recibe una
void* EscucharConexiones()
{
	SocketKernel = iniciar_servidor(Kernel_Logger, NOMBRE_PROCESO, "0.0.0.0", "33668");
	
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
	return EXIT_FAILURE;
}

//Funcion que se ejecuta para cada consola conectada
void* AdministradorDeModulo(void* arg)
{
	int* SocketClienteConectado = (int*)arg;

	//--------------------------------------------------------------------------------------------------------------------------------------------------
	//Acciones a realizar para cada consola conectado:

	int recibido;
	do
	{
		recibido = recibir_int(*SocketClienteConectado);
		log_info(Kernel_Logger, "Numero recibido: %d\n", recibido);
		enviar_int(Kernel_Logger, NOMBRE_PROCESO, SocketCPU, recibido);
		enviar_int(Kernel_Logger, NOMBRE_PROCESO, SocketMemoria, recibido);
		enviar_int(Kernel_Logger, NOMBRE_PROCESO, SocketMemoria, recibido);

	}while(recibido != 0);

	//--------------------------------------------------------------------------------------------------------------------------------------------------
	liberar_conexion(SocketClienteConectado);
	return;
}
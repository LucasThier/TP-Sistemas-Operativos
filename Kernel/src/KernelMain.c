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
int SocketFileSystem;

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


int InicializarConexiones()
{
	SocketCPU = conectar_servidor(Kernel_Logger, "CPU", "0.0.0.0", "35001");

	pthread_t HiloAdministradorDeConexiones;

    if (pthread_create(&HiloAdministradorDeConexiones, NULL, AdministradorDeConexiones, NULL) != 0) {
        exit(EXIT_FAILURE);
    }

}


void* AdministradorDeConexiones()
{
	SocketKernel = iniciar_servidor(Kernel_Logger, NOMBRE_PROCESO, "0.0.0.0", "33668");
	
	if(SocketKernel != 0)
	{
		while(true)
		{
			int SocketCliente = esperar_cliente(Kernel_Logger, NOMBRE_PROCESO, SocketKernel);

			if(SocketCliente != 0)
			{	
				pthread_t HiloAdministradorDeMensajes;
				if (pthread_create(&HiloAdministradorDeMensajes, NULL, AdministradorDeMensajes, (void*)&SocketCliente) != 0) {
					exit(EXIT_FAILURE);
				}
			}
		}
	}

	liberar_conexion(SocketKernel);
	return EXIT_FAILURE;
}

void* AdministradorDeMensajes(void* arg)
{
	int* SocketClienteConectado = (int*)arg;

	//Acciones a realizar para cada consola conectado
	int recibido;
	do
	{
		recibido = recibir_int(*SocketClienteConectado);
		log_info(Kernel_Logger, "Numero recibido: %d\n", recibido);
		enviar_int(Kernel_Logger, NOMBRE_PROCESO, SocketCPU, recibido);
	}while(recibido != 0);

	liberar_conexion(SocketClienteConectado);
	return;
}
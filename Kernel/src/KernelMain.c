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


int SocketServidor;

void sighandler(int s) {
	liberar_conexion(SocketServidor);
	exit(0);
}

int main(void)
{
	signal(SIGINT, sighandler);

	Kernel_Logger = log_create("Kernel.log", NOMBRE_PROCESO, true, LOG_LEVEL_INFO);

	InicializarConexiones();

	while(true){
		log_info(Kernel_Logger, "hilo principal esta ejecutando");
		sleep(10);
	}

	liberar_conexion(SocketServidor);
}


int InicializarConexiones()
{
	pthread_t HiloAdministradorDeConexiones;

    if (pthread_create(&HiloAdministradorDeConexiones, NULL, AdministradorDeConexiones, NULL) != 0) {
        exit(EXIT_FAILURE);
    }
}


void* AdministradorDeConexiones()
{
	SocketServidor = iniciar_servidor(Kernel_Logger, NOMBRE_PROCESO, "0.0.0.0", "33668");
	
	if(SocketServidor != 0)
	{
		while(true)
		{
			log_info(Kernel_Logger, "hilo CONEXIONES esta ejecutando");

			int SocketCliente = esperar_cliente(Kernel_Logger, NOMBRE_PROCESO, SocketServidor);

			if(SocketCliente != 0)
			{	
				printf("Socket del cliente 1 %d \n", SocketCliente);

				pthread_t HiloAdministradorDeMensajes;
				if (pthread_create(&HiloAdministradorDeMensajes, NULL, AdministradorDeMensajes, (void*)&SocketCliente) != 0) {
					exit(EXIT_FAILURE);
				}
			}
		}
	}

	liberar_conexion(SocketServidor);
	return EXIT_FAILURE;
}

void* AdministradorDeMensajes(void* arg)
{
	int* SocketClienteConectado = (int*)arg;
	printf("\nSocket del cliente %d \n", *SocketClienteConectado);

	//Acciones a realizar para cada cliente conectado
	
	//Ejemplo:
	int recibido = 100;
	do
	{
		log_info(Kernel_Logger, "hilo MENSAJES esta ejecutando");
		sleep(5);
		recibido = recibir_int(*SocketClienteConectado);
		log_info(Kernel_Logger, "Numero recibido: %d\n", recibido);
	}while(recibido != 0);
	//Fin ejemplo

	liberar_conexion(SocketClienteConectado);
	return;
}
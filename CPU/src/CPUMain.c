#include "../include/CPUMain.h"


t_log* CPU_Logger;
int SocketCPU;

void sighandler(int s) {
	liberar_conexion(SocketCPU);
	exit(0);
}


int main(void)
{
	signal(SIGINT, sighandler);

	CPU_Logger = log_create("CPU.log", NOMBRE_PROCESO, true, LOG_LEVEL_INFO);
	
	InicializarConexiones();

	while(true){
		log_info(CPU_Logger, "hilo principal esta ejecutando");
		sleep(10);
	}

	return EXIT_SUCCESS;
}

int InicializarConexiones()
{
	pthread_t HiloAdministradorDeConexiones;

    if (pthread_create(&HiloAdministradorDeConexiones, NULL, AdministradorDeConexion, NULL) != 0) {
        exit(EXIT_FAILURE);
    }
}



void* AdministradorDeConexion()
{
	SocketCPU = iniciar_servidor(CPU_Logger, NOMBRE_PROCESO, "0.0.0.0", "35001");
	
	if(SocketCPU != 0)
	{
		int SocketKernel = esperar_cliente(CPU_Logger, NOMBRE_PROCESO, SocketCPU);

		if(SocketKernel != 0)
		{	
			//Acciones a realizar para cada consola conectado
			int recibido;
			do
			{
				recibido = recibir_int(SocketKernel);
				log_info(CPU_Logger, "Numero recibido: %d\n", recibido);
			}while(recibido != 0);

			liberar_conexion(SocketKernel);
			return;
		}
	}
	liberar_conexion(SocketCPU);
	return EXIT_FAILURE;
}

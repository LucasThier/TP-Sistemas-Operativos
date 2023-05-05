#include "../include/FileSystemMain.h"

t_log* FS_Logger;

int SocketFileSystem;
int SocketMemoria;


void sighandler(int s) {
	liberar_conexion(SocketFileSystem);
	exit(0);
}


int main(void)
{
	signal(SIGINT, sighandler);

	FS_Logger = log_create("FileSystem.log", NOMBRE_PROCESO, true, LOG_LEVEL_INFO);
	
	InicializarConexiones();

	while(true){
		log_info(FS_Logger, "hilo principal esta ejecutando");
		sleep(10);
	}

	return EXIT_SUCCESS;
}

//Crea un hilo de escucha para el kernel
int InicializarConexiones()
{
	SocketMemoria = conectar_servidor(FS_Logger, "Memoria", "0.0.0.0", "35003");

	pthread_t HiloEscucha;

    if (pthread_create(&HiloEscucha, NULL, EscuchaKernel, NULL) != 0) {
        exit(EXIT_FAILURE);
    }
}

//Crea un servidor y espera al kernel, luego recibe mensajes del mismo
void* EscuchaKernel()
{
	SocketFileSystem = iniciar_servidor(FS_Logger, NOMBRE_PROCESO, "0.0.0.0", "35002");
	
	if(SocketFileSystem != 0)
	{
		int SocketKernel = esperar_cliente(FS_Logger, NOMBRE_PROCESO, SocketFileSystem);

		if(SocketKernel != 0)
		{	
			//--------------------------------------------------------------------------------------------------------------------------------------------------
			//Acciones a realizar cuando se conecta el kernel:
			
			int recibido;
			do
			{
				recibido = recibir_int(SocketKernel);
				log_info(FS_Logger, "Numero recibido: %d\n", recibido);
			}while(recibido != 0);

			//--------------------------------------------------------------------------------------------------------------------------------------------------
			liberar_conexion(SocketKernel);
			return;
		}
	}
	liberar_conexion(SocketFileSystem);
	return EXIT_FAILURE;
}
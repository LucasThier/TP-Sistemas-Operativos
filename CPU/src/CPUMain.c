#include "../include/CPUMain.h"


t_log* Memoria_Logger;

int SocketCPU;
int SocketMemoria;

void sighandler(int s) {
	liberar_conexion(SocketCPU);
	exit(0);
}


int main(int argc, char* argv[])
{
	signal(SIGINT, sighandler);

	LeerConfigs(argv[1]);

	Memoria_Logger = log_create("CPU.log", NOMBRE_PROCESO, true, LOG_LEVEL_INFO);
	
	InicializarConexiones();

	while(true){
		log_info(Memoria_Logger, "hilo principal esta ejecutando");
		sleep(10);
	}

	return EXIT_SUCCESS;
}

//Crea un hilo de escucha para el kernel
int InicializarConexiones()
{
	SocketMemoria = conectar_servidor(Memoria_Logger, "Memoria", IP_MEMORIA, PUERTO_MEMORIA);

	pthread_t HiloEscucha;

    if (pthread_create(&HiloEscucha, NULL, EscuchaKernel, NULL) != 0) {
        exit(EXIT_FAILURE);
    }
	return 1;
}


//Crea un servidor y espera al kernel, luego recibe mensajes del mismo
void* EscuchaKernel()
{
	SocketCPU = iniciar_servidor(Memoria_Logger, NOMBRE_PROCESO, "0.0.0.0", PUERTO_ESCUCHA);
	
	if(SocketCPU != 0)
	{
		int SocketKernel = esperar_cliente(Memoria_Logger, NOMBRE_PROCESO, SocketCPU);

		if(SocketKernel != 0)
		{
			//--------------------------------------------------------------------------------------------------------------------------------------------------
			//Acciones a realizar cuando se conecta el kernel:

			int recibido;
			do
			{
				recibido = recibir_int(SocketKernel);
				log_info(Memoria_Logger, "Numero recibido: %d\n", recibido);
			}while(recibido != 0);

			//--------------------------------------------------------------------------------------------------------------------------------------------------
			liberar_conexion(SocketKernel);
			return (void*)EXIT_SUCCESS;
		}
	}
	liberar_conexion(SocketCPU);
	return (void*)EXIT_FAILURE;
}

void LeerConfigs(char* path)
{	
    config = config_create(path);

    if(config_has_property(config, "IP_MEMORIA"))
        IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");

    if(config_has_property(config, "PUERTO_MEMORIA"))
        PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");

    if(config_has_property(config, "PUERTO_ESCUCHA"))
        PUERTO_ESCUCHA = config_get_string_value(config, "PUERTO_ESCUCHA");
}
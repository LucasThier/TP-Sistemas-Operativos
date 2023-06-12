#include "../include/FileSystemMain.h"

t_log* FS_Logger;

int SocketFileSystem;
int SocketMemoria;


void sighandler(int s) {
	liberar_conexion(SocketFileSystem);
	exit(0);
}

int main(int argc, char* argv[])
{
	signal(SIGINT, sighandler);

	FS_Logger = log_create("FileSystem.log", NOMBRE_PROCESO, true, LOG_LEVEL_INFO);


	//leer las config
	LeerConfigs(argv[1],argv[2]);

	char* bitarray;
	int cantBloques = atoi(BLOCK_COUNT);
	int tamBloques = atoi(BLOCK_SIZE);
	t_bitarray* bitmap[cantBloques];
	FCB* FCBs[cantBloques];

	for (int i = 0; i < cantBloques; i++) bitmap[i] = bitarray_create(bitarray, tamBloques);

	InicializarConexiones();

	while(true){
		log_info(FS_Logger, "hilo principal esta ejecutando");
		sleep(10);
	}

	return EXIT_SUCCESS;
}

//Crea un hilo de escucha para el kernel
void InicializarConexiones()
{
	SocketMemoria = conectar_servidor(FS_Logger, "Memoria", IP_MEMORIA , PUERTO_MEMORIA);

	pthread_t HiloEscucha;

    if (pthread_create(&HiloEscucha, NULL, EscuchaKernel, NULL) != 0) {
        exit(EXIT_FAILURE);
    }

}

//Crea un servidor y espera al kernel, luego recibe mensajes del mismo
void* EscuchaKernel()
{
	SocketFileSystem = iniciar_servidor(FS_Logger, NOMBRE_PROCESO, "127.0.0.1", PUERTO_ESCUCHA);
	
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
			LiberarMemoria();
			return NULL;
		}
	}
	liberar_conexion(SocketFileSystem);
	return (void*)EXIT_FAILURE;
}

void LeerConfigs(char* path, char* path_superbloque)
{
    config = config_create(path);

    if(config_has_property(config, "IP_MEMORIA"))
        IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");

    if(config_has_property(config, "PUERTO_MEMORIA"))
        PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");

    if(config_has_property(config, "PUERTO_ESCUCHA"))
        PUERTO_ESCUCHA = config_get_string_value(config, "PUERTO_ESCUCHA");

   configSB = config_create(path_superbloque);

	if(config_has_property(configSB, "BLOCK_SIZE"))
		BLOCK_SIZE = config_get_string_value(configSB, "BLOCK_SIZE");

	if(config_has_property(configSB, "BLOCK_COUNT"))
		BLOCK_COUNT = config_get_string_value(configSB, "BLOCK_COUNT");

}


void LiberarMemoria(){
	// Cerrar el archivo de bloques
	fclose(BLOQUES);

	free(bitmap);
}

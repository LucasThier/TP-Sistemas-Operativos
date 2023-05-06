#include "../include/ConsolaMain.h"
#include <readline/readline.h>
#include <readline/history.h>

int main(int argc, char* argv[])
{

	//Auxiliar para cada linea que lee
	char linea[1024];

	//ruta del pseudocodigo
	char* path = argv[2];

	//abro el archivo de pseudocodigo si existe
	FILE* archivo = fopen(path, "r");
	if (archivo == NULL)
	{
		exit(EXIT_FAILURE);
	}

	//loopea entre cada linea del archivo de pseudocodigo
    while (fgets(linea, 1024, archivo))
	{
		//agregar un \n al final de cada linea que no termine en \n
		if (linea[strlen(linea) - 1] != '\n'){
			strcat(linea, "\n");
		}
		//en realidad habria un AgregarPaquete(linea)
        printf("%s", linea);        
    }
	fclose(archivo);

	config = config_create(argv[1]);
	IP_KERNEL = config_get_string_value(config, "IP_KERNEL");
	PUERTO_KERNEL = config_get_string_value(config, "PUERTO_KERNEL");

	t_log* Consola_Logger = log_create("Consola.log", NOMBRE_PROCESO, true, LOG_LEVEL_INFO);

	int SocketServidor = conectar_servidor(Consola_Logger, "Kernel", IP_KERNEL, PUERTO_KERNEL);

	//temporal enviar ints al kernel(realmente habria que enviar el pseudocodigo)
	int leido;
	do
	{		
    	printf("Ingresa un entero a enviar: ");
		scanf("%d", &leido);
		enviar_int(Consola_Logger, NOMBRE_PROCESO, SocketServidor, leido);
	} while (leido != 0);

	liberar_conexion(SocketServidor);

	return EXIT_SUCCESS;
}
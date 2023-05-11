#include "../include/ConsolaMain.h"
#include <readline/readline.h>
#include <readline/history.h>

int SocketKernel;

int main(int argc, char* argv[])
{


	Consola_Logger = log_create("Consola.log", NOMBRE_PROCESO, true, LOG_LEVEL_INFO);

	config = config_create(argv[1]);

	//ruta del pseudocodigo
	char* path = argv[2];

	//Auxiliar para cada linea que lee
	char linea[1024];

	if(ConectarConKernel() == 0)
	{
		log_error(Consola_Logger, "No se pudo conectar con el Kernel");
		return EXIT_FAILURE;
	}

	//abro el archivo de pseudocodigo si existe
	FILE* archivo = fopen(path, "r");
	if (archivo == NULL)
	{
		return EXIT_FAILURE;
	}

	t_paquete* paquete = crear_paquete(INSTRUCCIONES);
	//loopea entre cada linea del archivo de pseudocodigo
    while (fgets(linea, 1024, archivo))
	{
		//agregar un \n al final de cada linea que no termine en \n
		if (linea[strlen(linea)] != '\n'){
			strcat(linea, "\n");
		}

		//agregar la linea al paquete
		agregar_a_paquete(paquete, linea, strlen(linea) + 1);

        printf("linea agregada: %s", linea);        
    }
	fclose(archivo);

	//enviar el paquete al kernel
	enviar_paquete(paquete, SocketKernel);

	/*//temporal enviar ints al kernel(realmente habria que enviar el pseudocodigo)
	int leido;
	do
	{		
    	printf("Ingresa un entero a enviar: ");
		scanf("%d", &leido);
		enviar_int(Consola_Logger, NOMBRE_PROCESO, SocketKernel, leido);
	} while (leido != 0);*/

	liberar_conexion(SocketKernel);

	return EXIT_SUCCESS;
}

int ConectarConKernel()
{
	IP_KERNEL = config_get_string_value(config, "IP_KERNEL");
	PUERTO_KERNEL = config_get_string_value(config, "PUERTO_KERNEL");
	SocketKernel = conectar_servidor(Consola_Logger, "Kernel", IP_KERNEL, PUERTO_KERNEL);
	return SocketKernel;
}
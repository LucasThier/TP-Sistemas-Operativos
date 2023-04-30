#include "../include/ConsolaMain.h"
#include <readline/readline.h>
#include <readline/history.h>

int main(void)
{
	t_log *ConexionesLogger = log_create("Consola.log", NOMBRE_PROCESO, true, LOG_LEVEL_INFO);

	int SocketServidor = conectar_servidor(ConexionesLogger, "Kernel", "0.0.0.0", "33668");

	int leido;

	do
	{
    	printf("Ingresa un entero a enviar: ");
		scanf("%d", &leido);
		enviar_int(ConexionesLogger, NOMBRE_PROCESO, SocketServidor, leido);
	} while (leido != 0);

	// liberar_conexion(SocketServidor);

	return EXIT_SUCCESS;
}
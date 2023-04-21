#include "../include/ConsolaMain.h"
#include <readline/readline.h>
#include <readline/history.h>

int main(void)
{
	t_log* ConexionesLogger = log_create("Consola.log", NOMBRE_PROCESO, true, LOG_LEVEL_INFO);

	int SocketServidor = crear_conexion(ConexionesLogger, "Kernel", "0.0.0.0", "33668");

	int leido;

	do
	{
		scanf("Introduce el entero a enviar: %d", &leido);
		log_info(ConexionesLogger, "Numero leido: %d\n", leido);
		enviar_int(ConexionesLogger, NOMBRE_PROCESO, SocketServidor, leido);
	} while (leido > -1);

	//liberar_conexion(SocketServidor);

	return EXIT_SUCCESS;
}